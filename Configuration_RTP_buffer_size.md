**rtp-buffersize** configuration entry is used to define the internal buffer size to use for RTP sockets. The higher this value is the lower will be the RTP packet loss. Please note that the maximum value depends on your system (e.g. 65535 on Windows). A very high value could introduce delay on video stream and it’s highly recommended to also [enable video jitter buffer option](Technical_Video_quality#Jitter_buffer.md).

Code usage:
```
setsockopt(SOL_SOCKET, SO_RCVBUF, rtp-buffsize-value);
setsockopt(SOL_SOCKET, SO_SNDBUF, rtp-buffsize-value);
```
Configuration:
```
rtp-buffersize = 65535
```
_Configuration 7: Setting RTP buffer size_