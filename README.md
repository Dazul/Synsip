# Synsip
Basic call machine using SIP and TextToSpeach softwares.

## /!\ This is a prototype
For now, Synsip is a prototype. A lot of configuration is hardcoded.

## Dependencies

Synsip use the PJSIP library in order to make the SIP calls.
The TextToSpeach is called through a Bash script. The script given with Synsip work with MBROLA.
You can change it.

Synsip use Syslog as loging platforme.

## Installation

Once PJSIP is installed on your computer, run this commandes on the Synsip foldes
```
./autogen.sh
./configure
make
make install
```

## Plateform

Synsip was tested only on GNU/Linux on amd64 computer and on the Raspberry Pi.
