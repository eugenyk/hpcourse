import argparse
import asyncore
import re
import socket
from multiprocessing import Pool, Process
import struct
from subprocess import Popen, PIPE
import threading
import Message_pb2

clients = []


def manage_script(args):
    child = Popen(args, stdout=PIPE, stderr=PIPE)
    output = child.stdout.read()
    err = child.stderr.read()
    child.communicate()
    rc = child.returncode
    if rc == 0:
        return output
    else:
        return err


def cmd_run(text, sock):
    text = text.split(' ')[1:]
    res = manage_script(text)
    msg = Message_pb2.Msg()
    msg.Sender = "server"
    msg.Text = res
    size = struct.pack('!I', len(msg.SerializeToString()))
    sock.send(size + msg.SerializeToString())


def if_cmd(text):
    prog = re.compile(r"/c (.*)")
    return prog.match(text)


def reader(message):
    size = struct.unpack('!I', message[:4])[0]
    if size == len(message[4:]):
        msg = Message_pb2.Msg()
        msg.ParseFromString(message[4:])
        if if_cmd(msg.Text):
            return dict(type="cmd", response=msg)
        return dict(type="message", response=msg)
    return dict(type="error", response="error")


def broadcast(message):
    size = struct.pack('!I', len(message.SerializeToString()))
    for client in clients:
        try:
            client.send(size + message.SerializeToString())
        except:
            clients.remove(client)


class CmdThread(threading.Thread):
    def __init__(self, cmd, sock):
        threading.Thread.__init__(self)
        self.cmd = cmd
        self.sock = sock
    def run(self):
        cmd_run(self.cmd, self.sock)

class Handler(asyncore.dispatcher_with_send):
    def __init__(self, sock, threads, buffer):
        asyncore.dispatcher_with_send.__init__(self, sock)
        self.threads = threads
        self.buffer = buffer
        self.p = Pool(threads)
        self.sock = sock

    def handle_read(self):
        data = self.recv(self.buffer)
        if data:
            res = self.p.apply_async(reader, [data])
            message = res.get()
            if message['type'] == "message":
                broadcast(message['response'])
                res.get()
            elif message['type'] == "cmd":
                thread = CmdThread(message['response'].Text, self)
                thread.start()


class Server(asyncore.dispatcher):
    def __init__(self, host, port, threads, buffer):
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind((host, port))
        self.listen(5)
        self.threads = threads
        self.buffer = buffer


    def handle_accept(self):
        pair = self.accept()
        if pair is not None:
            sock, addr = pair
            print 'Incoming connection from %s' % repr(addr)
            clients.append(sock)
            Handler(sock, self.threads, self.buffer)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--server', default='localhost')
    parser.add_argument('-p', '--port', default='10000')
    parser.add_argument('-t', '--threads', default='5')
    parser.add_argument('-b', '--buffer', default='4096')
    parsed_args = parser.parse_args()
    server = Server(parsed_args.server, int(parsed_args.port), int(parsed_args.threads), int(parsed_args.buffer))
    asyncore.loop()