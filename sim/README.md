# Device Simulation
This directory contains tools to simulate a discoverable device and the means by which the device advertises itself so that it may be discovered.


## The REST Server Simulator
The first tool is the Magellan REST Server Simulator, or `mrs`, and is really just a small web server (HTTPS only) that listens on a port for incoming REST requests.

The tool is written using Node.JS so you'll need to install Node to use it.  Once Node is installed, though, running the simulator is as easy as invoking the `mrs.sh` script which really just tells Node to execute the `mrs.js` Javascript app - the actual simulator.

>If you're feeling adventurous, take a look around inside `mrs.js` and modify it accordingly.

To run the simulator, just do the following in a terminal window:

```shell
$ ./mrs.js
```

## Advertising
Now that you have your simulator running, your Magellan-enabled application will need to discover it.

Magellan currently supports discovery through MDNS or SSDP.  Therefore, to discover the simulator, we need to advertise it using one (or both) of these protocols.  This is easily done with the `mad.sh` script - which you run in a seperate terminal window.  This script takes a single parameter to tell it what protocol to use.  For MDNS, pass `-mdns`.  For SSDP, pass `-ssdp`.

For example:
```shell
$ ./mad.sh -mdns
```

or

```shell
$ ./mad.sh -ssdp
```

If you take a look at the `mad.sh` script, you'll see that MDNS-based advertising is done using the Avahi subsystem of your Linux distro.  Specifically, it uses the `avahi-publish-service` command advertise the simulator.  For SSDP there is sadly no standard SSDP service on Linux as with Avahi.  So, for SSDP advertising, we simply use the `nc` Linux command to send the contents of a file (`response.ssdp`) to the standard IP multicast address and port used for SSDP.
>Be very careful when modifying the contents of `response.ssdp`.  It is the exact content of an SSDP packet - including `\r\n` characters required by the protocol.  Those characters won't always show up properly in your text editor.  Fundementally, though, the structure of an SSDP packet is basically a text file where each line ends with `\r\n` - the kind of text files that Windows machines produce.  This is different, of course, from Linux(ish) text files which simply end with `\n`.  If something goes wrong with the file, you can use the `dos2unix` Linux command to ensure that the file is in **Windows** format.

## Customization
The content served up by the simulator resides in `config.json`.  You can modify this content to your heart's content, making sure to restart the simulator whenever you do so.  Be careful though, to ensure that IDs are properly formatted and/or preserved and that the device's ID (if changed) is updated in the advertised content for MDNS and SSDP.  Also, be sure that whenever you change the configuration, you update the configuration version accordingly and **also** make sure that advertisements using MDNS and SSDP are updated accordingly.

For MDNS, make sure to update the `"id={d7107580-952d-4fd4-a4c2-a01f4067fd39}" "cv=234"` parameters. For SSDP, make sure that `X-MAGELLAN-CV`, `X-MAGELLAN-ID`, and `USN` are correct

## Certificates
X.509 certificates are key to the secure operation of all Magellan components.  One area of particular interest is that Magellan design recomnmends *mutual authentication*.  This means that not only does the client need to verify the authenticity of the server but that the server also verifies the authenticyty of the client.  Hence, both entities need to be in possession of their respective certificate (and associated private key) along with the CA certificate used to issue the *other* entity's certificate.  For example: if the server's certificate was issued by the `My Own Certificate Authority Inc` corporation, the the client needs to have the CA certificate of `My Own Certificate Authority Inc` in order to verify the server's certificate.  Likewise, if the client's certificate was issued by `Some Other Certificate Authority LLC`, then the server needs to be in possession of the CA certificate of `Some Other Certificate Authority LLC`.

>For our purposes we're using server and client certificates issued by the same certificate authority so we'll only have one CA certificate.

On the server side (the simulator), the HTTPS server is configured with a certificate and private (`server.crt` and `server.key`).  On the client side, the Magellan library is configured to use `client.crt` and `client.key`.  These certificates, private keys, and the CA certificate (`ca.crt` in this case) all reside in the `certs` directory of the repository.

>See the [README in the `certs` directory](../certs/README.md) on how you can create your own CA and client/server certificates.
