This directory contains a toy example of a graphical control interface that can be used
to modify parameters of an openMHA configuration while processing the live audio signal
and that can continuously read monitor variables from the running openMHA.

Requirements:
The GUI is based on the flow-based programming tool Node-RED, which needs to be
installed on your system to run the example. More information can be found under
https://nodered.org/
The Node-RED dashboard nodes (https://flows.nodered.org/node/node-red-dashboard)
are used in this example and must be installed on your system.

Please note that this example has been developed and tested under a Linux operating
system. Hence, the following instructions assume that a Linux system is used to run
the example. In general there is no obstacle to run the example under other operating
system as openMHA and Node-RED are available for several platforms. 

Run the example:
This example comes with an openMHA configuration file which performs live processing
of microphone input signals. The resulting signal is provided via the sound device
output channels.
A JACK server needs to be started with parameters according to the openMHA
configuration, here fs=44100 and 64 frames/period.

The processing is started by executing

mha ?read:node-red-gui-demo.cfg cmd=start

in this example directory. The configuration basically contains a dynamic range
compression plugin, a noise generator and a sine generator. These plugins can be
controlled using the GUI via a web browser.

The GUI can be started by running

node-red GUI_demo.flow

in the same example directory. A web server will be started that provides the Node-RED
configuration interface.
It can be reached through a web browser under:
http://127.0.0.1:1880/.

The dashboard, i.e., openMHA control interface defined in the Node-RED configuration
is accessible under:
http://127.0.0.1:1880/ui/#!/0

GUI elements
============
Left panel:
- Move the green dot in the "Compressor settings" panel to modify the sound
characteristics of the output signal. A set of manually generated  gaintables
is used here as an  example.
An overview of the characteristics at the center, edges and corner position of
the 2d-panel is provided in gaintables.pdf.
- Noise and sine generator can be switched on and off, levels controlled and
the frequency of the sine tone

Right panel:
- Broadband input levels of left and right microphone channels is measured.
Note that you are working with an non-calibrated setup such that the displayed
levels do not accord to the physical sound level reaching the microphones.
- Compressor input level displays the levels in the different frequency bands
of the dynamic range compressor for left and right input channel. Here, the
contributions of the signal generators are observable.
- Overall level displays the broadband level averaged over left and right input
channel.



# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2019 HörTech gGmbH
#
# openMHA is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# openMHA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License, version 3 for more details.
#
# You should have received a copy of the GNU Affero General Public License, 
# version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.
