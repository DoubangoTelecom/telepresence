**codecs** configuration entry defines the list of all supported codecs. Only **G.711** and **G.722** are natively supported and all other codecs have to be enabled when building the Doubango VoIP framework source code.
Each codec priority is equal to its position in the list. First codecs have highest priority.
Supported values are:
  * opus
  * pcma
  * pcmu
  * amr-nb-be
  * amr-nb-oa
  * speex-nb
  * speex-wb
  * speex-uwb
  * g729
  * gsm
  * g722
  * ilbc
  * h264-bp
  * h264-mp
  * vp8
  * h263
  * h263+
  * theora
  * mp4v-es.

```
codecs = pcma;pcmu;vp8;h264-bp;h264-mp
```
_Configuration 9: Setting audio/video codecs_