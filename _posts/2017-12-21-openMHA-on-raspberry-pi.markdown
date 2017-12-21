---
layout: post
title:  "openMHA on Raspberry Pi"
date:   2017-12-21 08:00:00
category: userproject
tags: []
---
![Hardware Setup](/images/RPi_hardware.png "Hardware Setup")
##### Hardware Setup*


A community project called __"A mobile hearing aid prototype based on openMHA"__ was recently introduced on Github.
The project aims at lowering the entry barrier for hearing aid algorithm development and evaluation. This is achieved by combining the openMHA software with a portable single-board computer, the Raspberry Pi. This way interested people can actively contribute to testing and improving hearing devices.
An important aspect beneath mobility was availability of suitable hardware at low cost.
To this end hardware components were selected that are available as affordable consumer hardware devices for about 250&euro; in total.
All software required to run the system is available open-source.
Its intended area of application is in teaching in the context of applied hearing aid research, but is not limited to that as it can serve as a platform for projects of the real-time audio signal processing enthusiastic community. 

Ten systems have already been set up at the Carl-von-Ossietzky Universität Oldenburg, Germany to be used in student projects and lab courses.


Documentation and software is available via Github:

[https://github.com/m-r-s/hearingaid-prototype](https://github.com/m-r-s/hearingaid-prototype)

The repository features a [part list](https://github.com/m-r-s/hearingaid-prototype/wiki#hardware-list), links to the [required software](https://github.com/m-r-s/hearingaid-prototype#main-ingredients) and documentation on [wiki pages](https://github.com/m-r-s/hearingaid-prototype/wiki).
An [SD card image](https://github.com/m-r-s/hearingaid-prototype/tree/master/signatures) and a [quick start guide](https://github.com/m-r-s/hearingaid-prototype/wiki#quick-start) are provided as well.  

This system implements a basic binaural hearing aid processing chain build from a set of openMHA plugins featuring
- calibration
- binaural coherence filter for noise suppression and feedback reduction
- and a multi-channel dynamic range compressor for hearing loss and recruitment compensation. 





###### *Image source: https://github.com/m-r-s/hearingaid-prototype/blob/master/images/hardware.svg