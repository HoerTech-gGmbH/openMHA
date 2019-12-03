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

from __future__ import division

import json

from MHAConnection import MHAConnection
import server_common


class LoopingWebSocket(server_common.MyWebSocketHandler):

    def __init__(self, *args, **kwargs):

        # grab our private keyword arguments
        self.mha_host = kwargs.pop('mha_host')
        self.mha_port = kwargs.pop('mha_port')
        self.interval = None
        self.pooling_id = kwargs.pop('pooling_id')
        pool_path = kwargs.pop('pool_path')

        # cache the location of the pooling plug-in
        with MHAConnection(self.mha_host, self.mha_port, self.interval) as mha_conn:
            self._plugin_path = mha_conn.find_id(self.pooling_id)[0]

        # If --pool-path was not passed, default to looking for a monitoring
        # plug-in in the same namespace as the acPooling_wave plug-in.
        if not pool_path:
            mon_path = self._plugin_path.replace(self.pooling_id, b'doasvm_mon')
            pool_path = mon_path + b'.pool'
        self._pool_path = pool_path

        super(LoopingWebSocket, self).__init__(*args, **kwargs)

    def _send_data(self):

        try:
            with MHAConnection(self.mha_host, self.mha_port, self.interval) as mha_conn:
                p = mha_conn.get_val_converted(self._pool_path)
                self.write_message(json.dumps({'data': p}))
        except ValueError as e:
            print("Error sending data: {}".format(e))

    def on_message(self, message):
        message = json.loads(message)
        try:
            if 'command' in message:
                if message['command'] == 'send_data':
                    self._send_data()
                else:
                    print('Unknown command "{}"'.format(message['command']))
            elif 'new_pooling_wndlen' in message:
                print('Pooling wndlen = {}'.format(message['new_pooling_wndlen']))
                with MHAConnection(self.mha_host, self.mha_port, self.interval) as mha_conn:
                    mha_conn.set_val(self._plugin_path + b'.pooling_wndlen',
                                     message['new_pooling_wndlen'])
            elif 'new_pooling_alpha' in message:
                print('Pooling alpha = {}'.format(message['new_pooling_alpha']))
                with MHAConnection(self.mha_host, self.mha_port, self.interval) as mha_conn:
                    mha_conn.set_val(self._plugin_path + b'.alpha',
                                     message['new_pooling_alpha'])
            elif 'new_pooling_type' in message:
                print('Pooling type = {}'.format(message['new_pooling_type']))
                with MHAConnection(self.mha_host, self.mha_port, self.interval) as mha_conn:
                    mha_conn.set_val(self._plugin_path + b'.pooling_type',
                                     message['new_pooling_type'])
            elif 'beamformer' in message:
                print('Beamformer = {}'.format(message['beamformer']))
                with MHAConnection(self.mha_host, self.mha_port, self.interval) as mha_conn:
                    if(message['beamformer']==False):
                        mha_conn.set_val(b'mha.doachain.post.select',"NoBf")
                    elif(message['beamformer']==True):
                        mha_conn.set_val(b'mha.doachain.post.select',"Bf")
                    else:
                        print('Unknown message "{}"'.format(message))
            elif 'new_interval' in message:
                print('Interval = {}'.format(message['new_interval']))
                self.interval = message['new_interval']
            else:
                print('Unknown message "{}"'.format(message))
        except Exception as e:
            print("Error handling message \"{}\": {}".format(message, e))

if __name__ == '__main__':

    import argparse

    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    server_common.add_common_args(parser)
    parser.add_argument(
        '--mha-host',
        default='127.0.0.1',
        help='The host on which MHA is running.',
    )
    parser.add_argument(
        '--mha-port',
        default=33337,
        type=int,
        help='The port on which MHA is listening.',
    )
    parser.add_argument(
        '--classification-id',
        default=b'doasvm_classification',
        type=lambda s: (s if type(s) == bytes else s.encode()),
        help="""The ID of a doasvm_classification instance.  This is used to
        fetch the "angles" variable in order to pass additional parameters to
        the web app.
        """
    )
    parser.add_argument(
        '--pooling-id',
        default=b'acPooling_wave',
        type=lambda s: (s if type(s) == bytes else s.encode()),
        help="""The ID of the desired acPooling_wave instance.  This is the
        instance that will be controlled from the web app.
        """
    )
    parser.add_argument(
        '--pool-path',
        default=b'',
        type=lambda s: (s if type(s) == bytes else s.encode()),
        help="""The full path to the desired "pool" variable to visualise.  If
        unset, it is assumed that a doasvm_mon instance (named "doasvm_mon")
        exists in the same namespace as the pooling plug-in specified by
        --pooling-id, and that it has has a variable named "pool".
        """
    )
    args = parser.parse_args()

    # abort the connection after a 5 second timeout
    with MHAConnection(args.mha_host, args.mha_port, 5) as mha_conn:
        plugin_path = mha_conn.find_id(args.classification_id)
        if not plugin_path:
            classification_id = args.classification_id.decode()
            exit('Error: Could not find plug-in with ID "' + classification_id
                 + '"')
        angles = mha_conn.get_val_converted(plugin_path[0] + b'.angles')

    ws_args = (
        LoopingWebSocket, {'mha_host': args.mha_host,
                           'mha_port': args.mha_port,
                           'pooling_id': args.pooling_id,
                           'pool_path': args.pool_path}
    )

    server_common.main(args, ws_args, 'mha', min(angles), max(angles),
                       len(angles))
