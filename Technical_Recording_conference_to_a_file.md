This feature is configured as explained [here](Configuration_Recording_conference_to_a_file.md).

We support almost any container depending on how you built _FFmpeg_ and which codecs are enabled. Right now we recommend only **.avi** and **.mp4** as they are fully tested. **.mkv** and **.webm** will also work but not fully tested yet. The audio and video mixing is done using _libavformat_ from FFmpeg.

  * _**.avi**_ (<b>recommended</b>) requires FFmpeg with **MPEG4** video codec and **AC3** audio codec
  * _**.mp4**_ requires FFmpeg with **H.264** ([libx264](https://code.google.com/p/telepresence/wiki/Support_BuildingSourceCode#Building_x264) thirdparty library) video codec and AAC audio codec. There is a built-in experimental **AAC** codec in FFmpeg but the code is intentionally designed to not accept such codec because of random crashes. For AAC audio codec, you’ll need to build FFmpeg with [libfaac](https://code.google.com/p/telepresence/wiki/Support_BuildingSourceCode#Building_libfaac) or any other [third-party AAC library](http://ffmpeg.org/trac/ffmpeg/wiki/AACEncodingGuide). Please note that all AAC libraries are not free.
  * _**.avi**_ is recommended instead of **.mp4** for the simple reason that the first one consume less CPU.
The output file will have the bridge identifier as name and container type as extension, e.g. _+336000000.avi_. The file is locked and invalid until the last user quit the bridge.

We highly recommend using [VLC](http://www.videolan.org/vlc/index.html) to play the output files.