The audio mixer is part of the [MCU](http://en.wikipedia.org/wiki/Multipoint_control_unit) engine.

The audio mixer supports mixing several streams with different settings (rate, channels, bits per sample or ptime). For example, a bridge can host a conference with two endpoints, one using g711 (8khz, mono, 20ms) and the other using opus (48khz, stereo, 30ms). As you may expect, it’s not technically possible to mix two streams with different settings without resampling.

In the audio mixer there is a notion of <u><i>"pivot settings"</i></u>. <u><i>"pivot settings"</i></u> is the audio parameters to which any stream is resampled to, before mixing. The pivot settings are defined using the configuration file as explained [here](Configuration_Audio#Pivot_settings.md).

The Doubango framework use [libspeexdsp](https://code.google.com/p/telepresence/wiki/Support_BuildingSourceCode#Building_libspeex_and_libspeexdsp) for the resampling while the [MCU](http://en.wikipedia.org/wiki/Multipoint_control_unit) uses libswresample (from FFmpeg). Both libraries are required.
It’s very important to understand the notion of <u><i>"pivot settings"</i></u> because using wrong values could lead to poor audio quality and high CPU usage.


![https://telepresence.googlecode.com/svn/trunk/images/audio_mixer_resampler.png](https://telepresence.googlecode.com/svn/trunk/images/audio_mixer_resampler.png)<br />
<u>Figure 1: Audio resampling</u>

From the above figure, you can easily see that the incoming audio samples from an endpoint to the MCU could be resampled up to two times if your pivot and negotiated codec settings mismatch. To minimize the number of audio resampling processes your codec settings have to be as close as possible to those used as pivot. If the settings (pivot, codecs) match, then no resampling will be done.

In this beta version, we support 2d and 3d mixing types. The type of mixing is defined using the configuration file as explained [here](Configuration_Audio#Pivot_settings.md).

The 2d mixing is linear (monophonic or stereophonic) and very basic. No additional thirdparties library is required for this.

The 3d mixing is stereoscopic (spatial) and requires [OpenAL Soft](https://code.google.com/p/telepresence/wiki/Support_BuildingSourceCode#Building_OpenAL_Soft).