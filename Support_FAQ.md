

## How to report issue? ##
We receive tens of messages on our dev-groups and private mailboxes every day. Without a commercial license, we only provide a best-effort support on [opentelepresence group](http://groups.google.com/group/opentelepresence). <br /> **We're happy to help you to fix your issues** but we'll not spend hours on them to understand what's wrong. If you want help, you **must**:
  * Provide clear technical description of the issue.
  * Change you _telepresence.cfg_ to use INFO debug level. See [here](Configuration_Debugging_the_system.md) for more information.
  * Attach the **full** logs to the message.
  * Provide information about the SVN revisions (both Doubango and TelePresence).
If the report is about building issues:
  * Attach config.log files for both Doubango and TelePresence
If the report is about video issues:
  * Attach results from http://www.speedtest.net/
you _should_ also:
  * Provide the network (Wireshark) capture at the server-side
  * Provide the client logs.

## How to create self-signed SSL certificates? ##
We'd recommend reading [this thread](https://groups.google.com/forum/#!topic/doubango/asAfP5ZCgdI).

## I see "error while loading shared libraries: XXX.so.y: cannot open shared object file: No such file or directory" when I run the server. How can I fix this? ##
When a program is started the loader search for all dependencies (shared libraries) in a predefined list of folders. You're getting this error because the folder containing _XXX.so.y_ is not in the list. To fix the issue, run:
```
# /!\IMPORTANT: change <path-to-XXX.so.y>
echo "<path-to-XXX.so.y>" >> /etc/ld.so.conf.d/telepresence.conf
ldconfig
```
To find _<path-to-XXX.so.y>_:
```
find / -name 'XXX.so.y'
```


## I see "Remote party requesting DTLS-DTLS (UDP/TLS/RTP/SAVPF) but this option is not enabled". How can I fix this ##
DTLS-SRTP is required by some WebRTC implementations (e.g. Firefox). You <b>MUST</b>:
  1. use a new OpenSSL version with support for DTLS-SRTP as explained [here](Support_BuildingSourceCode#Building_OpenSSL.md). Linux almost always comes with OpenSSL pre-installed which means building and installing OpenSSL by yourself will most likely duplicate it.
  1. make sure you don't have more than one OpenSSL version installed (look for libssl).
  1. rebuild TelePresence and make sure the "CONGRATULATIONS" message says that you have DTLS-SRTP enabled.
  1. make sure you're using SSL certificates in your configuration (see [here](Configuration_Security.md) for more information). DTLS-SRTP requires at least a valid Public Key (could be self signed).