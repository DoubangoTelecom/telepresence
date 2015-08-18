This section explains how to test the Telepresence system.

As already explained on this document, any SIP client can be used to connect to the Telepresence system but we highly recommend using our WebRTC client. The WebRTC client is hosted at [http://conf-call.org/](http://conf-call.org/).

Few steps to get started (we assume you have successfully built and installed the system):
  1. Build, install and run the system as explained [here](Support_BuildingSourceCode.md).
  1. Change (in your configuration file) the _debug level_ value to _INFO_ as explained [here](Configuration_Debugging_the_system.md) (not required).
  1. The default configuration already has a WebSocket transport listening on port 20060 but the IP address is automatically retrieve at the startup (because of the star (`*`) in the transport configuration entry). To bind to a specific IP address, change your settings as explained [here](Configuration_SIP_network_transports.md).
  1. Starts the system and check the console logs to be sure that all is ok. The logs also show which IP addresses and port are used for each protocol (WS, WSS, TCP, TLS and UDP).
  1. Go to [http://conf-call.org/settings.htm](http://conf-call.org/settings.htm) and enter your <u>Private Identity</u> and the <u>WebSocket connection URL</u>. The Private Identity is a SIP authentication name (**without special characters or SPACEs**). The connection URL must be the same the transport entry defined in the configuration file (the star (`*`) after the protocol name -scheme- must be replaced with a valid IP address or host name). Save your settings.
  1. Go back to the home page, enter a bridge identifier (any number would work) and press **"join"** button. If a bridge with this identifier doesn’t exist then, it will be created automatically. Any person calling the same identifier will be part of the conference. No pin code is required unless one is defined in the configuration file.

  * To report issues: [https://groups.google.com/group/opentelepresence](https://groups.google.com/group/opentelepresence).
  * For issues related to bandwidth, please attach results from [http://www.speedtest.net/](http://www.speedtest.net/).