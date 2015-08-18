**avpf-tail-length** configuration entry defines the maximum and minimum queue length used to store the outgoing RTP packets. The stored packets are used to honor incoming **RTCP-NACK** requests. See [here](Technical_Video_quality#AVPF_tail_length.md) for more technical information.
```
avpf-tail-length = 200;500 # min;max
```
_Configuration 8: Setting AVPF tail length_