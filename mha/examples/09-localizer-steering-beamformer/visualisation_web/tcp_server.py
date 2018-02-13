from __future__ import division

from ast import literal_eval
import json
import re
from warnings import warn

from tornado import tcpserver, ioloop

import server_common

_p = [None]
MHA_MESSAGES = frozenset((
    'new_pooling_wndlen',
    'new_pooling_alpha',
    'new_pooling_type',
))


class LoopingWebSocket(server_common.MyWebSocketHandler):

    def _send_data(self):

        if _p[0]:
            self.write_message(json.dumps({'data': _p[0]}))

    def on_message(self, message):

        message = json.loads(message)

        if 'command' in message:
            if message['command'] == 'send_data':
                self._send_data()
            else:
                print('Unknown command "{}"'.format(message['command']))
        elif len(message) == 1 and list(message.keys())[0] in MHA_MESSAGES:
            # ignore MHA plug-in commands
            pass
        else:
            print('Unknown message "{}"'.format(message))


class TCPListener(tcpserver.TCPServer):

    def __init__(self, model_length, *args, **kwargs):

        self._model_length = model_length

        super(TCPListener, self).__init__(*args, **kwargs)

    def _read_line(self):

        self._stream.read_until(b'\n', self._handle_read)

    def _handle_read(self, data):

        # When sending data via java_tcp(), Java stores the chars in *two*
        # bytes.  One of them is usually a NULL byte, so filter those out, too.
        data = re.sub('\s+', ', ', data.strip().decode().replace('\0', ''))
        data = literal_eval('[' + data + ']')
        if len(data) == self._model_length:
            _p[0] = data
        else:
            warn('Ignored data with invalid length {} (expected {}).'
                 .format(len(data), self._model_length))

        self._read_line()

    def handle_stream(self, stream, address):

        self._stream = stream
        self._read_line()

if __name__ == '__main__':

    import argparse

    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    server_common.add_common_args(parser)
    parser.add_argument(
        '--tcp-port',
        default=9990,
        type=int,
        help='The port the TCP server should listen on.',
    )
    parser.add_argument(
        'min_angle',
        type=int,
    )
    parser.add_argument(
        'max_angle',
        type=int,
    )
    parser.add_argument(
        'model_length',
        type=int,
    )
    args = parser.parse_args()

    tcp_server = TCPListener(io_loop=ioloop.IOLoop.current(),
                             model_length=args.model_length)
    tcp_server.listen(args.tcp_port, address='localhost')

    ws_args = (LoopingWebSocket,)

    server_common.main(args, ws_args, 'tcp', args.min_angle, args.max_angle,
                       args.model_length)
