#ifndef CommandExecutor_h
#define CommandExecutor_h

#include <iostream>

#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Notification.h>
#include <Poco/NotificationQueue.h>

#include <Poco/Process.h>
#include <Poco/PipeStream.h>
#include <Poco/StreamCopier.h>

class CommandNotification: public Poco::Notification
{
public:
    CommandNotification(const std::string command, Poco::Net::StreamSocket& socket): _command(command), _socket(socket) {}
    
    const std::string command() const
    {
        return _command.substr(3, _command.length() - 3 - 2);
    }
    
    Poco::Net::StreamSocket& socket()
    {
        return _socket;
    }
private:
    const std::string _command;
    Poco::Net::StreamSocket _socket;
};


class CommandExecutor : public Poco::Runnable {
    Poco::NotificationQueue& _queue;
   
public:
    CommandExecutor(Poco::NotificationQueue& queue): _queue(queue) {}
    
    void run() {
        Poco::AutoPtr<Poco::Notification> pNf(_queue.waitDequeueNotification());
        while (pNf)
        {
            CommandNotification* pWorkNf =
            dynamic_cast<CommandNotification*>(pNf.get());
            
            char* response = (char*)::operator new (1000);
            
            if (pWorkNf)
            {
                Poco::Pipe outPipe;
                Poco::Process::Args args;
                
                Poco::ProcessHandle ph(Poco::Process::launch(pWorkNf->command(), args, 0, &outPipe, 0));
                Poco::PipeInputStream istr(outPipe);
                
                istr >> response;
                ph.wait();
                
                pWorkNf->socket().sendBytes(response, 1000);
                pWorkNf->socket().sendBytes("\n", 1);
            }
            
            pNf = _queue.waitDequeueNotification();
        }
    }
    

};


#endif /* CommandExecutor_h */
