As a developer, the first action is to edit your configuration file to change the debug-level from **ERROR** to **INFO**.

When you connect to the MCU you always have video stream back (you see yourself) but your audio stream is never sent back. For debugging purposes, it could be useful to ask your audio stream back. Hearing your own sound helps testing that everything work. Without loopback audio, you must connect at least two endpoints to test audio (encoding, decoding, streaming, resampling…).
```
debug-level = INFO
debug-audio-loopback = yes
```
_Configuration 1: Useful debug settings_

**debug-level** - defines the minimum debug level to display. Supported values: INFO, WARN, ERROR and FATAL.

**debug-level-loopack** - whether to enable audio loopback for testing (see above for more information).

We require having your **debug-level** equal to **INFO** when reporting/sharing issues.