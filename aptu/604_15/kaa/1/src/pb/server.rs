use std::io;
use std::net::SocketAddr;

use mio::{self, Token, EventSet, PollOpt};
use mio::tcp::{TcpStream, TcpListener};

use super::{ProtoHandler, WorkerMessage};
use super::worker::{Workers, Worker};


pub struct ProtoServer<H: ProtoHandler> {
    socket: TcpListener,
    token: Token,
    workers: Workers<H>,
    worker_ptr: usize,
}

impl<H: ProtoHandler> ProtoServer<H> {
    pub fn start(addr: SocketAddr, handler: H, n_workers: usize) {
        let socket = TcpListener::bind(&addr)
        .ok().expect("Failed to bind address");

        let mut event_loop = mio::EventLoop::<Self>::new()
        .ok().expect("Failed to create event loop");

        let workers = Worker::start(handler, n_workers);
        let mut server = ProtoServer {
            socket: socket,
            token: mio::Token(1),
            workers: workers,
            worker_ptr: 0,
        };
        server.register(&mut event_loop)
              .ok().expect("Failed to register server with event loop");

        event_loop.run(&mut server)
                  .ok().expect("Failed to start event loop");
    }
    fn accept(&mut self, event_loop: &mut mio::EventLoop<Self>) {
        info!("accepted a new client socket");
        let socket = match self.socket.accept() {
            Ok(Some(sock)) => sock,
            Ok(None) => {
                error!("Failed to accept a new socket");
                self.reregister(event_loop);
                return;
            }
            Err(e) => {
                error!("Error while accepting connection, {:?}", e);
                self.reregister(event_loop);
                return;
            }
        };

        if let Err(e) = self.round_robin(socket) {
            error!("Failed to deliver connection to client, {:?}", e);
        }

        self.reregister(event_loop);
    }

    fn round_robin(&mut self, sock: TcpStream)
    -> Result<(), mio::NotifyError<WorkerMessage<H>>> {
        self.worker_ptr += 1;
        self.worker_ptr %= self.workers.len();

        let msg = WorkerMessage::NewConnection(sock);
        self.workers[self.worker_ptr].send(msg)
    }

    pub fn register(&mut self, event_loop: &mut mio::EventLoop<Self>)
    -> io::Result<()> {
        event_loop.register_opt(
            &self.socket,
            self.token,
            EventSet::readable(),
            PollOpt::edge() | PollOpt::oneshot()
        ).or_else(|e| {
            error!("Failed to register server {:?}, {:?}", self.token, e);
            Err(e)
        })
    }

    fn reregister(&mut self, event_loop: &mut mio::EventLoop<Self>) {
        event_loop.reregister(
            &self.socket,
            self.token,
            EventSet::readable(),
            PollOpt::edge() | PollOpt::oneshot()
        ).unwrap_or_else(|e| {
            error!("Failed to reregister server {:?}, {:?}", self.token, e);
            event_loop.shutdown();
        })
    }

}

impl<H: ProtoHandler> mio::Handler for ProtoServer<H> {
    type Timeout = ();
    type Message = ();

    fn ready(&mut self,
    event_loop: &mut mio::EventLoop<Self>,
    token: mio::Token,
    events: mio::EventSet) {
        debug!("events = {:?}", events);
        assert!(token != mio::Token(0), "[BUG]: Received event for Token(0)");
        assert!(self.token == token, "Received writable event for server");

        if events.is_error() {
            error!("Error event for server");
            event_loop.shutdown();
            return;
        }

        if events.is_readable() {
            trace!("Read event for {:?}", token);
            assert!(token == self.token, "Server received unexpected token");
            self.accept(event_loop);
        }
    }
}

