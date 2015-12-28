use std::io;

use protobuf;

use mio::{self, Token, EventSet, PollOpt, TryWrite, Handler};
use mio::buf::{Buf, ByteBuf};
use mio::tcp::TcpStream;

use proto_reader::ProtoReader;

pub struct Connection<M: protobuf::MessageStatic> {
    pub token: mio::Token,
    socket: TcpStream,
    interest: EventSet,
    send_queue: Vec<ByteBuf>,
    proto_reader: ProtoReader<M>,
}

impl<M: protobuf::MessageStatic> Connection<M> {
    pub fn new(socket: TcpStream, token: Token) -> Self {
        Connection {
            token: token,
            socket: socket,
            interest: EventSet::hup(),
            send_queue: Vec::new(),
            proto_reader: ProtoReader::new(),
        }
    }

    pub fn register<H: Handler>(&mut self, event_loop: &mut mio::EventLoop<H>) -> io::Result<()> {
        self.interest.insert(EventSet::readable());

        event_loop.register_opt(
            &self.socket,
            self.token,
            self.interest,
            PollOpt::edge() | PollOpt::oneshot()
        ).or_else(|e| {
            error!("Failed to register {:?}, {:?}", self.token, e);
            Err(e)
        })
    }

    pub fn reregister<H: Handler>(&mut self, event_loop: &mut mio::EventLoop<H>) -> io::Result<()> {
        event_loop.reregister(
            &self.socket,
            self.token,
            self.interest,
            PollOpt::edge() | PollOpt::oneshot()
        ).or_else(|e| {
            error!("Failed to reregister {:?}, {:?}", self.token, e);
            Err(e)
        })
    }

    pub fn send_message(&mut self, message: ByteBuf) -> io::Result<()> {
        self.send_queue.push(message);
        self.interest.insert(EventSet::writable());
        Ok(())
    }

    pub fn readable(&mut self) -> io::Result<Option<M>> {
        match self.proto_reader.read(&mut self.socket) {
            Ok(buf) => Ok(Some(buf)),
            Err(e) => match e.kind()  {
                io::ErrorKind::WouldBlock => return Ok(None),
                _ => return Err(e)
            }
        }
    }

    pub fn writable(&mut self) -> io::Result<()> {
        debug!("queue size for {:?} is {}", self.token, self.send_queue.len());

        while let Some(mut buf) = self.send_queue.pop() {
            match self.socket.try_write_buf(&mut buf) {
                Ok(None) => {
                    debug!("client flushing buf");
                    self.send_queue.push(buf);
                    break;
                }
                Ok(Some(n)) => {
                    debug!("Wrote {} bytes for {:?}", n, self.token);
                    if buf.has_remaining() {
                        self.send_queue.push(buf);
                        break;
                    }
                },
                Err(e) => {
                    error!("Failed to send buffer for {:?}, error: {:?}", self.token, e);
                    return Err(e);
                }
            }
        }

        if self.send_queue.is_empty() {
            self.interest.remove(EventSet::writable());
        }

        Ok(())
    }
}
