Many hearing aid algorithms work in the spectral domain and require a
short time fourier transform (STFT) signal to perform their signal
processing.

MHA provides dedicated plugins to transform time-domain signal to STFT
signal and back.  Most commonly, the overlapadd plugin is used for
this purpose.  Many examples in our examples directories use this
plugin.  Please refer to their configuration files for real-world
examples how the overlapadd plugin can be used.  Please refer to the
plugin documentation PDF for a description of the overlapadd procedure
with the overlapadd plugin.

The overlapadd plugin is recommended for most use cases where STFT
signal is required. It combines the transformation from time-domain
signal to STFT signal, lets another plugin perform the signal
processing in the STFT domain, and transforms the signal back to the
time domain.

For some use cases, users might want to have separate plugins performing the
forward and backward transformations. This is possible for example with the
wave2spec and spec2wave plugins. This directory contains a simple
configuration file that demonstrates these plugins in use.

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
