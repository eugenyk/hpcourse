import socket, select, string, sys, struct
import Message_pb2
import time

def has_full_messsage(response):
    size = struct.unpack('!I', response[:4])[0]

    return len(response[4:]) >= size

def put_message(response):
    size = struct.unpack('!I', response[:4])[0]

    message = Message_pb2.ClientMessage()

    message.ParseFromString(response[4:size + 4])

    print(message.Sender + " : " + message.Text)

    response = response[size + 4:]

    return response


def pack_message(sender, text):
    message = Message_pb2.ClientMessage()

    message.Sender = name
    message.Text = text

    return struct.pack('!I', len(message.SerializeToString())) + message.SerializeToString()
    

if __name__ == "__main__":
     
    host = sys.argv[1]
    port = int(sys.argv[2])
    name = sys.argv[3]
 
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverSocket.settimeout(10)
     
    try :
        serverSocket.connect((host, port))
    except :
        print 'Unable to connect'
        sys.exit()
     
    print 'Connected to remote host'
     
    response = ''

    while 1:
        socket_list = [sys.stdin, serverSocket]
         
        read_sockets, write_sockets, error_sockets = select.select(socket_list , [], [])
         
        for currentSocket in read_sockets:

            if currentSocket == serverSocket:

                data = currentSocket.recv(1024)

                if not data :
                    print 'Connection closed'
                    sys.exit()
                else :
                    response = response + data

                    while len(response) >= 4 and has_full_messsage(response):
                        response = put_message(response)
            else :
                message = pack_message(name, sys.stdin.readline())

                serverSocket.send(message)

