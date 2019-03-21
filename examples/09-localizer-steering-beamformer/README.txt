This directory contains an openMHA setup which estimates the direction of
arrival of sounds captured with 4 hearing aid microphones, and
which then controls a beamformer algorithm to steer to the most likely
direction of a localized (coherent) sound source.

In addition to the signal processing, a browser-based visualization
of the direction-of-arrival estimation can be started.

Please refer to file README_visualization.md to learn more about
the visualization procedure, and how to install the necessary
dependencies.

You can start the signal processing together with visualization by invoking
the shell skript

start_demo_live_binauralSteerBF.sh

Please also refer to the comments in this shell skript to learn how to prepare
the Jack server for this example.

# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 HörTech gGmbH
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
