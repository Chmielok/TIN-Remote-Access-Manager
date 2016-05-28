#!/usr/bin/python
'''
SimpleSecureHTTPServer.py - simple HTTP server supporting SSL.
usage: python SimpleSecureHTTPServer.py
'''
import socket, os
from threading import Lock
from SocketServer import ThreadingMixIn
from SocketServer import BaseServer
from BaseHTTPServer import HTTPServer
from SimpleHTTPServer import SimpleHTTPRequestHandler
from OpenSSL import SSL
import urlparse

CERT = 'server.pem'
KEY = 'key.pem'

class UserSessionStore():
    lock = Lock()
    


class SecureHTTPServer(HTTPServer):
    def __init__(self, server_address, HandlerClass):
        BaseServer.__init__(self, server_address, HandlerClass)
        ctx = SSL.Context(SSL.SSLv23_METHOD)
        ctx.use_privatekey_file (KEY)
        ctx.use_certificate_file(CERT)
        self.socket = SSL.Connection(ctx, socket.socket(self.address_family,
                                                        self.socket_type))
        self.server_bind()
        self.server_activate()
        
    def shutdown_request(self,request): 
        request.shutdown()

class SecureHTTPRequestHandler(SimpleHTTPRequestHandler):
    def setup(self):
        self.connection = self.request
        self.rfile = socket._fileobject(self.request, "rb", self.rbufsize)
        self.wfile = socket._fileobject(self.request, "wb", self.wbufsize)
    
    def do_POST(self):
        """Processes POST requests"""
        print(self.headers)
        self.send_response(200)
        content_len = int(self.headers.getheader('content-length', 0))
        post_data = urlparse.parse_qs(self.rfile.read(content_len).decode('utf-8'))
        for (key, value) in post_data.iteritems():
            if key == "login":
                login = value
            elif key == "password":
                password = value
        if not signIn(login, password):
            self.send_response(401, "Invalid login/password")


def signIn(login, password):
    """Checks whether user is signed in"""
    return False
        
class HTTPSServerMT(ThreadingMixIn, SecureHTTPServer):
    """Handle requests in separate thread"""

def test():
    server = HTTPSServerMT(('localhost', 4443), SecureHTTPRequestHandler)
    server.serve_forever()


if __name__ == '__main__':
    test()
