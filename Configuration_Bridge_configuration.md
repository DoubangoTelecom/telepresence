In this beta version, global configuration entries are ignored when reused with a <b><code>[</code>bridge<code>]</code></b> section. For example, setting the audio sample rate at global scoop applies to all bridges but redefining it for a [bridge](bridge.md) section will be ignored. In the release version, it will be possible to override almost any configuration entry.
It’s not required to add a bridge in order to be able to make conference calls.
You can add as many <b><code>[</code>bridge<code>]</code></b> sections as you want.

Supported entries are: **id** and **pin-code**.
```
[bridge]
id=10060
pin-code=1234

[bridge]
id=10061
pin-code=0000
```
_Configuration 17: Bride settings_

  * **id** - defines the bridge identifier (a SIP client would call _"sip:<the id>@domain"_ to connect to this bridge).
  * **pin-code** – A 4-digit code used to protect the bridge.
For information on how a client authenticates to a bridge, check [here](Technical_Protecting_a_bridge_with_password.md).