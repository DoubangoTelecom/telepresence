This section shows how to enable or disable symmetric RTP (technical details [here](Technical_NAT_and_firewall_traversal#Symmetric_RTP.md)), ICE (technical details [here](Technical_NAT_and_firewall_traversal#ICE.md)) and RTCP-MUX (technical details [here](Technical_NAT_and_firewall_traversal#RTCP-MUX.md)).
```
rtp-symmetric-enabled = yes # no to disable
ice-enabled = yes # no to disable
icestun-enabled = yes
stun-server = stun.l.google.com;19302;stun-user;stun-password
rtcp-mux-enabled = yes # no to disable
```
_Configuration 6: Enabling/disabling NAT traversal features_

  * **rtp-symmetric-enabled** - whether to enable symmetric RTP ([RFC 4961](http://tools.ietf.org/html/rfc4961)) for NAT and firewall traversal
  * **ice-enabled** - whether to enable ICE ([RFC 5245](http://tools.ietf.org/html/rfc5245)) for NAT and firewall traversal.
  * **icestun-enabled** - whether to use STUN to gather reflexive addresses or not. This option is useful when the server is on a public network or all peers are on the same local network. In these cases, disabling STUN for ICE will speed up the call setup. Disabling icestun is also useful when the system is installed on a PC without access to internet.
  * **stun-server** - defines the STUN/TURN server to use to gather reflexive addresses for the ICE candidates. If no server is defined then, a default one will be used. The default STUN/TURN server is **numb.viagenie.ca:3478**.
    * Format: <u>server-fqdn-value</u>; <u>server-port-value</u>; <u>user-name-value</u>; <u>user-password-value</u>
      * <u>server-fqdn-value</u>: A valid IPv4/v6 address or host name.
      * <u>server-port</u>: A valid port number.
      * <u>user-name-value</u>: The login to use for TURN authentication. Use star (`*`) to ignore.
      * <u>user-password-value</u>: The password to use for TURN authentication. Use star (`*`) to ignore.
  * **rtcp-mux-enabled** - whether to enable RTC-MUX ([RFC 5761](http://tools.ietf.org/html/rfc5761)).