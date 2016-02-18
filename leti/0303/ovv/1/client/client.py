import socket, select, string, sys, struct
import Message_pb2

if __name__ == "__main__":
     
    host = sys.argv[1]
    port = int(sys.argv[2])
    name = sys.argv[3]
 
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverSocket.settimeout(2)
     
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

                    size = struct.unpack('!I', response[:4])[0]

                    if size == len(response[4:]):
                        message = Message_pb2.ClientMessage()
                        message.ParseFromString(response[4:])
                        print(message.Sender + " : " + message.Text)
                        response = ''
            else :
                
                message = Message_pb2.ClientMessage()

                message.Sender = name

                message.Text = sys.stdin.readline()

                size = struct.pack('!I', len(message.SerializeToString()))

                serverSocket.send(size + message.SerializeToString())