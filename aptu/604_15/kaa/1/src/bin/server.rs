#[macro_use] extern crate log;
extern crate env_logger;
extern crate rustc_serialize;
extern crate docopt;
extern crate chat;

use std::net::SocketAddr;
use std::str::FromStr;
use std::cmp::max;

const USAGE: &'static str = "
Mio chat

Usage:
  chat [--workers=<n_workers> --addr=<host:port>]
  chat (-h | --help)

Options:
  -w, --workers=<n_workers>   Number of worker threads.
  --addr=<host:port>          Port to listen [default: 0.0.0.0:20053]
  -h, --help                  Show this screen.
";

#[derive(Debug, RustcDecodable)]
struct Args {
    flag_workers: usize,
    flag_addr: String
}


pub fn main() {
    env_logger::init().ok().expect("Failed to init logger");

    let args: Args = docopt::Docopt::new(USAGE)
        .and_then(|d| d.options_first(true).decode())
        .unwrap_or_else(|e| e.exit());

    let n_workers = max(args.flag_workers, 1);
    let addr = args.flag_addr;

    let addr: SocketAddr = FromStr::from_str(&addr)
        .ok().expect("Failed to parse host:port string");

    chat::start_server(addr, n_workers);
}


