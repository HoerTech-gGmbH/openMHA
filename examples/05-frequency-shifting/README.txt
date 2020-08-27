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

This directory contains two example configurations and input sound
files which demonstrate how to use the fshift_hilbert and fshift plugins.

The main difference between fshift and fshift_hilbert is that fshift 
is faster but can only shift the frequency by whole fft bins.

For a general description of the plugins, please refer to the
corresponding section in the openMHA_plugins.pdf manual.

fbc.cfg

fbc.cfg demonstrates how to frequency shift a 440Hz sine tone to 520Hz. 


fbc_combination.cfg

fbc_combination show how to use the frequency shifter 
in the prerelease-combination generic hearing aid configuration.
In a real setup this configuration could be used to somewhat supress
feedback.


fshift_hilbert_live.cfg

fshift_hilbert_live.cfg demonstrates the usage of fshift_hilbert in
a live i/o setup

fshift.cfg

fshift demonstrates the usage of the fshift plugin. Note the coarser
frequency shift.




