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
        self.read_buffer = b''

    def handle_read(self):
        data = self.recv(1024)

        self.read_buffer += data

        try:
            obj, end_index = self.decoder.raw_decode(
                self.read_buffer.decode('UTF-8')
                )
        except ValueError:
            # Probably didn't read enough
            return
        self.read_buffer = self.read_buffer[end_index:]

        #FIXME: check method name (must be handle_payment)
        #FIXME: handle exceptions in handle_payment
        result = self.router.handle_payment(**obj['params'])

        s = json.dumps({'id': obj['id'], 'result': result})
        self.send(bytearray(s, 'UTF-8'))

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

    BADONION = 0x8000
    PERM     = 0x4000
    NODE     = 0x2000
    UPDATE   = 0x1000

    #return values for handle_payment:
    OK                     = 0
    INVALID_REALM          = PERM|1
    TEMPORARY_NODE_FAILURE = NODE|2
    PERMANENT_NODE_FAILURE = PERM|NODE|2
    #FIXME: add the rest (see BOLT #4)

    def handle_payment(self, realm):
        #to be replaced in derived classes
        #By default, fail a transaction immediately
        return CustomRouter.PERMANENT_NODE_FAILURE

