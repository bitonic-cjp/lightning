import json
import logging
import os
import socket
import asyncore


class Connection(asyncore.dispatcher_with_send):
    def __init__(self, socket, router):
        asyncore.dispatcher_with_send.__init__(self, socket, map=router.channelMap)
        self.router = router
        self.decoder = json.JSONDecoder()

    def handle_read(self):
        data = self.recv(1024)
        #FIXME: do parsing
        self.router.handle_payment(data)
        #FIXME: handle exceptions / return value in handle_payment

    def handle_close(self):
        self.router.connections.remove(self)



class Listener(asyncore.dispatcher):
    def __init__(self, socket_path, router):
        asyncore.dispatcher.__init__(self, map=router.channelMap)
        self.create_socket(socket.AF_UNIX, socket.SOCK_STREAM)

        #of course, this is racy!
        try:
            self.connect(socket_path)
        except:
            pass
        else:
            raise Exception("Router filename '%s' in use" % socket_path)
        try:
            os.unlink(socket_path)
        except FileNotFoundError:
            pass
        self.bind(socket_path)

        self.listen(5)
        self.router = router

    def handle_accept(self):
        sock, addr = self.accept()
        self.router.connections.append(Connection(sock, self.router))



class CustomRouter:
    def __init__(self, socket_path, logger=logging, map=None):
        self.logger = logger
        self.connections = []
        self.channelMap = map
        self.listener = Listener(socket_path, self)

    def handle_payment(self, data):
        pass #to be replaced in derived classes

