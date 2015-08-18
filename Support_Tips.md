

## 1. Lowering CPU ##
### 1.1 Avoid audio resampling ###
To avoid audio resampling, the SIP clients connecting to a bridge have to use a codec with the sample rate, channels and bits per sample as the ["pivot settings"](Technical_Audio_mixer_design.md). For more information, check [here](Technical_Audio_mixer_design.md). <br />
<u><b>tip:</b></u> In you configuration file, enable codecs with same settings as the pivot.

### 1.2 Only record sessions if needed ###
Do not enable recording if it’s not important to you or use `*`.avi container which consume less CPU than `*`.mp4 (because of AAC encoder from [libfaac](Support_BuildingSourceCode#Building_libfaac.md)).

### 1.3 Use common video codec ###
All SIP clients with the same video codec will share a single encoder. Try to use common video codec for all clients. For example, if you have two clients, A and B, with A supporting both H.264 and VP8 and B only H.264 then, you should make sure that A will offer H.264 with highest priority. For more information, check [here](Technical_Video_mixer_design#Encoders_and_decoders.md). <br />
<u><b>tip:</b></u> In your configuration file, enable a single video codec if you cannot control the SIP clients.

### 1.4 Use 2d audio mixing ###
Enable 2D audio mixing instead of 3D.

### 1.5 Lower mixed video size and fps ###
If you have a weak CPU then, consider using a reasonable video size (e.g. VGA) and fps ([- 30](15.md)).

### 1.5 Multi-threading and ASM ###
Make sure to enable [YASM](Support_BuildingSourceCode#Building_YASM.md) and pthread when building [FFmpeg](Support_BuildingSourceCode#Building_FFmpeg.md), [x264](Support_BuildingSourceCode#Building_x264.md) and [VP8](Support_BuildingSourceCode#Building_libvpx.md).

## 2. Lowering bandwidth ##
  * Use "slow" motion rank (see [here](Configuration_Video#Bandwidth_and_congestion_control.md))
  * Use small mixed video size (see [here](Configuration_Video#Output_mixed_size.md))
  * Set the maximum upload and download bandwidth (see [here](Configuration_Video#Bandwidth_and_congestion_control.md))
  * Use small video frame rate (see [here](Configuration_Video#Bandwidth_and_congestion_control.md))

<u><b>tip:</b></u> To test your available bandwidth, we recommend http://www.speedtest.net/. <br />
<u><b>tip:</b></u> To check bandwidth usage, we recommend iftop.

## 3. Improving audio quality ##
  * Use [Opus](Support_BuildingSourceCode#Building_libopus.md) (or G.722) audio codec if supported by the SIP clients (see [here](Configuration_Audio_Video_codecs.md)).
  * Avoid audio up-sampling and down-sampling (see [here](Technical_Audio_mixer_design.md)).
  * If the ["pivot settings"](Technical_Audio_mixer_design.md) use a sample rate (SR) equal to S then, try to use codecs with a SR equal to "S << n" or "S >> n".

## 4. Improving video quality ##
  * Use Google Chrome as SIP client (check our WebRTC demo client at [http://conf-call.org/](http://conf-call.org/)).
  * Enable "Zero-Artifacts" feature (see [here](Technical_Video_quality#Zero-artifacts.md) and [here](Configuration_Video#Zero-artifacts.md))
  * Use a client supporting something close to 16/9 video size to avoid stretching issues
  * Avoid video up-sampling and down-sampling

## 5. Lowering recorded video file size ##

_--This section intentionally left blank--_