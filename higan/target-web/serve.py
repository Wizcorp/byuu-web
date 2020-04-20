#!/usr/bin/env python

import sys
import os
import re
import json

import SimpleHTTPServer
import BaseHTTPServer
import SocketServer

PORT = 8000

SimpleHTTPServer.SimpleHTTPRequestHandler.extensions_map.update({
    '.wasm': 'application/wasm',
    '.json': 'application/json',
    '.js': 'text/javascript'
})

class ThreadingSimpleServer(SocketServer.ThreadingMixIn, BaseHTTPServer.HTTPServer):
    pass
class RequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        SimpleHTTPServer.SimpleHTTPRequestHandler.end_headers(self)
    
    def translate_path(self, path):
        if path == '/':
            path = '/index.html'

        for prefix in sys.argv:
            if '=' in prefix:
                (prefix, subpath) = prefix.split('=', 1)
                path = re.sub(r'^{0}'.format(subpath), '', path)
            fullpath = prefix + path

            if os.path.isfile(fullpath):
                return fullpath
        return SimpleHTTPServer.SimpleHTTPRequestHandler.translate_path(self, path)

print "Serving at port", PORT
server = ThreadingSimpleServer(('', PORT), RequestHandler)

try:
    while 1:
        sys.stdout.flush()
        server.handle_request()
except KeyboardInterrupt:
    print "\nShutting down..."
    sys.exit(0)
