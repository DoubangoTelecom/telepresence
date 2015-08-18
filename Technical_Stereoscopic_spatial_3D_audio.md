Our 3D audio mixer is based on [OpenAL Soft](http://kcat.strangesoft.net/openal.html) and supports up to 256 sources. Your OpenAL version must be at least 1.15.1 and implements [ALC\_SOFT\_loopback](http://kcat.strangesoft.net/openal-extensions/SOFT_loopback.txt) extension.

To have a 3D audio, each SIP client should signal its position in the virtual room. The signaling is done using two SIP headers: TP-AudioPosition and TP-AudioVelocity. These two SIP headers must contain an array of three floating numbers.
```
TP-AudioPosition: [0.0f, 0.0f, 0.0f]
TP-AudioVelocity: [0.0f, 0.0f, 0.0f]
```
Using our [WebRTC TelePresence client](http://conf-call.org/contact.htm), the 3D settings could be defined at [http://conf-call.org/settings.htm](http://conf-call.org/settings.htm).

From OpenAL documentation, <i>"<u>AL_POSITION</u> specifies the current location of the object in the world coordinate system. Any 3-tuple of valid float values is allowed. Implementation behavior on encountering NaN and infinity is not defined. The object position is always defined in the world coordinate system."</i>

From OpenAL documentation, <i>“<u>AL_VELOCITY</u> specifies the current velocity (speed and direction) of the object, in the world coordinate system. Any 3-tuple of valid float/double values is allowed. The object AL_VELOCITY does not affect the source's position. OpenAL does not calculate the velocity from subsequent position updates, nor does it adjust the position over time based on the specified velocity. Any such calculation is left to the application. For the purposes of sound processing, position and velocity are independent parameters affecting different aspects of the sounds.”</i>