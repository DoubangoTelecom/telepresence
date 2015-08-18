

Presentation sharing allows **any SIP client** to share **PowerPoint** documents with any client connected to the same bridge. The only technical requirements are support for **HTTP(S)** (sharer only) and **SIP INFO**. <br />
This is a revolutionary way to share presentation as the documents are **“streamed”** in the video channel which means any client supporting video could see it. The slides are extracted from the document using [OpenOffice](Support_BuildingSourceCode#Building_openOffice.md) (or [LibreOffice](Support_BuildingSourceCode#Building_openOffice.md)) as JPEG pictures, re-encoded (H.264, VP8 or whatever) and mixed in the current video stream. Technically, OpenOffice is not integrated in the system but forked as new process and this is why both the SDK and application are required. The OpenOffice process will be started by the TelePresence system at boot time and added to the same job group as the current process to make sure the child will exit when the parent unexpectedly die. The default port used for the inter-process commutation is **2083** and could be changed using the configuration as explained [here](Configuration_Presentation_sharing.md).<br />
The TelePresence system supports [CORS](https://developer.mozilla.org/en-US/docs/HTTP/Access_control_CORS) which means the request could be sent from any domain. <br />
This feature could be tested using our online [WebRTC SIP client](http://conf-call.org/). <br />

Steps:
  1. Publish the PowerPoint document to the bridge using **HTTP(S) POST** requests.
  1. Receive feedbacks from the MCU (**SIP INFO** messages).
  1. Move from slide to slide using **SIP INFO** messages.
  1. Close the presentation session using **SIP INFO** message.

## Publishing the document ##
To start sharing a PowerPoint document you must have an active video session. You can only share **ONE** presentation at time. <br />
The document is sent to the MCU in **TWO** HTTP(S) requests using the same connection. The first request sends information about the document and the second the content. You must not mix the document information and content. <br />
The TelePresence system must be configured with an _http_ or _https_ transport (or both) as explained [here](Configuration_SIP_network_transports.md).
<br />
The first request structure:
<table width='100%' border='1'>
<tr>
<td align='center'><b>Element</b></td>
<td align='center'><b>Value</b></td>
<td align='center'><b>Availability</b></td>
</tr>
<tr>
<td align='center'>Request type</td>
<td align='center'>HTTP(S) POST</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>Request URL</td>
<td align='center'>/presentation</td>
<td align='center'>Optional</td>
</tr>
<tr>
<td align='center'>Content-Type</td>
<td align='center'>application/json</td>
<td align='center'>Mandatory</td>
</tr>
</table>

First request content (JSON):
<table width='100%' border='1'>
<tr>
<td align='center'><b>Field name</b></td>
<td align='center'><b>Field value</b></td>
<td align='center'><b>Type</b></td>
<td align='center'><b>Availability</b></td>
</tr>
<tr>
<td align='center'>action </td>
<td align='center'>“req_presentation_upload”</td>
<td align='center'>String</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>name</td>
<td align='center'><user defined></td>
<td align='center'>String</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>type</td>
<td align='center'><user defined></td>
<td align='center'>String</td>
<td align='center'>Optional</td>
</tr>
<tr>
<td align='center'>size</td>
<td align='center'><user defined></td>
<td align='center'>Integer</td>
<td align='center'>Optional</td>
</tr>
<tr>
<td align='center'>bridge_id</td>
<td align='center'><user defined></td>
<td align='center'>String</td>
<td align='center'>Optional</td>
</tr>
<tr>
<td align='center'>bridge_pin</td>
<td align='center'><user defined></td>
<td align='center'>String</td>
<td align='center'>Optional</td>
</tr>
<tr>
<td align='center'>user_id</td>
<td align='center'><user defined></td>
<td align='center'>String</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td>
<pre><code>POST /presentation HTTP/1.1<br>
Host: 192.168.0.37:20065<br>
Connection: keep-alive<br>
Content-Length: 174<br>
Origin: http://conf-call.org<br>
User-Agent: Mozilla/5.0 (Windows NT 6.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/28.0.1500.95 Safari/537.36<br>
Content-type: application/json<br>
Accept: */*<br>
Referer: http://conf-call.org<br>
Accept-Encoding: gzip,deflate,sdch<br>
Accept-Language: en-US,en;q=0.8<br>
<br>
{"action":"req_presentation_upload","name":"db_pres_01.ppt","type":"application/vnd.ms-powerpoint","size":752128,"bridge_id":"100600","bridge_pin":"1234","user_id":"johndoe"}<br>
</code></pre>
</td>
</tr>
</table>

The second request structure:
<table width='100%' border='1'>
<tr>
<td align='center'><b>Element</b></td>
<td align='center'><b>Value</b></td>
<td align='center'><b>Availability</b></td>
</tr>
<tr>
<td align='center'>Request type</td>
<td align='center'>HTTP(S) POST</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>Request URL</td>
<td align='center'>/presentation</td>
<td align='center'>Optional</td>
</tr>
<tr>
<td align='center'>Content</td>
<td align='center'>< Binary ></td>
<td align='center'>Mandatory</td>
</tr>
</table>

## Receiving feedbacks ##
Once the presentation is published the TelePresence system will send SIP INFO messages to give feedbacks about the session state. The **SIP INFO** messages always contain JSON content. <br />
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
<td align='center'>“res_presentation_state”</td>
<td align='center'>String</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>name</td>
<td align='center'>< server defined ></td>
<td align='center'>String</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>state</td>
<td align='center'>“opened” <br />“exported” <br />“closed”<br />“error”</td>
<td align='center'>String</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>id</td>
<td align='center'>< server defined ></td>
<td align='center'>Integer</td>
<td align='center'>Optional</td>
</tr>
<tr>
<td align='center'>page_index</td>
<td align='center'>< server defined ></td>
<td align='center'>Integer</td>
<td align='center'>Optional (0 if missing)</td>
</tr>
</table>

## Fetching the presentation ##
Once the presentation is opened (see previous section), you can navigate through it using **SIP INFO** messages. For security reasons the presentation is tied to your SIP connection to be sure no one else could control it. <br />
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
<td align='center'>“req_presentation_goto”</td>
<td align='center'>String</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>page_index</td>
<td align='center'>< user defined ></td>
<td align='center'>Integer</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>id</td>
<td align='center'>< user defined ></td>
<td align='center'>Integer</td>
<td align='center'>Optional</td>
</tr>
</table>

## Closing the presentation ##
At any time you can end the presentation session using **SIP INFO** request. <br />
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
<td align='center'>“req_presentation_close”</td>
<td align='center'>String</td>
<td align='center'>Mandatory</td>
</tr>
<tr>
<td align='center'>id</td>
<td align='center'>< user defined ></td>
<td align='center'>Integer</td>
<td align='center'>Optional</td>
</tr>
</table>