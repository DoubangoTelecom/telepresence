This is a short list with all **known issues** (to be fixed before the end of the beta phase).
  1. The audio quality on the recorded files is not as good as we expect. This looks like an issue on the PTS and DTS.
  1. Sometimes, the recorded file cannot be played. **Fixed in [r118](https://code.google.com/p/telepresence/source/detail?r=118)**.
  1. Mixed video looks stretched when the SIP clients is a mobile in portrait (e.g. iDoubs and IMSDroid). No issue when device is in landscape or using our WebRTC demo client. **Fixed in [r118](https://code.google.com/p/telepresence/source/detail?r=118)**.
  1. The algorithm to detect the current speaker and listeners is buggy.
  1. 3D (spatial) mixed audio quality is not as good as we expect.
  1. Mixer fails to negotiate H.263. **Fixed in [r118](https://code.google.com/p/telepresence/source/detail?r=118)**.
  1. "error while loading shared libraries: libtinySAK.so.0: cannot open shared object file: No such file or directory". You can find a temporary workaround at https://groups.google.com/forum/#!topic/opentelepresence/qR7OdJGAcAE. **Fixed in [r118](https://code.google.com/p/telepresence/source/detail?r=118)**.