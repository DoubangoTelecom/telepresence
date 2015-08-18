A presentation is any **PowerPoint** document and it could be shared from any SIP client running on any device. The presentation is uploaded to the TelePresence system using **HTTP(S) POST** request which means a http (or https) transport must be configured as explained  [here](Configuration_SIP_network_transports.md). More technical details could be found [here](Technical_Presentation_sharing.md).
```
presentation-sharing-enabled = yes
presentation-sharing-process-local-port = 2083
presentation-sharing-base-folder = ./presentations
presentation-sharing-app = soffice
```
_Configuration 11: Presentation sharing_

  * **presentation-sharing-enabled** - whether to enable presentation sharing. Default is **yes**. The application must be built with [OpenOffice](Support_BuildingSourceCode#Building_openOffice.md) (recommended) or [LibreOffice](Support_BuildingSourceCode#Building_openOffice.md) SDK to support this feature. This feature will be silently disabled if both SDKs are missing.
  * **presentation-sharing-process-local-port** - some implementations requires a third-party application (e.g. OpenOffice or LibreOffice) to export the presentation. The third-party application will be forked to run in the background and the local port ([1024-65535]) is used to communicate with the TelePresence system. For example, if the third-party application is OpenOffice and the local port is equal to **2083** then:
    * command string would be: `soffice -norestore –headless -nofirststartwizard -invisible "-accept=socket,host=localhost,port=2083;urp;StarOffice.ServiceManager"`
    * and the connection string would be: `uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager`. **soffice** is the OpenOffice application binary. Your **$PATH** environment variable must reference the folder containing the binary or **presentation-sharing-app** must contain a full path (e.g. _/opt/openoffice4/program/soffice_).
  * **presentation-sharing-base-folder** - base folder where to store uploaded presentations and temporary exported jpeg images. For example, a document named _mypres.ppt_ uploaded by _bob_ who is connected to a bridge with number equal to _100600_ would have a path equal to `<the base folder>/100600/bob/mypres.ppt`.
  * **presentation-sharing-app** - third-party application name. Could be full (e.g. _"/opt/openoffice4/program/soffice"_) or relative (e.g. _"soffice"_) path. Relative path requires having the folder containing the application in your **$PATH** environment variable.