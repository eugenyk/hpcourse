#[macro_use] extern crate log;
extern crate env_logger;
extern crate mio;
extern crate protobuf;
extern crate byteorder;
extern crate time;

use std::net::SocketAddr;

pub mod post;
pub mod proto_reader;
mod pb;
mod chat;
mod shell;


pub fn start_server(addr: SocketAddr, n_workers: usize) {
    println!("Starting server at {}", addr);
    let shell = shell::start();
    let handler = chat::ChatUser { shell: shell };

    pb::start_server(addr, handler, n_workers);
}
