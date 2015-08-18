The configuration file is used to specify whether to record the sessions, which container to use and where to store the output file.
```
record = yes
record-file-ext = mp4
```
_Configuration 10: Recording conference to a file_

  * **record** - whether to record the sessions.
  * **record-file-ext** - defines the container to use. Almost any container (<u>avi</u>, <u>mp4</u>, <u>webm</u>, <u>mkv</u> …) could be used but this depends on how you built FFmpeg. For more technical information, please check [here](Technical_Recording_conference_to_a_file.md).
We highly recommend using [VLC](http://www.videolan.org/vlc/index.html) to play the output file.