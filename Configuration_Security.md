The configuration file allows setting the SSL certificate files to be used for TLS and WSS signaling protocols. The certificates are also used for DTLS-SRTP. The Doubango framework must be built with [OpenSSL](https://code.google.com/p/telepresence/wiki/Support_BuildingSourceCode#Building_OpenSSL) enabled as explained [here](Support_BuildingSourceCode#Building_OpenSSL.md).
```
ssl-private-key = /tmp/ssl.pem
ssl-public-key = /tmp/ssl.pem
ssl-ca = /tmp/ssl.pem
ssl-mutual-auth = no
```
_Configuration 3: Setting SSL certificates_

  * **ssl-private-key** - the full path to the PEM file.
  * **ssl-public-key** - the full path to the PEM file.
  * **ssl-ca** - the full path to the PEM file.
  * **ssl-mutual-auth** - whether the incoming connection requests must fail if the remote peer certificates are missing or do not match the local ones. This only applies to **TLS** or **WSS** and is useless for **DTLS-SRTP** as certificates are always required.

The configuration file also allows setting the _SRTP_ type.
```
srtp-mode = optional
srtp-type = sdes;dtls
```
_Configuration 4: SRTP settings_

**srtp-mode** – defines the SRTP mode to use for negotiation. Supported values are <u>none</u>, <u>optional</u> and <u>mandatory</u>. Only optional and mandatory modes will work if the SIP client is a WebRTC browser as SRTP is required.
Based on the mode, the SDP on the outgoing INVITEs will be formed like this:
  * <u>none</u>:
    * profile will be equal to _RTP/AVP_
    * no crypto lines or certificate fingerprints will be added
  * <u>optional</u>:
    * profile will be equal to _RTP/AVP_
    * two crypto lines will be added if **srtp-type** includes <u>sdes</u>, plus certificate fingerprints if **srtp-type** also includes ‘dts’.
  * <u>mandatory</u>:
    * profile will be equal to _RTP/SAVP_ if **srtp-type** is equal to <u>SDES</u> or _UDP/TLS/RTP/SAVP_ if **srtp-type** is equal to <u>dtls</u>
    * two crypto lines will be added if **srtp-type** is equal to <u>sdes</u> or certificate fingerprints if **srtp-type** is equal to <u>dtls</u>

**srtp-type** - defines the list of all supported SRTP types. Defining multiple values only make sense if the srtp-mode value is equal to optional which means we want to negotiate the best one. Supported values are <u>sdes</u> and <u>dtls</u>.

_DTLS-SRTP_ requires valid SSL certificates and Doubango source code must be compiled with [OpenSSL](https://code.google.com/p/telepresence/wiki/Support_BuildingSourceCode#Building_OpenSSL) version 1.0.1 or later.