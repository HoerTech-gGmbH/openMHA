# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2021 HörTech gGmbH
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


# Import parent directory to be able to import the openMHA python package.
import sys
sys.path.append('../../mha/tools/python')
from openMHA import MHAConnection

# Make an MHAConnection instance to be able to communicate with the openMHA.
# Choose your individual host and port by calling MHAConnection(host,port).
# The default is host="localhost" and port=33337.
mha = MHAConnection()

# Send commands to the openMHA, read the values that were just written, and print them.
mha.set_val('fragsize', 128)
print('fragsize:', mha.get_val('fragsize'))
mha.set_val('srate', 44100)
print('srate:', mha.get_val('srate'))
mha.set_val('nchannels_in', 2)
print('nchannels_in:', mha.get_val('nchannels_in'))

# Read in a cfg file.
mha.read_cfg('../00-gain/gain_getting_started.cfg','')

# Read the loaded plugins in mhachain.
print('plugins in cfg:', mha.get_val_raw(b'mha.algos'))

# Change loaded plugins in mhachain to [gain identity].
# Identity has no influence on the signal.
mha.set_val('mha.algos', '[gain identity]')
print('plugins after changing:', mha.get_val_raw(b'mha.algos'))

# Check gain setting of the loaded configuration.
print('gains in cfg:', mha.get_val('mha.gain.gains'))

# Change gain setting of the loaded configuration.
mha.set_val('mha.gain.gains',[-5, 5])
print('gains after changing:', mha.get_val('mha.gain.gains'))


# Start the mha processing and stop it after finishing.
mha.set_val('cmd','start')
mha.set_val('cmd','quit')
