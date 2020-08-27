#!/usr/bin/env bash
# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 2019 2020 HörTech gGmbH
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

# This script starts a live demo of a 4-channel binaural beamformer that is
# steered by a probabilistic sound source localization algorithm.
# Note that in this example configuration file a localization model and
# beamformer filter coefficients are given a according to a specific hearing aid
# setup.

# Start JACK server
# Make sure that JACK has been set up according to your hardware interface and
# the parameters 'srate', 'nchannels_in', 'fragsize' given in the configuration
# file loaded by openMHA below.
# Also have a look at the io.con_in and io.con_out settings
# The example file assumes a binaural hearing aid setup with two microphones on
# each side:
# io.con_in = [<front-left> <front-right> <rear-left> <rear-right>]
# io.con_out = [<left> <right>]
qjackctl -s &
jack_wait -w  # wait for JACK to be available

# Start openMHA in xterm
# Read configuration file containing the whole setup
(
    xterm -e 'mha ?read:Jack_live_resampling_doasvm_4Chan_16K_pool_acSteerMVDR_binaural.cfg;sleep 20' &
)
