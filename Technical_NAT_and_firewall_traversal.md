This section explains how to tackle NAT and firewall traversal issues.



# Symmetric RTP #
This feature is enabled or disabled as explained [here](Configuration_NAT_Firewall_traversal.md).

The Telepresence system fully supports [RFC 4961](http://tools.ietf.org/html/rfc4961).

An RTP/RTCP stream is symmetric if the same port is used to send and receive packets. This helps for NAT and firewall traversal as the outgoing packets open a pinhole for the ongoing ones.

The local/outgoing stream (MCU → endpoints) is always symmetric.

If both parties (remote and local) have successfully negotiated ICE candidates then, none will be forced to use symmetric RTP/RTCP.

Let’s imagine your Telepresence instance is on a public network and the SIP client/endpoint on private network:
  1. Telepresence: Public IP address is **1.1.1.1**
  1. Client: Private IP address is **2.2.2.2** and public IP address is 1.1.1.2
  1. The SDP from the client to the Telepresence system will contain client’s private IP address (**2.2.2.2**) which is not reachable
  1. The RTP/RTCP packets from the client to the server will be received with source IP address equal to the client’s public IP address (**1.1.1.2**)
  1. If [rtp-symetric-enabled](Configuration_NAT_Firewall_traversal.md) option is used then, the Telepresence system will send RTP/RTCP packets to **1.1.1.2** (learnt from the received packets) instead of **2.2.2.2** which is private and unreachable.

# ICE #
This feature is enabled or disabled as explained [here](Configuration_NAT_Firewall_traversal.md).

The Telepresence system fully supports [RFC 5245](http://tools.ietf.org/html/rfc5245).

ICE is negotiated only if this feature is enabled and incoming SDP (SIP endpoint → MCU) contains candidates.

**ICE is mandatory for WebRTC endpoints**.

# RTCP-MUX #
This feature is enabled or disabled as explained [here](Configuration_NAT_Firewall_traversal.md).

The Telepresence system fully supports [RFC 5761](http://tools.ietf.org/html/rfc5761).

RTCP-MUX is used to minimize the number of ports and help for NAT traversal and administration.