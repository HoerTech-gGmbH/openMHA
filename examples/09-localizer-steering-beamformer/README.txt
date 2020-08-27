This directory contains an openMHA setup which estimates the direction of
arrival of sounds captured with 4 hearing aid microphones, and which then
controls a beamformer algorithm to steer to the most likely direction of a
localized (coherent) sound source.  It utilizes an implementation of a
localization algorithm based on GCC-PHAT (see [0] and [1] and the
configuration files for details).

You can start the signal processing by invoking the shell script

    start_demo_live_binauralSteerBF.sh

Please also refer to the comments in this shell script to learn how to prepare
the Jack server for this example.

If you wish to visualize the algorithm output, a separate tool is available at
[2]. (*Note*: this was part of openMHA from versions 4.9.0 to 4.11.0, in the
visualisation_web/ subdirectory.)

[0] C. Knapp and G. C. Carter, “The generalized correlation method for
estimation of time delay,” IEEE Transactions on Acoustics, Speech and Signal
Processing, vol. 24, no. 4, pp. 320–327, Aug. 1976.

[1] H. Kayser and J. Anemüller, “A discriminative learning approach to
probabilistic acoustic source localization,” In: International Workshop on
Acoustic Echo and Noise Control (IWAENC 2014), pp. 100 -- 104, Antibes,
France, 2014

[2] https://github.com/HoerTech-gGmbH/doasvm-visualizer

# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 2020 HörTech gGmbH
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
