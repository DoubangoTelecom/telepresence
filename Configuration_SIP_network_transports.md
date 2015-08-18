The system supports many network transports for the SIP signaling protocol and content sharing. The network connections are declared using **transport** configuration entries.
```
transport = udp;*;20060;*
transport = ws;*;20060;4
transport = wss;*;20062;*
transport = tcp;*;20063
transport = tls;*;20064;*
transport = http;*;20065
transport = https;*;20066

```
_Configuration 2: Signaling (SIP) transports_

**Format**: <u>protocol-value</u>;<u>ip-address-value</u>;<u>port-value</u>;<u>ip-version-value</u>.

  * <u>protocol-value</u> must be **udp** , **tcp** , **tls** , **ws**, **wss**, **http** or **https**. **ws** protocol defines WebSocket and **wss** the secure version (requires OpenSSL). At least one WebSocket transport must be added to allow a web browser (WebRTC SIP client) to connect to the system. The other protocols (tcp, tls and udp) are used for SIP-legacy devices or PSTN.
  * <u>local-ip-value</u> is any valid IPv4/IPv6 address or FQDN. Use star (`*`) to let the system choose the best local IP address to bind to. Examples: udp;`*`;5060 or ws;`*`;5061 or wss;192.168.0.10;5062
  * <u>local-port-value</u> is any unused local port to bind to. Use star (`*`) to let the system choose the best unused port to bind to. Examples: udp;`*`;`*` or ws;`*`;`*` or wss;`*`;5062
  * <u>ip-version-value</u> defines the IP version to use. Must be **4**, **6** or <b><code>*</code></b>. Star (`*`) is used to let the system choose the best one. Using star (`*`) only make sense if local-ip-value is a FQDN instead of IP address.
A transport configuration entry must have at least a protocol, IP address (or star) and port (or star). The IP version is optional.<br />
**udp** , **tcp** , **tls** , **ws** and **wss** protocols are used to transport SIP messages while **http** and **https** are used to upload presentations.