Source code freely provided to you by <a href='http://www.doubango.org'> Doubango Telecom ®</a>. This is a good and viable **open source** alternative to Google Hangouts.



<table>
<tr>
<td align='center'><a href='http://www.youtube.com/watch?feature=player_embedded&v=Yi3ufNso67U' target='_blank'><img src='http://img.youtube.com/vi/Yi3ufNso67U/0.jpg' width='425' height=344 /></a></td>
<td align='center'><img src='https://telepresence.googlecode.com/svn/trunk/images/presentation_sharing.png' /></td>
</tr>
<tr>
<td align='center'><i>Demonstration</i></td>
<td align='center'><i>Presentation sharing</i></td>
</tr>
</table>

## Main features ##
This is a short but not exhaustive list of supported features on this beta version:

  * Powerful [MCU (Multipoint Control Unit)](http://en.wikipedia.org/wiki/Multipoint_control_unit) for audio and video mixing
  * **Stereoscopic** (spatial) 3D and stereophonic audio
  * Full (**1080p**) and Ultra (**2160p**) HD video up to **120fps**
  * Conference **recording** to a file (containers: **.mp4**, **.avi**, **.mkv** or **.webm**)
  * Revolutionary way to **share presentations**: documents are "streamed" in the video channel to allow any SIP client running on any device to participate
  * Smart adaptive audio and video bandwidth management
  * Congestion control mechanism
  * SIP registrar
  * 4 SIP transports (**WebSocket**, **TCP**, **TLS** and **UDP**)
  * SA (direct connection to SIP clients) and AS (behind a server, such as [Asterisk](http://www.asterisk.org/), [reSIProcate](http://www.resiprocate.org/Main_Page), [openSIPS](http://www.opensips.org/), [Kamailio](http://www.kamailio.org/w/)…) modes
  * Support for any [WebRTC](http://www.webrtc.org/)-capable browser ([WebRTC demo client](http://conf-call.org/) at [http://conf-call.org/](http://conf-call.org/))
  * Mixing different audio and video codecs on a single bridge (**h264**, **vp8**, h263, mp4v-es, theora, **opus**, **g711**, speex, **g722**, gsm, **g729**, amr, ilbc)
  * **Protecting** a bridge with PIN code
  * **Unlimited** number of bridges and participants
  * Connecting **any SIP client** (Mobiles, Tablets, Desktops, Set-top-boxes, Smart TVs...)
  * Easy interconnection with **PSTN**
  * **NAT traversal** (Symmetric RTP, RTCP-MUX, ICE, STUN and TURN)
  * **RTCP Feedbacks** (NACK, PLI, FIR, TMMBN, REMB…) for better video experience
  * Secure signalling (WSS, TLS) and media (SDES-SRTP and DTLS-SRTP)
  * Continuous presence
  * Smart algorithm to detect speakers and listeners
  * Different video patterns/layouts
  * Multiple operating systems (**Linux**, **OS X**, **Windows** …)
  * 100% open source and free (no locked features)
  * [Full documentation](http://conf-call.org/technical-guide.pdf?svn=1)
  * …and many others

This short list is a good starting point to help you to understand what you could expect from our Telepresence system.

## Getting started ##
  1. Read the [technical guide](http://conf-call.org/technical-guide.pdf?svn=2) for more information on how to [build](Support_BuildingSourceCode.md), [install](Support_BuildingSourceCode#Installing_the_configuration_and_fonts_files.md) and run the system
  1. Test the system as explained [here](Support_Testing_the_system.md)
  1. Share issues and technical questions on our [developer group](https://groups.google.com/group/opentelepresence)
  1. Find our roadmap [here](Support_Roadmap.md)

Even if any SIP client could be used we highly recommend for this beta version to use our [WebRTC demo client](http://conf-call.org/) to ease debugging.

## Technical help ##
Please check our [issue tracker](http://code.google.com/p/telepresence/issues/list) or [developer group](https://groups.google.com/group/opentelepresence) if you have any problem. <br />
We highly recommend reading our [Technical guide](http://conf-call.org/technical-guide.pdf?svn=2). <br />
Please check the list of [known issues](Support_Known_issues.md) before reporting.