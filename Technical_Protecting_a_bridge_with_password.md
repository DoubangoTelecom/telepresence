This feature is configured as explained [here](Configuration_Bridge_configuration.md).

There are two ways for a SIP client to authenticate to a protected bridge: DTMF or **TP-BridgePin** SIP header. If authentication fails, a SIP 403 response will be returned with a short description.

The **DTMF** method doesn’t require changing your SIP client but is not supported yet in this beta version (on the roadmap for the release version). The second method (using the SIP header) requires some modifications on you SIP client to include this new header. If you are using our [WebRTC SIP telepresence demo client](http://conf-call.org/) then, no modification is needed.