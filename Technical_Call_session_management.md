

A call session is managed using **SIP INFO** messages with **JSON** content. For now only muting/unmuting the session is supported. Next versions will add support for ejecting a participant, getting the list of participants, getting the call state (packet loss, RTT, audio/video quality, etc etc <br />
## Muting/unmuting ##
The MCU could detect that a session is muted based on the RTP packets but it’s highly recommended to also send a **SIP INFO** message for confirmation. For audio-only sessions, muting a session without sending a **SIP INFO** could be interpreted as a crash or network issue which automatically disconnects the call. <br />
When the _“hangout”_ video pattern is selected the MCU renders the speaker's video with the highest quality and size. Detecting a speaker could be problematic when the participants are in a noisy environment. Manually muting/unmuting your session is a way to avoid such issues. <br />
JSON content:
<table width='100%' border='1'>
<tr>
<td align='center'><b>Field name</b></td>
<td align='center'><b>Field value</b></td>
<td align='center'><b>Type</b></td>
<td align='center'><b>Availability</b></td>
</tr>
<tr>
<td align='center'>action</td>
<td align='center'>“req_call_mute”</td>
<td align='center'>String</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>enabled</td>
<td align='center'>< user defined ></td>
<td align='center'>Boolean</td>
<td align='center'>Mandatory</td>
</tr>
</table>

## Ejecting a participant ##
_--This section intentionally left blank--_

## Getting the list of participants ##
_--This section intentionally left blank--_

## Getting the call state ##
_--This section intentionally left blank--_