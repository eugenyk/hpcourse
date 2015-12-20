use std::io;
use std::thread;

use mio::{self, Token, EventSet};
use mio::buf::{ByteBuf, Buf};
use mio::tcp::TcpStream;
use mio::util::Slab;
use time;

use super::ProtoHandler;
use super::{User, Sender, WorkerMessage};

mod connection;
mod utils;

use self::utils::*;
use self::connection::Connection;

pub type Workers<H> = Vec<mio::Sender<WorkerMessage<H>>>;

pub struct Worker<H: ProtoHandler> {
    id: usize,
    handler: H,
    connections: Slab<Connection<H::Proto>>,
    peers: Workers<H>,
    loop_end: u64
}

impl<H: ProtoHandler> Worker<H> {

    fn new(id: usize, handler: H, peers: Workers<H>) -> Self {
        assert!(id > 0);
        Worker {
            id: id,
            handler: handler,
            connections: Slab::new_starting_at(Token(100 * id), 100_000),
            peers: peers,
            loop_end: 0,
        }
    }

    pub fn start(handler: H, n_workers: usize) -> Workers<H> {
        assert!(n_workers > 0, "Need at least one worker");
        let loops = (0..n_workers)
        .map(|_| mio::EventLoop::<Self>::new().ok().expect("Failed to crate a worker"))
        .collect::<Vec<_>>();
        let chans = loops.iter().map(|l| l.channel()).collect::<Vec<_>>();

        for (id, mut l) in loops.into_iter().enumerate() {
            let peers = (0..n_workers)
            .filter(|&i| i != id).map(|i| chans[i].clone()).collect();
            let handler = handler.clone();

            thread::spawn(move || {
                l.run(&mut Worker::new(id + 1, handler, peers))
                .ok().expect("Failed to start a worker event loop");
            });

        }

        chans
    }

    fn accept(&mut self, event_loop: &mut mio::EventLoop<Self>, sock: TcpStream) {
        sock.set_nodelay(true).unwrap_or_else(|e|
                                              error!("Failed to set nodelay, {:?}", e));

        match self.connections.insert_with(|token| Connection::new(sock, token)) {
            Some(token) => {
                match self.connections[token].register(event_loop) {
                    Ok(_) => {},
                    Err(e) => {
                        error!("Failed to register connection, {:?}", e);
                        self.connections.remove(token);
                    }
                }
            }
            None => error!("Failed to insert connection into slab")
        }
    }

    fn readable(&mut self,
                event_loop: &mut mio::EventLoop<Self>,
                token: Token)
                -> io::Result<()> {

        while let Some(msg) = try!(self.connections[token].readable()) {
            let mut user = User::new(token, event_loop.channel());
            self.handler.recv(&mut user, msg);
            self.perform_requests(event_loop, token, user);
        }
        Ok(())
    }

    fn perform_requests(&mut self,
                        event_loop: &mut mio::EventLoop<Self>,
                        token: Token,
                        user: User<H>) {
        let  User {broadcast, echo, ..} = user;

        if let Some(proto) = broadcast {
            self.broadcast(event_loop, proto);
        }

        if let Some(proto) = echo {
            let buf = to_buf(&proto);
            self.connections[token].send_message(buf)
                .and_then(|_| self.connections[token].reregister(event_loop))
                .unwrap_or_else(|_| self.reset_connection(token));
        }
    }

    fn reset_connection(&mut self, token: Token) {
        info!("reset connection {:?}", token);
        self.connections.remove(token);
    }

    fn broadcast(&mut self, event_loop: &mut mio::EventLoop<Self>, proto: H::Proto) {
        for p in self.peers.iter() {
            if let Err(e) = p.send(WorkerMessage::Broadcast(proto.clone())) {
                error!("cannot forward post to peer, {:?}", e);
            }
        }
        self.broadcast_local(event_loop, proto)
    }

    fn broadcast_local(&mut self, event_loop: &mut mio::EventLoop<Self>, proto: H::Proto) {
        let buf = to_buf(&proto);
        let mut bad_tokens = Vec::new();
        for conn in self.connections.iter_mut() {
            conn.send_message(ByteBuf::from_slice(buf.bytes()))
                .and_then(|_| conn.reregister(event_loop))
                .unwrap_or_else(|e| {
                    error!("Failed to send message for {:?}: {:?}", conn.token, e);
                    bad_tokens.push(conn.token);
                });
        }

        for t in bad_tokens {
            self.reset_connection(t);
        }
    }
}


impl<H: ProtoHandler> mio::Handler for Worker<H> {
    type Timeout = ();
    type Message = WorkerMessage<H>;

    fn ready(&mut self,
             event_loop: &mut mio::EventLoop<Self>,
             token: Token,
             events: EventSet) {
        let start = time::precise_time_ns();
        debug!("\n\ntime since last: {} ns ", start - self.loop_end);
        debug!("events = {:?}", events);
        assert!(token != Token(0), "[BUG]: Received event for Token(0)");

        if events.is_error() || events.is_hup() {
            if events.is_error() {
                error!("Error event for {:?}", token);
            } else {
                warn!("Hup event for {:?}", token);
            }

            self.reset_connection(token);
            return;
        }

        if events.is_writable() {
            trace!("Write event for {:?}", token);

            self.connections[token].writable()
            .and_then(|_| self.connections[token].reregister(event_loop))
            .unwrap_or_else(|e| {
                error!("Write event failed for {:?}, {:?}", token, e);
                self.reset_connection(token);
            });
        }

        if events.is_readable() {
            trace!("Read event for {:?}", token);
            self.readable(event_loop, token)
            .and_then(|_| self.connections[token].reregister(event_loop))
            .unwrap_or_else(|e| {
                error!("Read event failed for {:?}: {:?}", token, e);
                self.reset_connection(token);
            });
        }
        let end = time::precise_time_ns();
        self.loop_end = end;
        debug!("loop duration: {} ns", end - start);
    }

    fn notify(&mut self, event_loop: &mut mio::EventLoop<Self>, msg: Self::Message) {
        info!("Worker {} received a message", self.id);
        match msg {
            WorkerMessage::NewConnection(sock) => {
                self.accept(event_loop, sock)
            }
            WorkerMessage::HandlerMessage(token, m) => {
                let mut user = User::new(token, event_loop.channel());
                self.handler.notify(&mut user, m);
                self.perform_requests(event_loop, token, user);
            }
            WorkerMessage::Broadcast(post) => {
                self.broadcast_local(event_loop, post)
            }
        }
    }
}
