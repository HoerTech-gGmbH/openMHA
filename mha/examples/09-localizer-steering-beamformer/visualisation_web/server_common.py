import os

try:
    from urllib.parse import quote as url_quote
except:
    from urllib import quote as url_quote

from tornado import httpserver, ioloop, web, websocket

# the set of supported plot types
PLOT_TYPES = frozenset(('2d', 'pseudo3d', 'polar', 'tiled'))


class MyWebSocketHandler(websocket.WebSocketHandler):

    def open(self):

        print("WebSocket opened")

    # needed for compatibility with tornado 4.x to prevent rejected connections
    def check_origin(self, origin):

        return True

    def on_close(self):
        print("WebSocket closed")


def add_common_args(parser):

    parser.add_argument(
        '--browser', '-b',
        help='The browser to launch.',
    )
    parser.add_argument(
        '--no-browser',
        action='store_true',
        help='Do not launch a browser.',
    )
    parser.add_argument(
        '--no-cache',
        dest='cache',
        action='store_false',
        help="""Tell tornado not to cache the files served by the web app.
        If unset, you must restart the server to see changes made to any part
        of the web application.
        """
    )
    parser.add_argument(
        '--host',
        default='localhost',
        help="""The host on which the HTTP and WebSocket handlers will listen.
        Change this if you want the web applications to be available remotely.
        However, first read the section "Security Concerns" in the README.
        """,
    )
    parser.add_argument(
        '--http-port',
        default=8080,
        type=int,
        help='The port on which the HTTP handler will listen.',
    )
    parser.add_argument(
        '--ws-port',
        default=8888,
        type=int,
        help='The port on which the WebSocket handler will listen.',
    )
    parser.add_argument(
        'type',
        nargs='?',
        choices=PLOT_TYPES,
        default='2d',
        help='The type of plot.  See the README for detailed descriptions.',
    )


def main(args, ws_args, server_type, *url_fmt_args):

    import webbrowser

    url = 'visualisation/{}.html?' + url_quote(
        'ws_port={}&server_type={}&min_angle={}&max_angle={}&model_length={}'
        .format(args.ws_port, server_type, *url_fmt_args)
    )
    default_url = url.format(args.type)

    type_redirects = [('/' + t, web.RedirectHandler, {'url': url.format(t),
                                                      'permanent': False})
                      for t in PLOT_TYPES]

    app = web.Application(
        type_redirects +
        [('/', web.RedirectHandler, {'url': default_url, 'permanent': False}),
         ('/ws',) + ws_args,
         ('/(.*)', web.StaticFileHandler, {'path':
                                           os.path.dirname(__file__)})],
        # static_hash_cache=False allows changes to be seen on page reload; see
        # http://www.tornadoweb.org/en/stable/web.html#tornado.web.Application
        static_hash_cache=args.cache,
    )
    app.listen(args.ws_port, address=args.host)

    http_server = httpserver.HTTPServer(app)
    http_server.listen(args.http_port, address=args.host)

    webbrowser.register('dwb', None, webbrowser.BackgroundBrowser('dwb'))
    webbrowser.register('rekonq', None, webbrowser.BackgroundBrowser('rekonq'))

    if not args.no_browser:
        browser = webbrowser.get(args.browser)
        browser.open(
            'http://{}:{}/{}'.format((args.host if args.host else 'localhost'),
                                     args.http_port, default_url)
        )

    ioloop.IOLoop.current().start()
