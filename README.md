S/PDIF tools
===========

These simple programs convert raw AC3 (Dolby Digital, A/52) and DTS streams
into WAV files containing the S/PDIF fake PCM encoding thereof. This means
that if such a .wav file is replayed correctly bit-by-bit via the S/PDIF
output of a device and supplied to a standards compliant decoder, it
will be recognized as a standards compliant AC3/DTS stream,
indistinguishable from a stream generated e.g. by a DVD player.

This is useful to e.g. author CDs containing compressed multichannel
data (such as DTS encoded CDs that commercially available).
Take care: Playing back the files produced via speakers at high volume
may damage these or your hearing.


A note about coding style
-------------------------

These sources are very old (from around 2006/2007) and show my coding
abilities back then when I started with C. I hope to clean this up
and modernize it at some point in the future.


ac32spdif
---------

This tool takes a raw .ac3 file and converts it into a .wav file of the
same sample rate containing one AC3 frame plus padding zeros for the
remaining time of that frame's time slot.


dts2spdif
---------

In a similar fashion, this tool translates a .dts file to a .wav file.
Currently it only supports 16 bit encapsulation, i.e. the data stream
is included in the .wav file 1:1.
The DTS specification also defines a 14 bit format for use with CDs
where there is a possibility that the data will be played back via
speakers in order to reduce the possibility of speaker/hearing damage
when the medium is played back without a DTS decoder. In this case,
only the lower 14 bits of a word would be used, with the high 2 bits
set to zero. This encoding may be added to dts2spdif later on.


License
-------

As usual, this code comes with no warranty whatsoever. I am especially
not responsible if you blow up your speakers or ears using these tools.

Do whatever you like with this code, but if you use it for your own
application I would appreciate an attribution note in the documentation.
Thanks!

 -- norly
