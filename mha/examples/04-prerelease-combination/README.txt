The configuration file in this directory, prerelease_combination.cfg,
combines plugins for calibration, adaptive differential microphones,
STFT processing, coherence filtering, a filterbank, and dynamic compression.

The configuration file contains extensive documentation. Please refer
to the comments in the configuration file to learn more about this
combination of algorithms.

The configuration performs audio file processing: It processes the sound
file 1speaker_diffNoise_4ch.wav in this directory and produces the output
sound file 1speaker_diffNoise_4ch_OUT.wav.

The processing is started like this:
mha ?read:prerelease_combination.cfg cmd=start cmd=quit

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
