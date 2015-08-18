This is the most important part as it’s where you’ll decide which features to support/enable. The Telepresence system use [Doubango VoIP Framework](https://code.google.com/p/doubango/) (requires **SVN [r989](https://code.google.com/p/telepresence/source/detail?r=989) or later**) and it’s highly recommended to rebuild it if you already have an old version installed because of some new and required features.

This section explains how to build the product on **CentOS64** but could be easily adapted for any Linux, Windows ([MinGW](http://www.mingw.org/)) or OS X ([MacPorts](http://www.macports.org/)).

For any issue, please ask on the [telepresence developer group](https://groups.google.com/group/opentelepresence).



## Preparing the system ##
```
sudo yum update
sudo yum install make libtool autoconf subversion git wget cmake gcc gcc-c++ pkgconfig nasm
```

## Building thirdparties libraries ##

### Required libraries ###
  * Always: [libspeexdsp](Support_BuildingSourceCode#Building_libspeex_and_libspeexdsp.md), [FFmpeg](Support_BuildingSourceCode#Building_FFmpeg.md) and [Doubango](Support_BuildingSourceCode#Building_Doubango.md)
  * For [WebRTC](http://conf-call.org/) clients: [libspeexdsp](Support_BuildingSourceCode#Building_libspeex_and_libspeexdsp.md), [libsrtp](Support_BuildingSourceCode#Building_libsrtp.md), [libvpx](Support_BuildingSourceCode#Building_libvpx.md), [OpenSSL 1.0.1+](Support_BuildingSourceCode#Building_OpenSSL.md)(Firefox only), [FFmpeg](Support_BuildingSourceCode#Building_FFmpeg.md) and [Doubango](Support_BuildingSourceCode#Building_Doubango.md)
... all other libraries are optional.

### Building libsrtp ###
[libsrtp](http://srtp.sourceforge.net/srtp.html) is optional unless you want to use WebRTC SIP clients. It’s highly recommended. The [WebRTC Telepresence demo client](http://conf-call.org/) requires a system with SRTP enabled.
```
git clone https://github.com/cisco/libsrtp/
cd libsrtp
git checkout v1.5.0
CFLAGS="-fPIC" ./configure --enable-pic && make && make install
```
You should not use any libsrtp package because the latest dev version is required and building the source **by yourself is highly recommended**.

### Building OpenSSL ###
[OpenSSL](http://www.openssl.org/) is required if you want to use TLS, WSS (Secure WebSocket) or DTLS-SRTP (also requires libsrtp). **OpenSSL version 1.0.1 is required if you want support for DTLS-SRTP** which is mandatory for WebRTC implementation from Mozilla (Firefox Nightly or Aurora).<br />
This section is only required if you don’t have OpenSSL installed on your system or using version prior to 1.0.1 and want to enable DTLS-SRTP.
A quick way to have OpenSSL may be installing openssl-devel package but this version will most likely be outdated (prior to 1.0.1).
Anyways, you can check the version like this:
```
openssl version 
```
To build OpenSSL:
```
wget http://www.openssl.org/source/openssl-1.0.1c.tar.gz
tar -xvzf openssl-1.0.1c.tar.gz
cd openssl-1.0.1c
./config shared --prefix=/usr/local --openssldir=/usr/local/openssl && make && make install
```
Some known issues when you have more than one openssl libraries installed is discussed at [https://groups.google.com/forum/#!topic/opentelepresence/JctxtEyW-dg](https://groups.google.com/forum/#!topic/opentelepresence/JctxtEyW-dg) (_see comment 4_).

### Building libogg, libvorbis and libtheora ###
These libraries are optional unless you want to use **.webm** or **.mkv** containers.
You can install the devel packages (**recommended**):
```
sudo yum install libogg-devel libvorbis-devel libtheora-devel
```
Or build the source code by yourself:
```
--This section intentionally left blank--
```

### Building libspeex and libspeexdsp ###
[libspeex](http://www.speex.org/) (audio codec) is optional but [libspeexdsp](http://www.speex.org/) (audio resampler, jitter buffer…) is **required**.
You can install the devel packages:
```
sudo yum install speex-devel
```
Or build the source by yourself:
```
wget http://downloads.xiph.org/releases/speex/speex-1.2beta3.tar.gz
tar -xvzf speex-1.2beta3.tar.gz
cd speex-1.2beta3
./configure --disable-oggtest --without-libogg && make && make install
```

### Building YASM ###
[YASM](http://yasm.tortall.net/) is only required if you want to enable and build **VPX** (VP8 video codec) or **x264** (H.264 codec). It’s highly recommended.
```
wget http://www.tortall.net/projects/yasm/releases/yasm-1.2.0.tar.gz
tar -xvzf yasm-1.2.0.tar.gz
cd yasm-1.2.0
./configure && make && make install
```

### Building libvpx ###
[libvpx](http://www.webmproject.org/code/) adds support for [VP8](http://en.wikipedia.org/wiki/VP8) and is optional but highly recommended if you want support for video when using Google Chrome or Mozilla Firefox. libvpx is required if you want to use **.webm** container or our [WebRTC SIP Telepresence client](http://conf-call.org/).
You can install the devel packages:
```
sudo yum install libvpx-devel
```
Or build the source by yourself:
```
git clone http://git.chromium.org/webm/libvpx.git
cd libvpx
./configure --enable-realtime-only --enable-error-concealment --disable-examples --enable-vp8 --enable-pic --enable-shared --as=yasm
make && make install
```

### Building opencore-amr ###
[opencore-amr](http://sourceforge.net/projects/opencore-amr/) is optional. Adds support for AMR audio codec.
```
git clone git://opencore-amr.git.sourceforge.net/gitroot/opencore-amr/opencore-amr
cd opencore-amr && autoreconf --install && ./configure && make && make install
```

### Building libopus ###
[libopus](http://www.opus-codec.org/) is optional but highly recommended as it’s an MTI codec for [WebRTC](http://www.webrtc.org/). Adds support for Opus audio codec.
```
wget http://downloads.xiph.org/releases/opus/opus-1.0.2.tar.gz
tar -xvzf opus-1.0.2.tar.gz
cd opus-1.0.2
./configure --with-pic --enable-float-approx && make && make install
```

### Building libgsm ###
[libgsm](http://en.wikipedia.org/wiki/Full_Rate) is optional. Adds support for [GSM](http://en.wikipedia.org/wiki/Full_Rate) audio codec.
You can install the devel packages (**recommended**):
```
sudo yum install gsm-devel
```
Or build the source by yourself:
```
wget http://www.quut.com/gsm/gsm-1.0.13.tar.gz
tar -xvzf gsm-1.0.13.tar.gz
cd gsm-1.0-pl13 && make && make install
#cp -rf ./inc/* /usr/local/include
#cp -rf ./lib/* /usr/local/lib
```

### Building g729 ###
[G729](http://en.wikipedia.org/wiki/G.729) is optional. Adds support for G.729 audio codec.
```
svn co http://g729.googlecode.com/svn/trunk/ g729b
cd g729b
./autogen.sh && ./configure --enable-static --disable-shared && make && make install
```

### Building iLBC ###
[iLBC](http://en.wikipedia.org/wiki/Internet_Low_Bitrate_Codec) is optional. Adds support for iLBC audio codec.
```
svn co http://doubango.googlecode.com/svn/branches/2.0/doubango/thirdparties/scripts/ilbc
cd ilbc
wget http://www.ietf.org/rfc/rfc3951.txt
awk -f extract.awk rfc3951.txt
./autogen.sh && ./configure
make && make install
```

### Building x264 ###
[x264](http://www.videolan.org/developers/x264.html) is optional but highly recommended and adds support for H.264 video codec (requires FFmpeg). [x264](http://www.videolan.org/developers/x264.html) is required if you want to use **.mp4** container.
```
wget ftp://ftp.videolan.org/pub/x264/snapshots/last_x264.tar.bz2
tar -xvjf last_x264.tar.bz2
# the output directory may be difference depending on the version and date
cd x264-snapshot-20121201-2245
./configure --enable-shared --enable-pic && make && make install
```

### Building libfreetype ###
[libfreetype](http://www.freetype.org/) is **required** and used for video overlays.
You can install the devel packages (**recommended**):
```
sudo yum install freetype-devel
```
Or build the source by yourself:
```
wget http://download.savannah.gnu.org/releases/freetype/freetype-2.4.12.tar.bz2
tar -xvjf freetype-2.4.12.tar.bz2
cd freetype-2.4.12
./configure && make && make install
```

### Building libfaac ###
[libfaac](http://www.audiocoding.com/faac.html) is optional unless you want support for [AAC](http://en.wikipedia.org/wiki/Advanced_Audio_Coding) audio codec or **.mp4** container for recording.
```
wget http://downloads.sourceforge.net/faac/faac-1.28.tar.bz2
tar -xvjf faac-1.28.tar.bz2
cd faac-1.28 && ./configure && make && make install
```
_Note: building the tests could fails but you can safely ignore it._

### Building FFmpeg ###
[FFmpeg](http://www.ffmpeg.org/) is **required** even if you don’t want support for video.
```
# [1] checkout source code
git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg
cd ffmpeg

# [2] grap a release branch
git checkout n1.2

# [3] configure source
./configure \
--extra-cflags="-fPIC" \
--extra-ldflags="-lpthread" \
\
--enable-pic --enable-memalign-hack --enable-pthreads \
--enable-shared --disable-static \
--disable-network --enable-pthreads \
--disable-ffmpeg --disable-ffplay --disable-ffserver --disable-ffprobe \
\
--enable-gpl \
\
--disable-debug \
\
--enable-libfreetype \
\
--enable-libfaac \
\
--enable-nonfree
# [4] build and install
make && make install
```
  * Remove _"--enable-nonfree"_ and _"--enable-libfaac"_ if you don’t want support for [AAC](http://en.wikipedia.org/wiki/Advanced_Audio_Coding) audio codec.
  * Add _--enable-libx264 --enable-encoder=libx264 --enable-decoder=h264_ to force building with support for H.264.

### Building `OpenH264` ###
**OpenH264** is optional. Adds support for **H.264** constrained baseline video codec.
```
git clone https://github.com/cisco/openh264.git
cd openh264
git checkout v1.1
make ENABLE64BIT=Yes # Use ENABLE64BIT=No for 32bit platforms
make install
```

### Building OpenAL Soft ###
[OpenAL Soft](http://kcat.strangesoft.net/openal.html) is optional. Adds support for Stereoscopic (spatial) 3D audio.
```
wget http://kcat.strangesoft.net/openal-releases/openal-soft-1.15.1.tar.bz2
tar -xvjf openal-soft-1.15.1.tar.bz2
cd openal-soft-1.15.1/build
cmake ..
make && make install
```

### Building openOffice ###

[OpenOffice](http://www.openoffice.org/) (or [LibreOffice](https://www.libreoffice.org/)) are optional and add support for presentation sharing. For information about this feature, check [the technical details](Technical_Presentation_sharing.md). **Version 4.0 or later is required**. Both the application and SDK are required.<br />
This section explain how to install (building would take hours) [OpenOffice](http://www.openoffice.org/). [LibreOffice](https://www.libreoffice.org/) could also be used but not recommended (not fully tested).<br />
<u>IMPORTANT:</u>**These instructions are for**Linux x86-64**and you must change the paths if you’re using a 32-bit system. Run _uname -m_ to get your CPU type. All rpms could be found at http://www.openoffice.org/download/other.html.
```
## Application (x64) ##
wget http://sourceforge.net/projects/openofficeorg.mirror/files/4.0.0/binaries/en-US/Apache_OpenOffice_4.0.0_Linux_x86-64_install-rpm_en-US.tar.gz
mkdir -p OpenOfficeApplication && tar -zxvf Apache_OpenOffice_4.0.0_Linux_x86-64_install-rpm_en-US.tar.gz -C OpenOfficeApplication
rpm -Uvih OpenOfficeApplication/en-US/RPMS/*rpm
```
```
## SDK (x64) ##
wget http://sourceforge.net/projects/openofficeorg.mirror/files/4.0.0/binaries/SDK/Apache_OpenOffice-SDK_4.0.0_Linux_x86-64_install-rpm_en-US.tar.gz
mkdir -p OpenOfficeSDK && tar -zxvf Apache_OpenOffice-SDK_4.0.0_Linux_x86-64_install-rpm_en-US.tar.gz -C OpenOfficeSDK
rpm -Uvih OpenOfficeSDK/en-US/RPMS/*rpm
```
Both OpenOffice application and SDK should be installed into**/opt/openoffice4**. If not, you’ll need to edit the script used to prepare the SDK headers.**<br />
Prepare the SDK headers:
```
LD_LIBRARY_PATH=/opt/openoffice4/program:/opt/openoffice4/sdk/lib /opt/openoffice4/sdk/bin/cppumaker -BUCR -O /opt/openoffice4/sdk/includecpp /opt/openoffice4/program/types.rdb
```
Please note that the destination folder must be named **includecpp**. <br />
Install java runtime (**required**):
```
yum install java-1.7.0-openjdk
```

#### Notes ####
Installing OpenOffice application will not add the binary (**soffice**) in your **$PATH** environment variable. The TelePresence system will try to start the program in the background using a relative path unless you have changed [presentation-sharing-app](Configuration_Presentation_sharing.md) configuration entry. You can change your **$PATH** environment variable to avoid editing [presentation-sharing-app](Configuration_Presentation_sharing.md) but this is not recommend if you’re testing different OpenOffice versions. It’s also highly recommended to append the folder containing the binary **AFTER $PATH**. Appending the folder before **$PATH** will force using shared libraries (e.g. libssl, libcurl…) installed with OpenOffice instead of yours.<br />
<u>Correct:</u> `export PATH=$PATH:/opt/openoffice4/program`<br />
<u><b>NOT</b> correct:</u> `export PATH=/opt/openoffice4/program:$PATH`

#### Known Issues ####
_"/usr/bin/ld: skipping incompatible /opt/openoffice4/sdk/lib/libuno\_sal.so when searching for -luno\_sal"_: CPU type mismatch (e.g. installed 64-bit libraries on 32-bit OS).

_"MSG: [TELEPRESENCE](TELEPRESENCE.md) [FFmpegOverlay](FFmpegOverlay.md) Not valid No such filter: 'drawtext'"_: [libfreetype](Support_BuildingSourceCode#Building_libfreetype.md) is missing. You'll have to rebuild FFmpeg with this library or disable overlays.

### Building Doubango ###
[Doubango VoIP framework 2.0](https://code.google.com/p/doubango/) **SVN [r989](https://code.google.com/p/telepresence/source/detail?r=989) or later is required**.
```
svn checkout http://doubango.googlecode.com/svn/branches/2.0/doubango doubango
cd doubango && ./autogen.sh && ./configure --with-speexdsp --with-ffmpeg
make && make install
```
Only few options are used to configure the source code and force enabling mandatory libraries. Any optional library is automatically detected. For example, use _"--with-opus"_ to force using Opus audio codec or _"--without-opus"_ to avoid automatic detection. You can also specify a path where to search for a library (e.g. _"--with-opus=/usr/local"_).
Use _"configure --help"_ for more information on supported options.

## Building the Telepresence system ##

```
svn checkout https://telepresence.googlecode.com/svn/trunk/ telepresence
cd telepresence
./autogen.sh && ./configure
make && make install
```

If no prefix is defined then, the binaries will be installed into **/usr/local/sbin**.


### Installing the configuration and fonts files ###
This is only required for first-time installations and **will override any existing configuration file**.
```
make samples
```

### Testing ###
We highly recommend using our [WebRTC SIP telepresence client](http://conf-call.org/) to test the system.

For more information on how to test the system: click [here](Support_Testing_the_system.md)