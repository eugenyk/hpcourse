#!/usr/bin/env python
import argparse
import random
import socket
import sys
import struct
import Message_pb2
import threading
try:
    import thread
except ImportError:
    import _thread as thread
try:
    import readline
except ImportError:
    import pyreadline as readline

BUFFER = 1024


def cmd_input(sock):
    try:
        cmd = ''
        name = int(random.random() * 10) % 10
        message = Message_pb2.Msg()
        message.Sender = 'client%s' % str(name)

        while cmd != 'exit':
            cmd = str(input(' '))
            if len(cmd) != 0:
                message.Text = cmd
                size = struct.pack('!I', len(message.SerializeToString()))
                sock.send(size + message.SerializeToString())
                # sock.send(message.SerializeToString())
    except:
        return 0


def read_thread(sock):
    while True:
        sys.stdout.write('\r' + ' ' * (len(readline.get_line_buffer()) + 2) + '\r')
        data = None
        try:
            data = sock.recv(BUFFER)
        except:
            print("read error")
            break
        if data is not None:
            size = struct.unpack('!I', data[:4])[0]
            if size == len(data[4:]):
                msg = Message_pb2.Msg()
                msg.ParseFromString(data[4:])
                print(msg.Sender + " : " + msg.Text)
        sys.stdout.write(' ' + readline.get_line_buffer())
        sys.stdout.flush()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--server', default='localhost')  # 172.20.10.3
    parser.add_argument('-p', '--port', default='10001')
    parsed_args = parser.parse_args()
    try:
        sock = socket.socket()
        sock.connect((parsed_args.server, int(parsed_args.port)))
        thread.start_new_thread(read_thread, (sock,))
        cmd_input(sock)
        sock.close()
    except:
        print('\nBye')
    sys.exit(0)
