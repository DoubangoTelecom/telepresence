Many SIP clients require to be registered (logged in) before being able to make calls. By default, any **REGISTER** request to the gateway will be rejected.

**accept-sip-reg** entry defines whether to accept incoming SIP REGISTER requests or not (acting as SIP registrar).
```
accept-sip-reg = yes # no to disable
```
_Configuration 5: Enabling/disabling SIP registration_

When the Telepresence system is behind a SIP registrar (e.g. **Asterisk**) then, this configuration entry is useless as the REGISTER requests will not be forwarded to the MCU.

See also:
  * [SA versus AS modes](Technical_SA_versus_AS_modes.md)