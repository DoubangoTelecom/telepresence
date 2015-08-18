This section explains how to use settings related to the video.



# Bandwidth and congestion control #
There are two kinds of video bandwidths: <u>upload</u> and <u>download</u>.

<u>Upload</u>: Bandwidth (kbps) used by the video stream (RTP + RTCP) from the MCU to a single endpoint.

<u>Download</u>: Bandwidth (kbps) used by the video stream (RTP + RTCP) from the one endpoint to the MCU. This user-defined value will be forwarded to the remote endpoint using the SDP and RTCP-REMB and it’s up to this one to respect it or not. For more technical information, check [here](Technical_Bandwidth_management_and_congestion_control.md).

The configuration file allows setting the maximum upload and download bandwidths to use. If these values are undefined then, the upload bandwidth is computed following this formula:
```
video-max-upload-bandwidth (kbps) = ((video-width * video-height * video-fps * motion-rank * 0.07) / 1024)
```
For example, 720P video stream @15 frames per second with medium (2) motion rank will consume <b>1280<code>*</code>720<code>*</code>15<code>*</code>2<code>*</code>0.07 = 1935360 bps = ~1890 kbps</b> unless _video-max-upload-bandwidth_ entry is defined.
```
Congestion-ctrl-enabled = yes
video-max-upload-bandwidth = -1 # in kbps, <=0 means undefied
video-max-download-bandwidth = -1 # in kbps, <=0 means undefied
video-motion-rank = 2 # 1(low), 2(medium) or 4(high)
video-fps = 15 # [1 - 120]
```
_Configuration 12: Setting video bandwidth and congestion control_

  * **Congestion-ctrl-enabled** – whether to enable [draft-alvestrand-rtcweb-congestion-03](http://tools.ietf.org/html/draft-alvestrand-rtcweb-congestion-03) and [draft-alvestrand-rmcat-remb-01](http://tools.ietf.org/html/draft-alvestrand-rmcat-remb-01). Check [here](Technical_Bandwidth_management_and_congestion_control.md) for more information.
  * **video-max-upload-bandwidth** - defines the maximum bandwidth (kbps) to use for outgoing video stream (per endpoint). If congestion control is enabled then, the bandwidth will be updated based on the network conditions but these new values will never be higher than what you defined in your configuration file
  * **video-max-download-bandwidth** - defines the maximum bandwidth (kbps) to use for incoming video stream (per endpoint). If congestion control is enabled then, the bandwidth will be updated based on the network conditions but these new values will never be higher than what you defined in your configuration file
  * **video-motion-rank** - defines the video type. Supported values: <u>1</u> (low, e.g. home video security systems), <u>2</u> (medium, e.g conference call) or <u>3</u> (high, e.g. basketball game).
  * **video-fps** - defines the video framerate for the mixed stream regardless the input fps. Supported values: <u><code>[</code>1 – 120<code>]</code></u>.

  1. To check available bandwidth: [http://www.speedtest.net/](http://www.speedtest.net/)
  1. To check bandwidth usage: [iftop](http://linux.die.net/man/8/iftop).

# Output size, pixel aspect ratio and letterboxing #
The output (MCU → endpoints) mixed video size is independent of the input sizes (from the endpoints). **video-mixed-size** configuration entry is used to set the preferred value.

Accepted values are:
  * sqcif(128x98)
  * qcif(176x144)
  * qvga(320x240)
  * cif(352x288)
  * hvga(480x320)
  * vga(640x480)
  * 4cif(704x576)
  * svga(800x600)
  * 480p(852x480)
  * 720p(1280x720)
  * 16cif(1408x1152)
  * 1080p(1920x1080)
  * 2160p(3840x2160)
If no value is defined then, the mixed video size is assumed to be equal to <u>vga</u> (640x480).
<u>720p</u>, <u>1080p</u> and <u>2160p</u> are commonly named HD, Full HD and Ultra HD.
```
video-mixed-size = vga
```
_Configuration 13: Setting output mixed video size_

In this beta version, it’s not allowed to set arbitrary values because of backward compatibility. The final version will probably allow this. <br />
To draw the speaker and listeners video on the output mixed video we need to resize these frames to feet the destination. The video frames are linearly resized following a specific [Pixel Aspect Ratio (PAR)](http://en.wikipedia.org/wiki/Pixel_aspect_ratio) before being [letterboxed](http://en.wikipedia.org/wiki/Letterboxing_(filming)).
```
video-speaker-par = 0:0
video-listener-par = 1:1
```
_Configuration 15: Setting Pixel Aspect Ratio_<br />
A PAR equal to _1:1_ means _“skip the linear resizing”_ and a value of _0:0_ means _“skip both linear resizing and letterboxing”_. <br />
Common PAR values: _1:1_, _16:9_ or _4:3_.


# Jitter Buffer #
**video-jb-enabled** configuration entry is used to enable or disable video jitter buffer. It's highly recommended to enable video jitter buffer because it's required to have RTCP-FB (NACK, FIR, PLI... as per [RFC 5104](http://tools.ietf.org/html/rfc5104)) fully functional. Enabling video jitter buffer gives better quality and improves smoothness. For example, no RTCP-NACK messages will be sent to request dropped RTP packets if this option is disabled. It’s also up to the jitter buffer to reorder RTP packets.
For more technical information, check [here](Technical_Video_quality#Jitter_buffer.md).
```
video-jb-enabled = yes # no to disable
```
_Configuration 14: Enabling or disabling video jitter buffer_

# Zero-artifacts #
It’s up to the MCU to decode all video streams from the endpoints, mix them before sending the result. If RTP packets are lost on one stream then, artifacts will be introduced on the mixed frame (result). Enabling zero-artifact feature fix this issue. There are some requirements on the endpoints to have this feature fully functional.
For more technical information, check [here](Technical_Video_quality#Zero-artifacts.md).
```
video-zeroartifacts-enabled = yes # no to disable
```
_Configuration 15: Enabling or disabling zero-artifact_

# Mixing type #

_--This section intentionally left blank--_

# Overlays #
The configuration file allows managing the overlays (font size and type, position, watermark…).
```
overlay-fonts-folder-path = ./fonts/truetype/freefont
overlay-copyright-text = Doubango Telecom
overlay-copyright-fontsize = 12
# full path: ./fonts/truetype/freefont/FreeSerif.ttf
overlay-copyright-fontfile = FreeSerif.ttf
overlay-speaker-name-enabled = yes
overlay-speaker-name-fontsize = 16
# full path: ./fonts/truetype/freefont/FreeMonoBold.ttf
overlay-speaker-name-fontfile = FreeMonoBold.ttf
overlay-speaker-jobtitle-enabled = yes
overlay-watermark-image-path = ./images/logo35x34.jpg
```
_Configuration 16: Video overlays_

  * **overlay-fonts-folder-path** - defines the base folder path where to look for the font types. The default fonts come from [ftp://ftp.gnu.org/pub/gnu/freefont](ftp://ftp.gnu.org/pub/gnu/freefont). For more fonts (not free), we recommend [http://www.dafont.com/](http://www.dafont.com/).
  * **overlay-copyright-text** - defines the copyright text to display on the mixed video. Comment the line to disable this feature.
  * **overlay-copyright-fontsize** - defines the font size to use to draw the copyright text.
  * **overlay-copyright-fontfile** - defines the font file to use to draw the copyright text on the mixed video. The full path to the TruType file will be ‘overlay-fonts-folder-path+"/"+overlay-copyright-fontfile’.
  * **overlay-speaker-name-enabled** - whether to draw the speaker's name on the mixed video.
  * **overlay-speaker-name-fontsize** - defines the font size to use to draw the speaker's name (and job title) on the mixed video.
  * **overlay-speaker-name-fontfile** - defines the font file to use to draw the speaker's name on the mixed video. The full path to the TruType file will be ‘overlay-fonts-folder-path+"/"+overlay-speaker-name-fontfile’.
  * **overlay-speaker-jobtitle-enabled** - whether to draw the speaker's job title on the mixed video.
  * **overlay-watermark-image-path** - defines the full path to the image to use to watermark the mixed video. Comment the line to disable this feature.

For more technical information, check [here](Technical_Video_mixer_design#Overlays.md).

To test test the overlays we highly recommended using the [WebRTC Telepresence client](http://conf-call.org/).

# Patterns #

_--This section intentionally left blank--_