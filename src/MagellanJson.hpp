//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef MAGELLANJSON_HPP
#define MAGELLANJSON_HPP

#include <stdio.h>
#include <iostream>
#include "nlohmann/json.hpp"

namespace Magellan
{
    //-----------------------------------------------------------
    #define JSON_SERIALIZED_CLASS(_cn) \
        class _cn; \
        static void to_json(nlohmann::json& j, const _cn& p); \
        static void from_json(const nlohmann::json& j, _cn& p);

    #define IMPLEMENT_JSON_DOCUMENTATION(_cn) \
        public: \
        static void document(const char *path = nullptr) \
        { \
            _cn   example; \
            example.initForDocumenting(); \
            std::string theJson = example.serialize(3); \
            std::cout << "------------------------------------------------" << std::endl \
                      << #_cn << std::endl \
                      << theJson << std::endl \
                      << "------------------------------------------------" << std::endl; \
            \
            if(path != nullptr && path[0] != 0) \
            { \
                std::string fn = path; \
                fn.append("/"); \
                fn.append(#_cn); \
                fn.append(".json");  \
                \
                FILE *fp = _internalFileOpener(fn.c_str(), "wt");\
                \
                if(fp != nullptr) \
                { \
                    fputs(theJson.c_str(), fp); \
                    fclose(fp); \
                } \
                else \
                { \
                    std::cout << "ERROR: Cannot write to " << fn << std::endl; \
                } \
            } \
        }

    #define IMPLEMENT_JSON_SERIALIZATION() \
        public: \
        bool deserialize(const char *s) \
        { \
            try \
            { \
                if(s != nullptr && s[0] != 0) \
                { \
                    from_json(nlohmann::json::parse(s), *this); \
                } \
                else \
                { \
                    return false; \
                } \
            } \
            catch(...) \
            { \
                return false; \
            } \
            return true; \
        } \
        \
        std::string serialize(const int indent = -1) \
        { \
            nlohmann::json j; \
            to_json(j, *this); \
            return j.dump(indent); \
        }

    #define IMPLEMENT_WRAPPED_JSON_SERIALIZATION(_cn) \
        public: \
        std::string serializeWrapped(const int indent = -1) \
        { \
            nlohmann::json j; \
            to_json(j, *this); \
            \
            std::string rc; \
            char firstChar[2]; \
            firstChar[0] = #_cn[0]; \
            firstChar[1] = 0; \
            firstChar[0] = tolower(firstChar[0]); \
            rc.assign("{\""); \
            rc.append(firstChar); \
            rc.append((#_cn) + 1); \
            rc.append("\":"); \
            rc.append(j.dump(indent)); \
            rc.append("}"); \
            \
            return rc; \
        }

    #define TOJSON_IMPL(__var) \
        {#__var, p.__var}

    #define FROMJSON_IMPL_SIMPLE(__var) \
        getOptional(#__var, p.__var, j)

    #define FROMJSON_IMPL(__var, __type, __default) \
        getOptional<__type>(#__var, p.__var, j, __default)

    static FILE *_internalFileOpener(const char *fn, const char *mode)
    {
        FILE *fp = nullptr;

        #ifndef WIN32
            fp = fopen(fn, mode);
        #else
            if(fopen_s(&fp, fn, mode) != 0)
            {
                fp = nullptr;
            }
        #endif

        return fp;
    }

    template<class T>
    static void getOptional(const char *name, T& v, const nlohmann::json& j, T def)
    {
        try
        {
            j.at(name).get_to(v);
        }
        catch(...)
        {
            v = def;
        }
    }

    template<class T>
    static void getOptional(const char *name, T& v, const nlohmann::json& j)
    {
        try
        {
            j.at(name).get_to(v);
        }
        catch(...)
        {
        }
    }        


    //-----------------------------------------------------------
    static std::string EMPTY_STRING;


    //-----------------------------------------------------------
    class JsonObjectBase
    {
    public:
        JsonObjectBase()
        {
            _documenting = false;
        }

        virtual ~JsonObjectBase()
        {
        }

        virtual void initForDocumenting()
        {
            _documenting = true;
        }

    protected:
        bool _documenting;
    };    



    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(NetworkAddress)
    /**
    * @brief Helper class for serializing and deserializing the NetworkAddress JSON
    *
    * Helper C++ class to serialize and de-serialize NetworkAddress JSON
    *
    * Example: @include[doc] examples/NetworkAddress.json
    */
    class NetworkAddress : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(NetworkAddress)

    public:
        /**
         * @brief Address
         */
        std::string                             address;

        /**
         * @brief Port
         */
        int                                     port;

        NetworkAddress()
        {
            clear();
        }

        void clear()
        {
            address.clear();
            port = 0;
        }

        bool matches(NetworkAddress& other)
        {
            return ( address.compare(other.address) == 0 &&
                     port == other.port );
        }
    };

    static void to_json(nlohmann::json& j, const NetworkAddress& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(address),
            TOJSON_IMPL(port)
        };
    }

    static void from_json(const nlohmann::json& j, NetworkAddress& p)
    {
        p.clear();
        FROMJSON_IMPL(address, std::string, EMPTY_STRING);
        FROMJSON_IMPL(port, int, 0);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(DiscoveredDevice)
    /**
    * @brief Helper class for serializing and deserializing the DiscoveredDevice JSON
    *
    * Helper C++ class to serialize and de-serialize DiscoveredDevice JSON
    *
    * Example: @include[doc] examples/DiscoveredDevice.json
    */
    class DiscoveredDevice : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoveredDevice)

    public:
        /**
         * @brief Discoverer key (internal use)
         */
        std::string                             discovererKey;

        /**
         * @brief ID of the device
         *
         * A unique, device-provided string identifier.  Typically a GUID surrounded by braces.
         */
        std::string                             id;

        /**
         * @brief Current configuration version
         *
         * The number representing the current configuration version of the device.  The device must
         * increment this number each time a configuration change it made.
         */
        unsigned long                           configVersion;

        /**
         * @brief URL of the root REST API
         *
         * The URL representing the root REST API for the device
         */
        std::string                             rootUrl;


        DiscoveredDevice()
        {
            clear();
        }

        void clear()
        {
            discovererKey.clear();
            id.clear();
            configVersion = 0;
            rootUrl.clear();
        }
    };

    static void to_json(nlohmann::json& j, const DiscoveredDevice& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(discovererKey),
            TOJSON_IMPL(id),
            TOJSON_IMPL(configVersion),
            TOJSON_IMPL(rootUrl)
        };
    }

    static void from_json(const nlohmann::json& j, DiscoveredDevice& p)
    {
        p.clear();
        FROMJSON_IMPL(discovererKey, std::string, EMPTY_STRING);
        FROMJSON_IMPL(id, std::string, EMPTY_STRING);
        FROMJSON_IMPL(configVersion, unsigned long, 0);
        FROMJSON_IMPL(rootUrl, std::string, EMPTY_STRING);
    }    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(ThingInfo)
    /**
    * @brief Helper class for serializing and deserializing the ThingInfo JSON
    *
    * Helper C++ class to serialize and de-serialize ThingInfo JSON
    *
    * Example: @include[doc] examples/ThingInfo.json
    */
    class ThingInfo : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(ThingInfo)

    public:
        /**
         * @brief ID of the thing
         */
        std::string                             id;

        /**
         * @brief A string describing the type of device
         */
        std::string                             type;

        /**
         * @brief A string containing the thing's manufacturer
         */
        std::string                             manufacturer;

        /**
         * @brief A string array of capabilities
         */
        std::vector<std::string>                capabilities;

        ThingInfo()
        {
            clear();
        }

        void clear()
        {
            id.clear();
            type.clear();
            manufacturer.clear();
            capabilities.clear();
        }
    };

    static void to_json(nlohmann::json& j, const ThingInfo& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(id),
            TOJSON_IMPL(type),
            TOJSON_IMPL(manufacturer),
            TOJSON_IMPL(capabilities)
        };
    }

    static void from_json(const nlohmann::json& j, ThingInfo& p)
    {
        p.clear();
        FROMJSON_IMPL(id, std::string, EMPTY_STRING);
        FROMJSON_IMPL(type, std::string, EMPTY_STRING);
        FROMJSON_IMPL(manufacturer, std::string, EMPTY_STRING);
        getOptional<std::vector<std::string>>("capabilities", p.capabilities, j);
    }    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Presence)
    /**
    * @brief Helper class for serializing and deserializing the Presence JSON
    *
    * Helper C++ class to serialize and de-serialize Presence JSON
    *
    * Example: @include[doc] examples/Presence.json
    */
    class Presence : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Presence)

    public:
        /**
         * @brief True if presence is to be transmitted on every audio TX
         */
        bool                                    forceOnAudioTransmit;

        /**
         * @brief Format of the presence data
         */
        int                                     format;

        /**
         * @brief Interval at which to send updates
         */
        int                                     intervalSecs;

        Presence()
        {
            clear();
        }

        void clear()
        {
            forceOnAudioTransmit = false;
            format = 1;
            intervalSecs = 30;
        }

        bool matches(Presence& other)
        {
            return (forceOnAudioTransmit == other.forceOnAudioTransmit &&
                    format == other.format &&
                    intervalSecs == other.intervalSecs);
        }
    };

    static void to_json(nlohmann::json& j, const Presence& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(forceOnAudioTransmit),
            TOJSON_IMPL(format),
            TOJSON_IMPL(intervalSecs)
        };
    }

    static void from_json(const nlohmann::json& j, Presence& p)
    {
        p.clear();
        FROMJSON_IMPL(forceOnAudioTransmit, bool, false);
        FROMJSON_IMPL(format, int, 1);
        FROMJSON_IMPL(intervalSecs, int, 30);
    }    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(TxAudio)
    /**
    * @brief Helper class for serializing and deserializing the TxAudio JSON
    *
    * Helper C++ class to serialize and de-serialize TxAudio JSON
    *
    * Example: @include[doc] examples/TxAudio.json
    */
    class TxAudio : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TxAudio)

    public:
        /**
         * @brief Encoder name
         */
        std::string                             encoder;

        /**
         * @brief Allows full-duplex operation when set to true.
         */
        bool                                    fdx;

        /**
         * @brief Maximum number of seconds of TX allowed per talk spurt
         */
        int                                     maxTxSecs;

        /**
         * @brief Milliseconds of audio to code per packet (encoder dependent)
         */
        int                                     framingMs;

        /**
         * @brief Disables header extensions when set to true
         */
        bool                                    noHdrExt;

        /**
         * @brief Packet interval at which to send header extensions
         */
        int                                     extensionSendInterval;

        /**
         * @brief Number of headers to include at beginning of TX
         */
        int                                     initialHeaderBurst;

        /**
         * @brief Number of trailing headers to send at conclusion of a talk spurt
         */
        int                                     trailingHeaderBurst;


        TxAudio()
        {
            clear();
        }

        void clear()
        {
            encoder = "ctOpus8000";
            fdx = false;
            maxTxSecs = 30;
            framingMs = 60;
            noHdrExt = false;
            extensionSendInterval = 10;
            initialHeaderBurst = 5;
            trailingHeaderBurst = 5;
        }

        bool matches(TxAudio& other)
        {
            return ( encoder.compare(other.encoder) == 0 &&
                     fdx == other.fdx &&
                     maxTxSecs == other.maxTxSecs &&
                     framingMs == other.framingMs &&
                     noHdrExt == other.noHdrExt &&
                     extensionSendInterval == other.extensionSendInterval &&
                     initialHeaderBurst == other.initialHeaderBurst &&
                     trailingHeaderBurst == other.trailingHeaderBurst
                   );
        }
    };

    static void to_json(nlohmann::json& j, const TxAudio& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(encoder),
            TOJSON_IMPL(fdx),
            TOJSON_IMPL(maxTxSecs),
            TOJSON_IMPL(framingMs),
            TOJSON_IMPL(noHdrExt),
            TOJSON_IMPL(extensionSendInterval),
            TOJSON_IMPL(initialHeaderBurst),
            TOJSON_IMPL(trailingHeaderBurst)
        };
    }

    static void from_json(const nlohmann::json& j, TxAudio& p)
    {
        p.clear();
        FROMJSON_IMPL(encoder, std::string, "ctOpus8000");
        FROMJSON_IMPL(fdx, bool, false);
        FROMJSON_IMPL(maxTxSecs, int, 30);
        FROMJSON_IMPL(framingMs, int, 60);
        FROMJSON_IMPL(noHdrExt, bool, false);
        FROMJSON_IMPL(extensionSendInterval, int, 10);
        FROMJSON_IMPL(initialHeaderBurst, int, 5);
        FROMJSON_IMPL(trailingHeaderBurst, int, 5);
    }    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(NetworkOptions)
    /**
    * @brief Helper class for serializing and deserializing the NetworkOptions JSON
    *
    * Helper C++ class to serialize and de-serialize NetworkOptions JSON
    *
    * Example: @include[doc] examples/NetworkOptions.json
    */
    class NetworkOptions : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(NetworkOptions)

    public:
        /**
         * @brief Transmission priority mapped to OS-specific implementation of QoS
         */
        int                                     priority;

        /**
         * @brief Packet TTL
         */
        int                                     ttl;


        NetworkOptions()
        {
            clear();
        }

        void clear()
        {
            priority = 4;
            ttl = 1;
        }

        bool matches(NetworkOptions& other)
        {
            return ( priority == other.priority &&
                     ttl == other.ttl
                   );
        }
    };

    static void to_json(nlohmann::json& j, const NetworkOptions& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(priority),
            TOJSON_IMPL(ttl)
        };
    }

    static void from_json(const nlohmann::json& j, NetworkOptions& p)
    {
        p.clear();
        FROMJSON_IMPL(priority, int, 4);
        FROMJSON_IMPL(ttl, int, 1);
    }    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(TalkgroupSecurity)
    /**
    * @brief Helper class for serializing and deserializing the TalkgroupSecurity JSON
    *
    * Helper C++ class to serialize and de-serialize TalkgroupSecurity JSON
    *
    * Example: @include[doc] examples/TalkgroupSecurity.json
    */
    class TalkgroupSecurity : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TalkgroupSecurity)

    public:
        /**
         * @brief Minimum security level required to access the group
         */
        int                                     minLevel;

        /**
         * @brief Maximum security level offered by the group
         */
        int                                     maxLevel;


        TalkgroupSecurity()
        {
            clear();
        }

        void clear()
        {
            minLevel = 0;
            maxLevel = 0;
        }

        bool matches(TalkgroupSecurity& other)
        {
            return ( minLevel == other.minLevel &&
                     maxLevel == other.maxLevel
                   );
        }
    };

    static void to_json(nlohmann::json& j, const TalkgroupSecurity& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(minLevel),
            TOJSON_IMPL(maxLevel)
        };
    }

    static void from_json(const nlohmann::json& j, TalkgroupSecurity& p)
    {
        p.clear();
        FROMJSON_IMPL(minLevel, int, 0);
        FROMJSON_IMPL(maxLevel, int, 0);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Rallypoint)
    /**
    * @brief Helper class for serializing and deserializing the Rallypoint JSON
    *
    * Helper C++ class to serialize and de-serialize Rallypoint JSON
    *
    * Example: @include[doc] examples/Rallypoint.json
    */
    class Rallypoint : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Rallypoint)

    public:
        /**
         * @brief Address and port of the machine hosting the Rallypoint
         */
        NetworkAddress                          host;

        Rallypoint()
        {
            clear();
        }

        void clear()
        {
            host.clear();
        }

        bool matches(Rallypoint& other)
        {
            return ( host.matches(other.host) );
        }
    };

    static void to_json(nlohmann::json& j, const Rallypoint& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(host)
        };
    }

    static void from_json(const nlohmann::json& j, Rallypoint& p)
    {
        p.clear();
        getOptional<NetworkAddress>("host", p.host, j);
    }    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Talkgroup)
    /**
    * @brief Helper class for serializing and deserializing the Talkgroup JSON
    *
    * Helper C++ class to serialize and de-serialize Talkgroup JSON
    *
    * Example: @include[doc] examples/Talkgroup.json
    */
    class Talkgroup : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Talkgroup)

    public:
        /**
         * @brief ID of the talk group
         */
        std::string                             id;

        /**
         * @brief Talk group type
         */
        int                                     type;

        /**
         * @brief Name of the talk group
         */
        std::string                             name;

        /**
         * @brief Crypto password in hex-represented binary
         */
        std::string                             cryptoPassword;

        /**
         * @brief Presence information if of type 2
         */
        Presence                                presence;

        /**
         * @brief Array of Rallypoints
         */
        std::vector<Rallypoint>                 rallypoints;

        /**
         * @brief RX address and port
         */
        NetworkAddress                          rx;

        /**
         * @brief TX address and port
         */
        NetworkAddress                          tx;

        /**
         * @brief TX audio
         */
        TxAudio                                 txAudio;


        /**
         * @brief Network options
         */
        NetworkOptions                          networkOptions;

        /**
         * @brief Security options
         */
        TalkgroupSecurity                       security;

        Talkgroup()
        {
            clear();
        }

        void clear()
        {
            id.clear();
            type = 0;
            name.clear();
            cryptoPassword.clear();
            presence.clear();
            rallypoints.clear();
            rx.clear();
            tx.clear();
            txAudio.clear();
            networkOptions.clear();
            security.clear();
        }

        bool matches(Talkgroup& other)
        {
            if( id.compare(other.id) == 0 &&
                type == other.type &&
                name.compare(other.name) == 0 &&
                cryptoPassword.compare(other.cryptoPassword) == 0 &&
                presence.matches(other.presence) &&
                rx.matches(other.rx) &&
                tx.matches(other.tx) &&
                txAudio.matches(other.txAudio) &&
                networkOptions.matches(other.networkOptions) &&
                security.matches(other.security)                
               )
            {
                // Looks good so far, now check out the rallypoint array
                if(rallypoints.size() == other.rallypoints.size())
                {
                    for(size_t x = 0; x < rallypoints.size(); x++)
                    {
                        if(!rallypoints[x].matches(other.rallypoints[x]))
                        {
                            return false;
                        }
                    }

                    return true;
                }
            }

            return false;
        }
    };

    static void to_json(nlohmann::json& j, const Talkgroup& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(id),
            TOJSON_IMPL(type),
            TOJSON_IMPL(name),
            TOJSON_IMPL(cryptoPassword),
            TOJSON_IMPL(presence),
            TOJSON_IMPL(rallypoints),
            TOJSON_IMPL(rx),
            TOJSON_IMPL(tx),
            TOJSON_IMPL(txAudio),
            TOJSON_IMPL(networkOptions),
            TOJSON_IMPL(security)
        };
    }

    static void from_json(const nlohmann::json& j, Talkgroup& p)
    {
        p.clear();
        FROMJSON_IMPL(id, std::string, EMPTY_STRING);
        FROMJSON_IMPL(type, int, 0);
        FROMJSON_IMPL(cryptoPassword, std::string, EMPTY_STRING);
        getOptional<Presence>("presence", p.presence, j);
        getOptional<std::vector<Rallypoint>>("rallypoints", p.rallypoints, j);
        getOptional<NetworkAddress>("rx", p.rx, j);
        getOptional<NetworkAddress>("tx", p.tx, j);
        getOptional<TxAudio>("txAudio", p.txAudio, j);
        getOptional<NetworkOptions>("networkOptions", p.networkOptions, j);
        getOptional<TalkgroupSecurity>("security", p.security, j);
    }    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(DeviceConfiguration)
    /**
    * @brief Helper class for serializing and deserializing the DeviceConfiguration JSON
    *
    * Helper C++ class to serialize and de-serialize DeviceConfiguration JSON
    *
    * Example: @include[doc] examples/DeviceConfiguration.json
    */
    class DeviceConfiguration : public JsonObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DeviceConfiguration)

    public:
        /**
         * @brief Discoverer key (internal use)
         */
        std::string                             discovererKey;

        /**
         * @brief Current configuration version
         */
        unsigned long                           version;

        /**
         * @brief Timestamp of last configuration change (UTC)
         */
        std::string                             dateTimeStamp;

        /**
         * @brief ThingInfo
         */
        ThingInfo                               thingInfo;

        /**
         * @brief Array of talkgroups
         */
        std::vector<Talkgroup>                  talkgroups;

        DeviceConfiguration()
        {
            clear();
        }

        void clear()
        {
            discovererKey.clear();
            version = 0;
            dateTimeStamp.clear();
            thingInfo.clear();
            talkgroups.clear();
        }
    };

    static void to_json(nlohmann::json& j, const DeviceConfiguration& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(discovererKey),
            TOJSON_IMPL(version),
            TOJSON_IMPL(dateTimeStamp),
            TOJSON_IMPL(thingInfo),
            TOJSON_IMPL(talkgroups)
        };
    }

    static void from_json(const nlohmann::json& j, DeviceConfiguration& p)
    {
        p.clear();
        FROMJSON_IMPL(discovererKey, std::string, EMPTY_STRING);
        FROMJSON_IMPL(version, unsigned long, 0);
        FROMJSON_IMPL(dateTimeStamp, std::string, EMPTY_STRING);
        getOptional<ThingInfo>("thingInfo", p.thingInfo, j);
        getOptional<std::vector<Talkgroup>>("talkgroups", p.talkgroups, j);
    } 
}
#endif
