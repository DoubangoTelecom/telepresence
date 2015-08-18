This section explains how to secure both signaling and media plans.



# Signaling #
Two secure signaling protocols are supported: **TLS** and **WSS**. **WSS** is WebSocket secured using **TLS**. For more information on how to enable these transport protocols using the configuration file, please refer to [this wiki page](Configuration_SIP_network_transports.md).

Both transports require [OpenSSL](https://code.google.com/p/telepresence/wiki/Support_BuildingSourceCode#Building_OpenSSL) which have to be enabled when building the Doubango framework only.

More information on how to configure the SSL certificates could be found [here](Configuration_Security.md).

# Media #
Both **SRTP-SDES** ([RFC 4568](http://tools.ietf.org/html/rfc4568)) and **SRTP-DTLS** ([RFC 5763](http://tools.ietf.org/html/rfc5763), [RFC 5764](http://tools.ietf.org/html/rfc5764)) are supported.

Check [here](Configuration_Security.md) for more information on how these features have to be configured.

_--This section intentionally left blank--_