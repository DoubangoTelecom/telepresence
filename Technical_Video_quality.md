This section explains how to make sure to have a good video quality when using the system.



# Packet loss recovery #
This feature requires the jitter buffer to be enabled.
When a RTP packet is lost then, we request the remote party to send it again using <b>RTCP-NACK</b>s as per [RFC 5104](http://tools.ietf.org/html/rfc5104). Support for this feature is indicated using the SDP (attribute _a=rtcp-fb: `*` nack_). If the remote peer cannot honor the request in a reasonable delay then, the packet is considered as definitely lost. If we’re very lucky then, losing a packet would just introduce artifacts. Otherwise (not lucky), this will break the prediction chain and any attempt to decode the video would fail until the next IDR frame. Enabling zero-artifacts feature would fix artifact and prediction issues.
Increasing the internal RTP buffer size as explained [here](Configuration_RTP_buffer_size.md) could also help to lower packet loss.

# Jitter buffer #
The video jitter buffer could be enabled as explained [here](Configuration_Video#Jitter_Buffer.md).
Enabling the video jitter buffer introduce small delay (<b>~100ms</b>) but worth it. Buffering the video packets allows requesting missing packets using <b>RTCP-NACK</b> ([RFC 5104](http://tools.ietf.org/html/rfc5104)) and reordering them based on the RTP sequence numbers. It’s also up to the jitter buffer to consume any delay or burst to have smooth video according to the frame rate. The video frame rate is negotiated using the SDP but this value will be updated based on the RTP timestamps.

# Zero-artifacts #
This feature is enabled or disabled as explained [here](Configuration_Video#Zero-artifacts.md).
A video stream contains artifacts when some RTP packets are lost. The MCU try its best to avoid packet loss (see [here](Technical_Video_quality#Packet_loss_recovery.md)) but sometimes it fails and this leads to visual artifacts. As the video streams are mixed then, the artifacts will be propagated to all endpoints. When ‘zero-artifact’ is enabled then, the MCU pauses the rendering on the stream and sends <b>RTCP-FIR</b> ([RFC 5104](http://tools.ietf.org/html/rfc5104)) to request new <b>IDR</b> frame to repair the prediction chain. Only the stream with the missing RTP packets is paused until the next <b>IDR</b> frame is received. Support for <b>RTCP-FIR</b> is signaled to the remote endpoints using the SDP (attribute _a=rtcp-fb: `*` fir_). The MCU sends <b>IDR</b> frames when it receives <b>RTCP-FIR</b> or <b>RTCP-PLI</b> from one of the endpoints.

# AVPF tail length #
As already explained, <b>RTCP-NACK</b>s are used to ask a peer to send a packet again. In order to be able to honor these requests we need to save the outgoing RTP packets in a queue. The <b>AVPF</b> tail length option defines the minimum and maximum lengths for the queue. The higher these values are the better will be the video quality. The default queue length will be equal to the minimum value and it’s up to the MCU to increase this value depending on the number of unrecoverable packet loss. The final value will be at most equal to the maximum defined in the configuration file. Unrecoverable packet loss occurs when the MCU receives an <b>RTCP-NACK</b> for an already removed sequence number (very common when network <b>RTT</b> is very high or bandwidth very low).
Setting the <b>AVPF</b> tail length (min, max) is done as explained [here](Configuration_AVPF_tail_length.md).

# FEC (Forward Error Correction) #
--This section intentionally left blank--

# RED (Redundant video data) #
--This section intentionally left blank--