extern crate byteorder;
extern crate chat;
extern crate time;
extern crate simple_parallel;
extern crate docopt;
extern crate rustc_serialize;
extern crate protobuf;


use std::net;
use std::io::{Write, Read};
use std::str::FromStr;

use chat::post::Post;
use chat::proto_reader::ProtoReader;

const USAGE: &'static str = "
bench

Usage:
  bench [--rps --packed --med --large --huge --c10k]
  chat (-h | --help)

Options:
  --rps          Request per second benchmark. Single connection.
  --packed       Messages per second benchmark. Several message in packet. Single connection.
  --med          Megabytes per second benchmark. 1k message, single connection.
  --large        Megabytes per second benchmark. 2.7mb message, single connection.
  --huge         Megabytes per second benchmark. 800mb message, single connection.
  --c10k         10k concurrent connections benchmark. Four threads.
  -h, --help     Show this screen.
";


#[derive(Debug, RustcDecodable)]
struct Args {
    flag_rps: bool,
    flag_packed: bool,
    flag_med: bool,
    flag_large: bool,
    flag_huge: bool,
    flag_c10k: bool,
}



fn main() {
    let args: Args = docopt::Docopt::new(USAGE)
        .and_then(|d| d.options_first(true).decode())
        .unwrap_or_else(|e| e.exit());
    if args.flag_rps {
        println!("rps benchmark");
        rps();
    }
    if args.flag_packed {
        println!("\n\npacked benchmark");
        packed();
    }
    if args.flag_med {
        println!("\n\nmed benchmark");
        med();
    }
    if args.flag_large {
        println!("\n\nlarge benchmark");
        large();
    }
    if args.flag_huge {
        println!("\n\nhuge benchmark");
        huge();
    }
    if args.flag_c10k {
        println!("\n\nc10k benchmark");
        c10k();
    }
    println!("\n\nBenchmarks finished");
}

fn message(message_size: usize) -> Post {
    let text = std::iter::repeat("Hello, World!").take(message_size)
    .collect::<Vec<_>>()
    .join(" ");
    Post::from_text("matklad".to_string(), vec![text.to_string()])
}

fn c10k() {
    let addr: net::SocketAddr = FromStr::from_str("0.0.0.0:20053").unwrap();
    let mut socks = Vec::new();
    let message = message(1).to_bytes();
    let start = time::precise_time_s();
    let mut pool = simple_parallel::Pool::new(4);
    for n_cons in 0..10_000 {
        if n_cons % 500 == 0 {
            println!("{} concurrent connections, {:.2} seconds",
                     n_cons, time::precise_time_s() - start);
        }
        let mut sock = net::TcpStream::connect(&addr).unwrap();
        if n_cons + 1 == 10_000 {
            println!("c10k!");
        }
        sock.write_all(&message).unwrap();
        socks.push(sock);
        pool.for_(socks.iter_mut(), |mut sock| {
            sock_read_post(&mut sock);
        });
    }

    let end = time::precise_time_s();
    let duration = end - start;

    println!("time {:.2} seconds", duration);
}


fn packed() {
    let addr: net::SocketAddr = FromStr::from_str("0.0.0.0:20053").unwrap();
    let n_requests = 100_000;
    let pack = 4;
    let mut sock = net::TcpStream::connect(&addr).unwrap();

    let mut bytes_writen = 0;
    let start = time::precise_time_s();
    let message = message(1).to_bytes();
    let message_len = message.len();
    println!("message len {} bytes", message_len);

    let buffer = {
        let mut v = Vec::with_capacity(message_len * 4);
        for _ in 0..pack {
            v.extend(message.iter());
        }
        v
    };
    for _ in 0..n_requests / pack {
        bytes_writen += buffer.len();
        sock.write_all(&buffer).unwrap();

        for _ in 0..pack {
            sock_read_post(&mut sock);
        }

    }

    let end = time::precise_time_s();
    let duration = end - start;
    let mb = bytes_writen / 1024 / 1024;
    println!("Written {} megabytes", mb);
    println!("time {:.2} seconds", duration);
    println!("Throughput {:.2} mb/s", mb as f64 / duration);
    println!("Throughput {:.2} messages/s", n_requests as f64 / duration);
}


fn rps()   { requests(100_000, 1)         }
fn med()   { requests(100_000, 73)        }
fn large() { requests(100    , 200000)    }
fn huge()  { requests(5      , 60000000)  }

fn requests(n_requests: u32, message_size: usize) {
    let addr: net::SocketAddr = FromStr::from_str("0.0.0.0:20053").unwrap();

    let mut sock = net::TcpStream::connect(&addr).unwrap();

    let message = {
        let m = message(message_size);
        m.to_bytes()
    };
    let mut bytes_writen = 0;
    let start = time::precise_time_s();
    let message_len = message.len();
    let message_len_kb = message_len / 1024;
    let message_len_mb = message_len_kb / 1024;
    if message_len_mb == 0 {
        println!("message len {} bytes", message_len);
    } else if message_len_mb < 10 {
        println!("message len {} kb", message_len_kb);
    } else {
        println!("message len {} mb", message_len_mb);
    }
    for _ in 0..n_requests {
        bytes_writen += message.len();
        sock.write_all(&message).unwrap();
        sock_read_post(&mut sock);
    }

    let end = time::precise_time_s();
    let duration = end - start;
    let mb = bytes_writen / 1024 / 1024;
    println!("Written {} megabytes", mb);
    println!("time {:.2} seconds", duration);
    println!("Throughput {:.2} mb/s", mb as f64 / duration);
    println!("Throughput {:.2} requests/s", n_requests as f64 / duration);
}


fn sock_read_post(mut sock: &mut net::TcpStream) -> Post {
    let mut reader = ProtoReader::<Post>::new();
    let msg = reader.read(&mut sock).unwrap();
    msg
}
