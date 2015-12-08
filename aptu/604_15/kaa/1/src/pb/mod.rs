use std::net::SocketAddr;

use mio::{self, Token};
use mio::tcp::TcpStream;
use protobuf;

mod server;
mod worker;

pub fn start_server<H: ProtoHandler>(addr: SocketAddr, handler: H, n_workers: usize) {
    server::ProtoServer::start(addr, handler, n_workers);
}

pub trait ProtoHandler: Sized + Clone + Send + 'static {
    type Proto: protobuf::Message + protobuf::MessageStatic + Send;
    type Message: Send + Clone;

    fn recv(&mut self, user: &mut User<Self>, message: Self::Proto);
    fn notify(&mut self, user: &mut User<Self>, message: Self::Message);
}

pub struct User<H: ProtoHandler> {
    sender: Sender<H>,
    broadcast: Option<H::Proto>,
    echo: Option<H::Proto>,
}

impl<H: ProtoHandler> User<H> {
    fn new(token: Token, sender: mio::Sender<WorkerMessage<H>>) -> Self {
        User {
            sender: Sender { token: token, sender: sender},
            broadcast: None,
            echo: None,
        }
    }

    pub fn channel(&self) -> Sender<H> {
        self.sender.clone()
    }

    pub fn broadcast(&mut self, message: H::Proto) {
        self.broadcast = Some(message);
    }

    pub fn echo(&mut self, message: H::Proto) {
        self.echo = Some(message);
    }
}

#[derive(Clone)]
pub struct Sender<H: ProtoHandler> {
    token: Token,
    sender: mio::Sender<WorkerMessage<H>>,
}

impl<H: ProtoHandler> Sender<H> {
    pub fn send(&self, message: H::Message)
                -> Result<(), mio::NotifyError<WorkerMessage<H>>> {
        self.sender.send(WorkerMessage::HandlerMessage(self.token, message))
    }
}

pub enum WorkerMessage<H: ProtoHandler> {
    NewConnection(TcpStream),
    HandlerMessage(Token, H::Message),
    Broadcast(H::Proto),
}
