import socket, select, string, sys, struct
import Message_pb2


if __name__ == "__main__":
     
    host = sys.argv[1]
    port = int(sys.argv[2])
    name = sys.argv[3]
     
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)
     
    try :
        s.connect((host, port))
    except :
        print 'Unable to connect'
        sys.exit()
     
    print 'Connected to remote host'
     
    while 1:
        socket_list = [sys.stdin, s]
         
        read_sockets, write_sockets, error_sockets = select.select(socket_list , [], [])
         
        for sock in read_sockets:

            if sock == s:
                data = sock.recv(4096)
                if not data :
                    print 'Connection closed'
                    sys.exit()
                else :
                    sys.stdout.write(data)
            else :
                
                message = Message_pb2.ClientMessage()

                message.Sender = 'client(%s)' % str(name)

                message.Text = sys.stdin.readline()

                size = struct.pack('!I', len(message.SerializeToString()))

                s.send(size + message.SerializeToString())