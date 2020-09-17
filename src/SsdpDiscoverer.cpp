//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <inttypes.h>

#include "SsdpDiscoverer.hpp"
#include "ILogger.hpp"
#include "MagellanCore.hpp"
#include "MagellanDataModel.hpp"

namespace Magellan
{
    /* Struct : lssdp_nbr */
    #define LSSDP_FIELD_LEN         128
    #define LSSDP_LOCATION_LEN      256

    typedef struct lssdp_nbr 
    {
        char            usn         [LSSDP_FIELD_LEN];          // Unique Service Name (Device Name or MAC)
        char            location    [LSSDP_LOCATION_LEN];       // URL or IP(:Port)

        /* Additional SSDP Header Fields */
        char            sm_id       [LSSDP_FIELD_LEN];
        char            device_type [LSSDP_FIELD_LEN];
        long long       update_time;
        struct lssdp_nbr * next;
    } lssdp_nbr;


    #define LSSDP_INTERFACE_NAME_LEN    16                      // IFNAMSIZ
    #define LSSDP_INTERFACE_LIST_SIZE   16
    #define LSSDP_IP_LEN                16

    typedef enum
    {
        unknown,
        msearch,
        notify,
        response
    } SsdpMethod_t;

    /** Struct: lssdp_packet **/
    typedef struct lssdp_packet 
    {
        SsdpMethod_t    method;
        char            st              [LSSDP_FIELD_LEN];
        char            usn             [LSSDP_FIELD_LEN];
        char            location        [LSSDP_LOCATION_LEN];

        char            sm_id           [LSSDP_FIELD_LEN];
        char            device_type     [LSSDP_FIELD_LEN];
        char            cache_control   [LSSDP_FIELD_LEN];
        char            server          [LSSDP_FIELD_LEN];
        char            date            [LSSDP_FIELD_LEN];
        uint64_t        received_ts;

        char            x_magellan_cv   [LSSDP_FIELD_LEN];
        char            x_magellan_id   [LSSDP_FIELD_LEN];
    } lssdp_packet;

    static const char *HEADER_MSEARCH = "M-SEARCH * HTTP/1.1\r\n";
    static const char *HEADER_NOTIFY = "NOTIFY * HTTP/1.1\r\n";
    static const char *HEADER_RESPONSE = "HTTP/1.1 200 OK\r\n";

    static const char *TAG = "SsdpDiscoverer";

    SsdpDiscoverer::SsdpDiscoverer()
    {
        setImplementation("Ssdp");
        _running = false;
    }

    SsdpDiscoverer::~SsdpDiscoverer()
    {
    }

    void SsdpDiscoverer::deleteThis()
    {
        stop();
        ReferenceCountedObject::deleteThis();
    }
        
    bool SsdpDiscoverer::configure(DataModel::JsonObjectBase& configuration)
    {
        _configuration = (DataModel::Ssdp&)configuration;
        return true;
    }

    bool SsdpDiscoverer::start()
    {
        bool rc = false;

        Core::getLogger()->d(TAG, "{%p} started", (void*) this);

        if(_running)
        {
            return true;
        }

        _running = true;

        _workerThreadHandle = std::thread(&SsdpDiscoverer::workerThread, this);

        return rc;
    }

    void SsdpDiscoverer::stop()
    {
        Core::getLogger()->d(TAG, "{%p} stopped", (void*) this);

        _running = false;

        if(_workerThreadHandle.joinable())
        {
            _workerThreadHandle.join();
        }
    }

    void SsdpDiscoverer::pause()
    {
        Core::getLogger()->d(TAG, "{%p} paused", (void*) this);
    }

    void SsdpDiscoverer::resume()
    {
        Core::getLogger()->d(TAG, "{%p} resumed", (void*) this);
    }

    void SsdpDiscoverer::workerThread()
    {
        static const size_t BUFF_SZ = 4096;

        char buffer[BUFF_SZ] = "";

        int                 sock = 0;
	    struct timeval      tv;
	    struct sockaddr_in  groupSock;
	    struct sockaddr_in  localSock;
	    struct ip_mreq      group;
        uint64_t            errCount = 0;
        ssize_t             rc;

        _neighbors.clear();
        _lastNeighborCheck = 0;

        while( _running )
        {
            checkNeighbors();

            if(sock != 0)
            {
                close(sock);
                sock = 0;
            }

            if(errCount > 0)
            {
                uint64_t msWait = (errCount * 100);
                if(msWait > _configuration.maxReconnectMs)
                {
                    msWait = _configuration.maxReconnectMs;
                }

                Core::getLogger()->d(TAG, "waiting for %" PRIu64 " milliseconds before reconnect attenpt", msWait);

                uint64_t tsStarted = Core::getNowMs();

                while(_running)
                {
                    if(Core::getNowMs() - tsStarted >= msWait)
                    {
                        break;
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                if(!_running)
                {
                    break;
                }
            }

            sock = socket(AF_INET, SOCK_DGRAM, 0);
            if(sock <= 0)
            {
                Core::getLogger()->e(TAG, "socket() failed");
                errCount++;
                continue;
            }

            // Reuse
            {
                int reuse = 1;
                if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) != 0)
                {
                    Core::getLogger()->e(TAG, "setsockopt(SO_REUSEADDR) failed");
                    errCount++;
                    continue;
                }
            }

            // Receive timeout
            {
	            tv.tv_sec = 1;
	            tv.tv_usec = 0;
	            if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) != 0)
                {
                    Core::getLogger()->e(TAG, "setsockopt(SO_RCVTIMEO) failed");
                    errCount++;
                    continue;
                }
            }

            // Multicast loopback
            {
                char loopch = 0;
                if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopch, sizeof(loopch)) != 0)
                {
                    Core::getLogger()->e(TAG, "setsockopt(IP_MULTICAST_LOOP) failed");
                    errCount++;
                    continue;
                }
            }

            // Bind
	        {
                localSock.sin_family = AF_INET;
                localSock.sin_port = htons(1900);
                localSock.sin_addr.s_addr = INADDR_ANY;
                if(bind(sock, (struct sockaddr*)&localSock, sizeof(localSock)) != 0)
                {
                    Core::getLogger()->e(TAG, "bind() failed");
                    errCount++;
                    continue;
                }
            }

            // Multicast join
            {
                group.imr_multiaddr.s_addr = inet_addr(_configuration.listener.address.c_str());
                if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) != 0)
                {
                    Core::getLogger()->e(TAG, "setsockopt(IP_ADD_MEMBERSHIP) failed");
                    errCount++;
                    continue;
                }
            }

            groupSock.sin_family = AF_INET;
	        groupSock.sin_addr.s_addr = inet_addr(_configuration.listener.address.c_str());
	        groupSock.sin_port = htons(_configuration.listener.port);
            snprintf(buffer, BUFF_SZ, 
                        "M-SEARCH * HTTP/1.1\r\n"
                        "HOST: %s:%d\r\n"
                        "ST: %s\r\n"
                        "MAN: \"ssdp:discover\"\r\n"
                        "MX: %d\r\n"
                        "USER-AGENT: %s"
                        "\r\n",

		                _configuration.listener.address.c_str(), 
                        _configuration.listener.port, 
                        _configuration.st.c_str(),
                        _configuration.mx,
                        _configuration.userAgent.c_str());

            Core::getLogger()->i(TAG, "sending M-SEARCH '%s'", buffer);                        

            if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) != (ssize_t)strlen(buffer))
            {
                Core::getLogger()->e(TAG, "sendto() failed");
                errCount++;
                continue;
            }
    
            while( _running )
            {
                checkNeighbors();

                struct sockaddr_in senderAddr;
                socklen_t slen = sizeof(senderAddr);

                rc = recvfrom(sock, buffer, BUFF_SZ, 0, (struct sockaddr*)&senderAddr, &slen);
                if(rc <= 0)
                {
                    if(errno == EAGAIN)
                    {
                        //Core::getLogger()->d(TAG, "recvfrom() timeout");
                        continue;
                    }

                    Core::getLogger()->e(TAG, "recvfrom() failed, errno=%d", errno);
                    break;
                }

                // Reset errors to 0 upon first successful receive
                errCount = 0;

                buffer[rc] = 0;
                //Core::getLogger()->d(TAG, "-------------------\n%s", buffer);

                DataModel::DiscoveredDevice    *dd = parseMessage(buffer);
                if(dd != nullptr)
                {
                    Core::processDiscoveredDevice(dd);
                }
            }

            close(sock);
            sock = 0;
        }

        if(sock != 0)
        {
            close(sock);
        }

        for(NeighborMap_t::iterator itr = _neighbors.begin();
            itr != _neighbors.end();
            itr++)
        {
            Core::processUndiscoveredDevice(itr->first.c_str());
        }

        _neighbors.clear();
    }

    void SsdpDiscoverer::checkNeighbors()
    {
        uint64_t now = Core::getNowMs();

        if( (_lastNeighborCheck == 0) || 
            ((now - _lastNeighborCheck) >= _configuration.staleNeighorCheckIntervalMs))
        {
            _lastNeighborCheck = now;

            std::vector<std::string> trash;

            for(NeighborMap_t::iterator itr = _neighbors.begin();
                itr != _neighbors.end();
                itr++)
            {
                if(itr->second._expiresAt <= now)
                {
                    trash.push_back(itr->first);
                }
            }

            for(std::vector<std::string>::iterator itr = trash.begin();
                itr != trash.end();
                itr++)
            {
                Core::getLogger()->i(TAG, "'%s' has disappeared", itr->c_str());
                Core::processUndiscoveredDevice(itr->c_str());
                _neighbors.erase(*itr);
            }
        }
    }

    static int trim_spaces(const char *string, ssize_t *start, ssize_t *end) 
    {
        ssize_t i = *start;
        ssize_t j = *end;

        while (i <= *end   && (!isprint(string[i]) || isspace(string[i]))) 
        {
            i++;
        }

        while (j >= *start && (!isprint(string[j]) || isspace(string[j])))
        {
            j--;
        }

        if (i > j) 
        {
            return -1;
        }

        *start = i;
        *end   = j;
        
        return 0;
    }    

    static ssize_t get_colon_index(const char *string, ssize_t start, ssize_t end) 
    {
        ssize_t i;

        for (i = start; i <= end; i++) 
        {
            if (string[i] == ':') 
            {
                return i;
            }
        }

        return -1;
    }

    static bool splitIt(const char *fieldName, const char *field, size_t field_len, const char *value, size_t value_len, char *dst)
    {
        if(field_len > 0 && strncasecmp(field, fieldName, field_len) == 0) 
        {
            memcpy(dst, value, value_len < LSSDP_FIELD_LEN ? value_len : LSSDP_FIELD_LEN - 1);
            return true;
        }

        return false;
    }

    static int parse_field_line(const char *data, ssize_t start, ssize_t end, lssdp_packet * packet) 
    {
        // 1. find the colon
        if (data[start] == ':') 
        {
            Core::getLogger()->w(TAG, "the first character of line should not be colon");
            Core::getLogger()->d(TAG, "%s", data);

            return -1;
        }

        ssize_t colon = get_colon_index(data, start + 1, end);
        if (colon == -1) 
        {
            Core::getLogger()->w(TAG, "there is no colon in line");
            Core::getLogger()->d(TAG, "%s", data);

            return -1;
        }

        if (colon == end) 
        {
            // value is empty
            return -1;
        }


        // 2. get field, field_len
        ssize_t i = start;
        ssize_t j = colon - 1;
        if (trim_spaces(data, &i, &j) == -1) 
        {
            return -1;
        }

        const char * field = &data[i];
        ssize_t field_len = j - i + 1;


        // 3. get value, value_len
        i = colon + 1;
        j = end;
        if (trim_spaces(data, &i, &j) == -1) 
        {
            return -1;
        };

        const char * value = &data[i];
        ssize_t value_len = j - i + 1;

        // 4. set each field's value to packet
        if(splitIt("st", field, field_len, value, value_len, packet->st)) return 0;     // NT and ST are the same
        if(splitIt("nt", field, field_len, value, value_len, packet->st)) return 0;     // NT and ST are the same
        if(splitIt("usn", field, field_len, value, value_len, packet->usn)) return 0;
        if(splitIt("location", field, field_len, value, value_len, packet->location)) return 0;
        if(splitIt("cache-control", field, field_len, value, value_len, packet->cache_control)) return 0;
        if(splitIt("server", field, field_len, value, value_len, packet->server)) return 0;
        if(splitIt("date", field, field_len, value, value_len, packet->date)) return 0;

        if(splitIt("sm_id", field, field_len, value, value_len, packet->sm_id)) return 0;
        if(splitIt("dev_type", field, field_len, value, value_len, packet->device_type)) return 0;

        if(splitIt("x-magellan-cv", field, field_len, value, value_len, packet->x_magellan_cv)) return 0;
        if(splitIt("x-magellan-id", field, field_len, value, value_len, packet->x_magellan_id)) return 0;

        // the field is not in the struct packet
        return 0;
    }    

    DataModel::DiscoveredDevice *SsdpDiscoverer::parseMessage(char *msg)
    {
        if(msg == nullptr || msg[0] == 0) 
        {
            Core::getLogger()->e(TAG, "data should not be NULL");
            return nullptr;
        }

        //Core::getLogger()->i(TAG, "%s", msg);

        size_t msgLen = strlen(msg);

        lssdp_packet packet;

        memset(&packet, 0, sizeof(packet));

        size_t i;
        if ((i = strlen(HEADER_MSEARCH)) < msgLen && memcmp(msg, HEADER_MSEARCH, i) == 0) 
        {
            packet.method = SsdpMethod_t::msearch;
        } 
        else if ((i = strlen(HEADER_NOTIFY)) < msgLen && memcmp(msg, HEADER_NOTIFY, i) == 0) 
        {
            packet.method = SsdpMethod_t::notify;
        } 
        else if ((i = strlen(HEADER_RESPONSE)) < msgLen && memcmp(msg, HEADER_RESPONSE, i) == 0) 
        {
            packet.method = SsdpMethod_t::response;
        } 
        else 
        {
            packet.method = SsdpMethod_t::unknown;

            Core::getLogger()->w(TAG, "received unknown SSDP packet");
            Core::getLogger()->d(TAG, "%s", msg);

            return nullptr;
        }

        size_t start = i;
        for (i = start; i < msgLen; i++) 
        {
            if(msg[i] == '\n' && i - 1 > start && msg[i - 1] == '\r') 
            {
                parse_field_line(msg, start, i - 2, &packet);
                start = i + 1;
            }
        }

        packet.received_ts = Core::getNowMs();

        if(strcasecmp(packet.st, _configuration.st.c_str()) != 0)
        {
            //Core::getLogger()->d(TAG, "non-matching ST '%s' - ignoring", packet.st);
            return nullptr;
        }

        if(packet.usn[0] == 0)
        {
            Core::getLogger()->d(TAG, "no USN - ignoring");
            return nullptr;
        }

        if(packet.x_magellan_id[0] == 0)
        {
            Core::getLogger()->d(TAG, "no Magellan ID - ignoring");
            return nullptr;
        }

        if(packet.x_magellan_cv[0] == 0)
        {
            Core::getLogger()->d(TAG, "no Magellan version - ignoring");
            return nullptr;
        }

        char key[1024];
        sprintf(key, "%s/%s/%s/%s", getImplementation(), packet.st, packet.usn, packet.x_magellan_id);

        NeighborData_t *nd = nullptr;
        NeighborMap_t::iterator itr = _neighbors.find(key);
        bool needsProcessing = false;

        if(itr == _neighbors.end())
        {
            NeighborData_t d;
            d._version = (unsigned long)atol(packet.x_magellan_cv);;
            _neighbors[key] = d;
            nd = &(_neighbors.find(key)->second);
            needsProcessing = true;

            Core::getLogger()->i(TAG, "new neighbor - '%s'", key);
        }
        else
        {
            nd = &(itr->second);
        }

        // TODO: get expiration from cache-control header
        nd->_expiresAt = (Core::getNowMs() + 5000);
        
        if(!needsProcessing)
        {
            if(nd->_version != (unsigned long)atol(packet.x_magellan_cv))
            {
                nd->_version = (unsigned long)atol(packet.x_magellan_cv);
                needsProcessing = true;
                Core::getLogger()->i(TAG, "neighbor changed version - '%s'", key);
            }
        }
        
        if(!needsProcessing)
        {
            //Core::getLogger()->d(TAG, "cached neighbor - '%s'", key);
            return nullptr;
        }

        DataModel::DiscoveredDevice    *dd = new DataModel::DiscoveredDevice();

        dd->discovererKey.assign(key);
        dd->id.assign(packet.x_magellan_id);
        dd->configVersion = atoi(packet.x_magellan_cv);
        dd->rootUrl.assign(packet.location);

        Core::getLogger()->d(TAG, "type=%s\nloc=%s\nmeth=%d\nsm=%s\nst=%s\nusn=%s\nupd=%" PRIu64 "\ncc=%s",
                            packet.device_type,
                            packet.location,
                            (int)packet.method,
                            packet.sm_id,
                            packet.st,
                            packet.usn,
                            packet.received_ts,
                            packet.cache_control);        

        return dd;
    }
}
