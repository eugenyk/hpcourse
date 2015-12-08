use std::thread;
use std::sync::mpsc;
use std::process::{Command, Output};

pub trait Sender: Send + 'static {
    fn send(&self, result: String);
}

pub struct Task<S: Sender> {
    pub cmd: String,
    pub reply_to: S,
}

pub fn start<S: Sender>() -> mpsc::Sender<Task<S>> {
    let (tx, rx) = mpsc::channel();
    thread::spawn(move || {
       loop {
           match rx.recv() {
               Err(e) => {error!("Failed to receive shell message: {:?}", e);},
               Ok(Task {ref cmd, ref reply_to} ) => {
                   <S as Sender>::send(reply_to, exec(cmd));
               },
           }
       }
    });
    return tx;
}

pub fn exec(cmd: &str) -> String {
    let args = cmd.split_whitespace().collect::<Vec<_>>();
    if args.len() == 0 {
        return "Bad command".to_string();
    }

    match Command::new(args[0]).args(&args[1..]).output() {
        Err(e) => format!("Command {} failed: {}", cmd, e).to_string(),
        Ok(Output {stdout, .. }) => String::from_utf8_lossy(&stdout).to_string()
    }
}
