use std::sync::mpsc;

use post::Post;
use shell::{self, Task};
use pb;

impl shell::Sender for pb::Sender<ChatUser> {
    fn send(&self, s: String) {
        self.send(s).unwrap_or_else(|_| error!("Failed to send command result"));
    }
}

#[derive(Clone)]
pub struct ChatUser {
    pub shell: mpsc::Sender<Task<pb::Sender<ChatUser>>>,
}

type User = pb::User<ChatUser>;

impl pb::ProtoHandler for ChatUser {
    type Proto = Post;
    type Message = String;

    fn recv(&mut self, user: &mut User, post: Post) {
        if let Some(cmd) =  post.command() {
            let task = Task {
                cmd: cmd.to_string(),
                reply_to: user.channel(),
            };

            self.shell.send(task)
                .unwrap_or_else(|e| error!("Failed to send command, {}", e));
        }
        user.broadcast(post)
    }

    fn notify(&mut self, user: &mut User, result: String) {
        let post = Post::from_result(result);
        user.echo(post);
    }
}


