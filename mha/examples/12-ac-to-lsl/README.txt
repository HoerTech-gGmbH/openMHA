This is a minimal example for the ac2lsl plugin. It consists of two parts:

The configuration file ac2lsl.cfg instructs mha to process chirp.wav and
includes the rmslevel, save_wave and ac2lsl plugins in the processing chain.
rmslevel and save_wave add multiple ac variables to the ac space, which are
automatically served to the network as lsl stream outlet by the ac2lsl plugin.
The io lib is set up to process one buffer on every start command so that the
behaviour of the plugins is easier to observe.

The source file receive_lsl.cpp has to be compiled with the Makefile. The
resulting executable is a minimal example how to receive and unpack lsl
streams. It opens the lsl stream given as a command line argument or, if
no name is given, it lists the available streams and opens the first avail-
able stream.

The different parts of this example can, but do not need to be executed on
the same computer.

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
