# -*- coding: utf-8 -*-
# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 2019 HörTech gGmbH
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

from ast import literal_eval
from collections import Sequence, MutableSequence
from encodings.utf_8 import encode as encode_utf8
import sys
import telnetlib

if sys.version_info.major == 2:
    from string import maketrans
else:
    maketrans = str.maketrans

round_to_square_brackets = maketrans('()', '[]')


class MHAConnection(telnetlib.Telnet):
    """A class for connecting to a Master Hearing Aid (MHA) instance.

    An instance of this class represents a connection to an MHA process.  Its
    main methods are documented below.  In addition, this class implements the
    context manager protocol, so you can use it as an item in a with statement.
    """

    def send_command(self, buffer):
        """Send a command to an MHA instance.

        The argument is a buffer as expected by self.write(), and hence should
        be terminated by a newline character (b'\\n').
        """

        self.write(buffer)
        err_code, _match, resp = self.expect([b'\(MHA:success\)',
                                              b'\(MHA:failure\)'])
        if err_code == 0:
            return resp.rpartition(b'(MHA:success)')[0].strip()
        else:
            raise ValueError('Error sending message "{}" with error code {}:\nResponse: {}'
                             .format(buffer, err_code, resp))

    def get_val(self, path):
        """Get the value of the variable located at "path".
        """

        return self.send_command(path.strip() + b'?val\n')

    def get_type(self, path):
        """Get the type of the variable located at "path".
        """

        return self.send_command(path.strip() + b'?type\n')

    def is_writable(self, path):
        """Return True if the variable located at "path" is writeable.
        """

        ret = self.send_command(path.strip() + b'?perm\n')
        return ret == b'writable'

    def get_val_converted(self, path):
        """Get the converted value of the variable located at "path".

        This is the same as self.get_val(), except that the value is converted
        to an equivalent Python type.  For example, vector and matrix types
        are converted to Python lists.
        """

        data_type = self.get_type(path)
        data = self.get_val(path)

        if b'vector' in data_type or b'matrix' in data_type:
            data = data.replace(b' ', b', ').replace(b';', b',')

        # literal_eval() is a safe version of eval() that only accepts a small
        # list of literal structures
        return literal_eval(data.decode())

    def set_val(self, path, value):
        """Set the value of the variable located at "path" to "value".
        """

        cmd = path.strip() + b'=' + encode_utf8(value.strip())[0] + b'\n'
        return self.send_command(cmd)

    def set_val_converted(self, path, value):
        """Set the value of "path" to "value", after conversion to a string.

        This is the same as self.set_val(), except that the value is converted
        to a string as expected by MHA, e.g., Python sequence types are
        converted to MHA's vector or matrix types (and hence must have at most
        two dimensions!).

        Note: if you want to pass a NumPy array, use its tolist() method.
        """

        if isinstance(value, str):
            pass  # nothing to do
        elif isinstance(value, Sequence):
            if not isinstance(value, MutableSequence):
                value = str(value).translate(round_to_square_brackets)
            value = str(value).replace('],', '];').replace(',', '')
        else:
            value = str(value)  # hope for the best...

        return self.set_val(path, value)

    def list_ids(self):
        """Return a dictionary of plug-in paths and IDs.

        The dictionary references all plug-ins with an ID and consists of
        path/ID key-value-pairs.
        """

        ids = self.send_command(b'?listid\n').splitlines()
        return dict(id.split(b' = ') for id in ids)

    def find_id(self, plugin_id):
        """Return a tuple of all plug-in paths with the given ID.
        """

        ids = self.list_ids()
        return tuple(path for path, id in ids.items() if id == plugin_id)

    def __enter__(self):

        return self

    def __exit__(self, exc_type, exc_value, traceback):

        self.__del__()
