This section explains how to use settings related to the audio.



# Pivot settings #
The notion of **pivot settings** is explained [here](Technical_Audio_mixer_design.md).
```
audio-channels = 1
audio-bits-per-sample = 16
audio-sample-rate = 8000
audio-ptime = 20
audio-volume = 1.0f
audio-dim = 2d
audio-max-latency = 200
```
_Configuration 11: Audio settings_

  * **audio-channels** - number of audio channels to use. Supported values are <u>1</u> and <u>2</u>.
  * **audio-bits-per-sample** - number of bits for each audio sample. Supported values are <u>8</u>, <u>16</u> and <u>32</u>.
  * **audio-sample-rate**: - audio sample rate. Almost any value is supported: http://en.wikipedia.org/wiki/Sampling_rate. Unit: Hz.
  * **audio-ptime** - number of milliseconds for each audio frame. The value should be multiple of 10. Supported values: <u><code>[</code>1 - 255<code>]</code></u>
  * **audio-volume** - attenuation (or gain) to apply to the mixed audio. Supported values: <u><code>[</code>0.0f - 1.0f<code>]</code></u>.
  * **audio-dim** - mixer dimensions. The value must be <u>2d</u> (Linear) or <u>3d</u> (Spatial). <u>3d</u> requires building the system with [OpenAL Soft](Support_BuildingSourceCode#Building_OpenAL_Soft.md).
  * **audio-max-latecncy** - maximum audio delay (because of clock drift) before resetting the jitter buffer. The value could be any positive value. Unit: milliseconds.