The video mixing and scaling is completely managed using FFmpeg. In this beta version, only 2D mixing is supported. We’ll consider support for [libyuv](https://code.google.com/p/libyuv/) in the next versions.



# Encoders and decoders #
To minimize CPU utilization, all endpoints with the same video codec use the same encoder but different decoders. For example, if you have 7 endpoints using VP8 codec then, the MCU will have 1 encoder and 7 decoders. Using a single encoder gives better CPU performances but more bandwidth than what’s needed because if one endpoint requests an IDR then, the prediction chain is restarted for all other peers.

# Overlays #
The overlays are configured as explained [here](Configuration_Video#Overlays.md).

The mixed video contains many overlays displayed using [FFmpeg filters](http://www.ffmpeg.org/ffmpeg-filters.html). There are two kinds of overlays: texts and images. The text overlays use [drawtext filter](http://www.ffmpeg.org/ffmpeg-filters.html#drawtext-1) which requires [libfreetype](https://code.google.com/p/telepresence/wiki/Support_BuildingSourceCode#Building_libfreetype) to be enabled when building FFmpeg. The image overlays (watermarks) use [movie filter](http://www.ffmpeg.org/ffmpeg-filters.html#movie-1) and accept any PNG or JPEG file as input. JPEG images are natively supported by FFmpeg (thanks to MPEG4) while PNG requires [zlib](http://zlib.net/) to be enabled.

The configuration file could be used to define custom font types for the text overlays. The font types must be [TrueType](https://en.wikipedia.org/wiki/TrueType)-compatible. The default fonts come from [ftp://ftp.gnu.org/pub/gnu/freefont](ftp://ftp.gnu.org/pub/gnu/freefont).