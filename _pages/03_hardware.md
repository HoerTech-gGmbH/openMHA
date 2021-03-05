---
layout: page
title: Hardware
permalink: /hardware/
menu: main
---

![Portable Hearing Laboratory](/images/phl.png "Portable Hearing Laboratory"){: align="right" width="400px"}

#### Portable Hearing Laboratory
The Portable Hearing Laboratory (PHL) combines portable hardware and the openMHA software to an integrated hearing aid research platform. The PHL consist of a [Beaglebone Black wireless](https://beagleboard.org/black-wireless) single-board computer extended with a multi-channel audio board based on the "Cape4all" audio interface (see below). The system includes a battery and a set of binaural behind-the-ear (BTE) hearing aids that comprise two microphones on each side and receivers in the canal (RIC).  The BTE- RIC devices are connected with the processing box via flexible cables. The optimized Linux operating system [Mahalia](http://mahalia.openmha.org/) runs on the hardware. The whole system can be worn by a user with a neckband and allows hearing aid research in the field based on openMHA. More information can be found at the according [BatAndCat webpage](https://batandcat.com/portable-hearing-laboratory-phl.html). 

![Cape4all on BeagleBone Black wireless](/images/BBBw_Cape4all.jpg "Cape4all on BeagleBone Black wireless. Photo: Hendrik Kayser"){: align="right" width="240px"}

#### Cape4all on BeagleBone Black  

This setup is based on the [Beaglebone Black](https://beagleboard.org/black) or [Beaglebone Black wireless](https://beagleboard.org/black-wireless) single-board computer.
It is extended with a multi-channel audio interface board in the form of a ["cape"](https://elinux.org/Beagleboard:BeagleBone_Capes) for the BeagleBone: the [Cape4all](https://github.com/HoerTech-gGmbH/Cape4all).
Its hardware design is available under open-source license on Github and was developed by the Leibniz University Hannover in the Cluster of Excellence ["Hearing4all"](http://hearing4all.eu/) with a focus on low-latency multi-channel audio capabilities.
In collaboration with Daniel James and Chris Obbard from [64Studio](https://64studio.com/) an ALSA sound driver was developed and a Linux system was optimized for low-latency, real-time audio processing with openMHA: [Mahalia](https://github.com/64studio/mahalia-utils).     
More information and source files are availble under the links given above.
ISO images of the Mahalia distribution can be downloaded from [mahalia.openmha.org](http://mahalia.openmha.org/).



### Publications / Conference Contributions
[Linux Audio Conference 2018](/docs/LAC2018cape4all.pdf) (paper)

[Embedded Linux Conference 2018](https://osseu18.sched.com/event/FwHJ/preemptrt-isnt-just-for-lasers-the-perfect-match-for-hearing-aid-research-christopher-obbard-daniel-james-64-studio-ltd)

### News articles

[opensource.com](https://opensource.com/article/18/7/open-hearing-aid-platform)

[Heise](https://www.heise.de/make/meldung/Cape4all-Hoergeraete-sollen-von-Open-Hardware-profitieren-4108799.html) (in German)


#### Raspberry Pi

A user project running openMHA on a Raspberry Pi setup with (almost) off-the-shelf audio hardware was introduced at the end of 2017. Further details and pointers are found in a corresponding [news entry](/userproject/2017/12/21/openMHA-on-raspberry-pi.html).