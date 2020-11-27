from ast import literal_eval
from collections.abc import Sequence, MutableSequence
from encodings.utf_8 import encode as encode_utf8
from functools import update_wrapper
import re
import telnetlib

_round_to_square_brackets = str.maketrans('()', '[]')
# This matches either a) digits *not* preceded by an opening parenthesis and
# followed by a space, or b) closing parentheses.  This prevents the
# real-valued part of a complex number from suddenly being separated by a
# comma.
_vcomplex_add_comma = re.compile(br'((?<!\()\d\s|\))')
# This matches complex numbers without a real part, which are not surrounded by
# parentheses.  The purpose is to work around an MHA parser quirk where the
# real part has to be explicitly represented as "0" in the string, e.g.,
# "(0+1.2i)" vs. "1.2i".
_complex_prepend_0 = re.compile(r'([\de\+\.]*\dj(?!\)))')


class _stringify:

    def __init__(self, inputs=True, outputs=True):
        """A decorator that "stringifies" a function.

        This decorator wraps a function "func" that expects bytes arguments and
        returns bytes.  The wrapping function has an identical signature except
        that it automatically encodes str arguments as UTF-8 and also
        automatically decodes the bytes return value.
        """

        self.inputs = inputs
        self.outputs = outputs

    def __call__(self, func):

        def new_func(*args, **kwargs):

            # automatically handle string arguments
            if self.inputs:
                new_args = (
                    (encode_utf8(arg)[0] if isinstance(arg, str) else arg)
                    for arg in args
                )
                new_kwargs = {
                    k: (encode_utf8(v)[0] if isinstance(v, str) else v)
                    for k, v in kwargs.items()
                }
            else:
                new_args = args
                new_kwargs = kwargs

            # optionally return MHA's response as a string
            ret = func(*new_args, **new_kwargs)
            if self.outputs:
                return ret.decode()
            return ret

        # update new_func() to look like func()
        update_wrapper(new_func, func)

        return new_func


class MHAConnection:
    """A class for communicating with a Master Hearing Aid (MHA) instance.

    An instance of this class represents a connection to an MHA process and
    provides a thin abstraction over its network protocoll.

    See the documentation of `telnetlib.Telnet` for any additional constructor
    arguments.
    """

    def __init__(self, host="localhost", port=33337, *args, **kwargs):

        self._args = (host, port, *args)
        self._kwargs = kwargs
        self._tn_con = telnetlib.Telnet(host, port, *args, **kwargs)

        # convenience aliases; defined like this so that they retain the
        # original doc-string and in order to avoid an additional function call
        self.get = self.get_val
        self.set = self.set_val

    def _reopen(self):
        """Close the connection and open it again.
        """

        self._tn_con.close()
        self._tn_con.open(*self._args, **self._kwargs)

    def _send_command(self, buffer, /):
        """Send a command to an MHA instance.

        The argument is a buffer as expected by self.write(), and hence should
        be terminated by a newline character (b'\\n').
        """

        self._tn_con.write(buffer)
        err_code, _match, resp = self._tn_con.expect(
            [br'\(MHA:success\)', br'\(MHA:failure\)']
        )
        if err_code == 0:
            return resp.rpartition(b'(MHA:success)')[0].strip()
        else:
            raise ValueError(
                'Error sending message {} with error code {}:\nResponse: {}'
                .format(buffer, err_code, resp)
            )

    @_stringify()
    def get_contents(self, path=b'', /):
        """Return the contents of the element at "path".
        """

        return self._send_command(path.strip() + b'?\n')

    @_stringify()
    def get_help(self, path, /):
        """Return the documentation of the element at "path".
        """

        return self._send_command(path.strip() + b'?help\n')

    @_stringify()
    def get_type(self, path, /):
        """Return the type of the variable located at "path".
        """

        return self._send_command(path.strip() + b'?type\n')

    @_stringify(outputs=False)
    def is_writable(self, path, /):
        """Return True if the variable located at "path" is writeable.
        """

        ret = self._send_command(path.strip() + b'?perm\n')
        return ret == b'writable'

    def get_val_raw(self, path, /):
        """Return the value of the variable located at "path".
        """

        return self._send_command(path.strip() + b'?val\n')

    @_stringify(outputs=False)
    def get_val(self, path, /):
        """Return the converted value of the variable located at "path".

        This is the same as self.get_val_raw(), except that the value is
        converted to an equivalent Python type.  For example, vector and matrix
        types are converted to Python lists.
        """

        data_type = self.get_type(path)
        data = self.get_val_raw(path)

        # Return plain strings immediately since they contain no quotes, which
        # would cause the below literal_eval() to fail.
        if data_type == 'string':
            return data.decode()

        # Types returned by OpenMHA contain additional type information (e.g.,
        # matrix<float>), thus we cannot simply check for equality.
        if 'vector' in data_type or 'matrix' in data_type:
            data = data.replace(b' ', b', ')
        elif 'vcomplex' in data_type or 'mcomplex' in data_type:
            data = _vcomplex_add_comma.sub(br'\1,', data) \

        if 'complex' in data_type:
            data = data.replace(b'i', b'j')

        if 'matrix' in data_type or 'mcomplex' in data_type:
            data = data.replace(b';', b',')

        # literal_eval() is a safe version of eval() that only accepts a small
        # list of literal structures
        return literal_eval(data.decode())

    def set_val_raw(self, path, value, /):
        """Set the value of the variable located at "path" to "value".
        """

        cmd = path.strip() + b'=' + value.strip() + b'\n'
        return self._send_command(cmd)

    def set_val(self, path, value, /):
        """Set the value of "path" to "value", after conversion to a string.

        This is the same as self.set_val_raw(), except that the value is
        converted to a string as expected by MHA, e.g., Python sequence types
        are converted to MHA's vector or matrix types (and hence must have at
        most two dimensions!).

        Note: if you want to pass a NumPy array, use its tolist() method.
        """

        if isinstance(path, str):
            path = encode_utf8(path)[0]
        data_type = self.get_type(path)

        if isinstance(value, (str, bytes)):
            pass  # nothing to do
        elif isinstance(value, Sequence):
            if not isinstance(value, MutableSequence):
                value = str(value).translate(_round_to_square_brackets)
            if 'complex' in data_type:
                value = _complex_prepend_0.sub(r'(0+\1)', str(value)) \
                        .replace('j', 'i')
            value = str(value).replace('],', '];').replace(',', ' ')
        elif 'complex' in data_type:
            value = _complex_prepend_0.sub(r'(0+\1)', str(value))
            value = value.replace('j', 'i')
        else:
            value = str(value)

        if isinstance(value, str):
            value = encode_utf8(value)[0]

        return self.set_val_raw(path, value).decode()

    @_stringify()
    def get_range(self, path, /):
        """Return the supported range of values of the variable at "path".
        """

        return self._send_command(path.strip() + b'?range\n')

    @_stringify()
    def get_substitutions(self, path, /):
        """Return the variable substitutions applied to the node at "path".
        """

        return self._send_command(path.strip() + b'?subst\n')

    @_stringify(outputs=False)
    def get_entries(self, path, /):
        """Return the list of nodes under the node at "path".
        """

        resp = self._send_command(path.strip() + b'?entries\n').decode()
        return tuple(resp.strip('[]').split(' '))

    def list_ids(self):
        """Return a dictionary mapping plug-in paths to IDs.
        """

        ids = self._send_command(b'?listid\n').splitlines()
        return dict(id.decode().split(' = ') for id in ids)

    def find_id(self, plugin_id, /):
        """Return a tuple of all plug-in paths with the given ID.
        """

        if isinstance(plugin_id, bytes):
            plugin_id = plugin_id.decode()

        ids = self.list_ids()
        return tuple(path for path, id in ids.items() if id == plugin_id)

    @_stringify()
    def save_node(self, file_name, path=b'', /, *, with_comments=True):
        """Save the contents of the node at "path" into a file.

        The contents are saved with comments if "with_comments" is equal to
        True (the default).
        """

        save_cmd = (b'?save:' if with_comments else b'?saveshort:')

        return self._send_command(path.strip() + save_cmd + file_name + b'\n')

    @_stringify()
    def save_monitor_vars(self, file_name, path=b'', /):
        """Save the contents of all monitor variables under the node at "path".
        """

        return self._send_command(path.strip() + b'?savemons:' + file_name +
                                  b'\n')

    @_stringify()
    def read_cfg(self, file_name, path=b'', /):
        """Read the contents of "file_name" into the parser node at "path".
        """

        return self._send_command(path.strip() + b'?read:' + file_name + b'\n')

    def __enter__(self):
        """The enter method of the context manager protocol.
        """

        return self

    def __exit__(self, exc_type, exc_value, traceback):
        """The exit method of the context manager protocol.
        """

        self._tn_con.close()
        # do *not* ignore exceptions raised in the with-statement context
        return False
