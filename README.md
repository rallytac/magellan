# Magellan
Magellan is a US-government sponsored project that simplifies and secures configuration, management, discovery, and interoperability of two-way radio systems with enterprise IP network applications.
>This project has been named for [Ferdinand Magellan](https://en.wikipedia.org/wiki/Ferdinand_Magellan), the Portugese explorer renowed for his navigational skill and tenacity which contributed to man's first circumnavigation sailing of the Earth which concluded in 1522.

## Background
The progress of integration of voice-capable IP networks and two-way radio systems has existed in a state of stagnation for a great many years. Industry and practitioners alike have simply come to accept the limitations of “state of the art” technologies and have built their architectures and operating procedures around these limitations.

In particular, the methodology used for interfacing two-way radio systems and IP networks has been in a state of stasis since basic audio exchange between the two technologies were put in place decades ago through the use of simple analog-to-IP gateway devices. Essentially, many years ago, industry demonstrated a limited, non-secure, non-standard capability for audio to flow back and forth between a two-way radio system and an IP network. Customer practitioners jumped at this new capability and implemented it - warts and all - because they needed it so desperately. However, technology has improved, security issues have become even more relevant, systems have increased in complexity, and the sheer scale of such communications has exceeded anyone’s wildest imaginations.

The areas of problems created with this stagnating state of affairs include, but are certainly not limited to:
- Vendor-specific implementations that are not industry standard, lack of interoperability between vendors’ products, and inconsistent implementations. These all create vendor-lockin for customers; increasing costs, complicating deployments, and limiting choice.
- An almost complete lack of security in areas such as system access, encryption, and configuration. In the past, this was (perhaps naively) addressed by placing these critical systems on secured networks in secured enclaves. However, it has been shown that an organization faces as many (if not more) threats from inside as from outside its secure environment. In addition, a secured environment is not always a feasible construct given that operators often need access in as many situations and on as many networks as possible - including public networks and those shared with (sometimes) trusted partners.
- Aside from basic security issues concerning encryption and such, the impact of improper use of communications resources is vastly underestimated. For example: consider a situation where a user is transmitting from an IP network onto a combatnet radio system (CNR). And they are doing so when they should not be, such as when more important communication is needed from the CNR side. This not only impacts mission execution but also endangers the lives of operators. In the majority of implementations, it is hoped that the training given to users will mitigate against such abuse. But that is a rather naive hope and not easily enforceable (or trackable).
- Configuration and management - especially of a centralized nature - has become exponentially more complex as systems have scaled in terms of in size, geographic distribution, and device/user count. In addition, the sheer increase in network traffic has resulted in practitioners seeking alternative means to convey this traffic - increasing the width of their threat plane and significantly impacting their security posture. This problem, alone, then creates a whole new set of challenges for practitioners in areas such as acquisition, deployment, and management of secured infrastructures such as VPNs and the like. These, in turn, drive up costs yet again and add even more burden to administrative personnel and operators.

## Solution
The simplest way to imagine the operation of Magellan is to think of the way in which a printer may be discovered and utilized. For example: consider needing to print a document from a desktop or mobile device. Simply opening a printer browser on a network-connected device displays a list of available printers, their capabilities (color printing, double-sided printing, etc.), and their current state such as *busy*, *standby mode*, *out of paper*, *print queue length*, and so on. The user can then select the printer they want to use and print their document with the minimal amount of trouble.

In the same way, Magellan proposes to discover two-way radio assets available on a particular network, present those assets to the user, and offer the user the option to utilize those assets. This discovery takes the form of service announcements made by gateway devices connected to the radio systems or by newer IP enabled radios themselves. Service announcements include sufficient information for users receiving those announcements in order to decide whether they wish to use the asset - very much like discovering available printers.

However, the design of Magellan goes beyond this discovery capability by placing control of who may use the radio system (and how) in the hands of the gateway or IP enabled radio itself - effectively turning the gateway or IP enabled radio into a ***gatekeeper***. This capability includes authentication of users wishing to utilize the radio assets as well as controlling the operations those authenticated users may carry out, such as listen-only or being granted transmit privileges. In addition, the gateway/gatekeeper also has control over other aspects of communications, such as managed floor control and transmit priority.

Furthermore, Magellan provides for encryption of the two-way radio traffic on the IP network so that, even if the network is compromised by a rogue entity, those entities will not be able to make sense of traffic on the network. In addition, Magellan also provides for individually tokenized transmit and receive capabilities to create “subchannels” of traffic within traffic streams. This effectively allows for *private* transmissions within already locked-down traffic streams.

### MAP
The above-described capability is implemented using JSON-based messages exchanged between user devices and gateways. These messages are secured with TLS on unicast and, if needed, DTLS in multicast environments. The combined protocol set is known as the ``Magellan Asset Protocol`` or ``MAP`` for short.

### Key Magellan Requirements & Elements
Key high-level elements and requirements for Magellan are as follows:
- **Asset Advertisement**: Advertisement utilizes industry-accepted protocols such as SSDP and mDNS/Bonjour/Avahi/NSD. These are well-known and proven protocols, have numerous stable implementations, and are incorporated into almost every operating platform in the market.
- **Authentication**: Authentication is based on mutual authentication principles utilizing industry-standard X.509 certificates.
- **Authorization & Privilege Grant**: Grant of authority and privileges is primarily driven by the contents of X.509 certificates exchanged between gateways/gatekeepers and their users. However, it is expected that individual practitioner requirements may require extension of this capability to delegation of decisions to central authority entities such as that practitioner’s LDAP or ActiveDirectory infrastructure.
- **Encryption**: Magellan utilizes government-approved encryption technologies such as Advanced Encryption Standard (AES) operating in Cipher Block Chaining mode with 128/192/256-bit key sizes, Transport Layer Security (TLS) v1.2, and other NIST/NSA/DSA-approved encryption suites necessary.
- **IP Addressing**: Magellan supports IP version 4 and IP version 6.
- **IP Networking**: Magellan is capable of operating on multicast and unicast IP networks.
- **Programming Language**: Magellan is primarily developed using the C++ language - specifically following the C++11 standard in order to be compiled by the broadest range of C++ compilers in the market.
- **Operating Systems and CPU Architectures**:  Magellan is POSIX-compliant and capable of being built for and running on standard desktop environments such as Microsoft Windows, Apple macOS, and common distributions of Linux. Magellan is also be for Android and Apple iOS platforms as well as select Linux-based embedded platforms. The platform support X86 32- and 64-bit CPU architectures as well as 32- and 64-bit ARM processors.
- **Delivery**: Magellan is delivered as pre-compiled binaries for the above-mentioned platforms as well as available in source code form from a publicly-accessible repository on Github. Included in the delivery package is appropriate documentation as well as sample code that third-parties may utilize to develop Magellan-based capabilities into their portfolio.
- **Review Process**: In addition to on-going development and design between the Magellen development team and a select group of practitioner representives, personnel; it is envisioned that the design and implementation methodology for Magellan will be formally submitted into the ``Request For Comments`` (RFC) process.

### Reference Implementation Notes
1. The reference implementation (`libmagellan`) requires certificate and key files in `PEM` storage format.  Practioners are welcome to modify this requirement to use other formats such as PKCS#12.

2. The [curl](https://curl.haxx.se) library is used by `libmagellan` to interact with gateway devices using REST.  On Linux systems, libmagellan uses the standard curl library provided with the Linux distro.  On Windows, the curl implementation is a pre-compiled build using Microsoft's `schannel` module. Practioners are welcome to replace curl with their own implementations.

# Development Tools
Tools and platforms used in the development and testing of the Magellan software includes the
following:
- Ubuntu Linux 19.10
- Visual Studio Code
- C++11 compliant GCC or CLang variant
  - all warnings enabled and warnings treated as errors by the compiler
  - Google Address Sanitization library (wherever possible)
- Valgrind
- Git
- CMake build system

# Organization Of This Repository

```
\---|
    |--- src        Contains source code for the Magellan library and test applications
    |--- mth        Configuration and script to run the Magellan Test Harness (mth)
    |--- sim        A simulated gateway that supports Magellan
    |--- certs      X.509 certificates and keys used by mth and the simulator
    |--- doc        Documentation
```

>Most directories contain a README file that further describes the contents of that directory.

# Building
Once you have the above-mentioned development tools in place, you simply need to run the ``build.sh`` script in the ``src`` directory.

```shell
$ ./build.sh
```

Artifacts are written to the ``src/.build`` directory.
