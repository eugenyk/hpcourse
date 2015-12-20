use protobuf::{self, Message};

use mio::buf::{ByteBuf, Buf};

mod message;

use self::message::Message_Type;

pub use self::message::Message as Post;

impl Post {
    pub fn command(&self) -> Option<&String> {
        match self.get_field_type() {
            Message_Type::COMMAND => {
                Some(&self.get_text()[0])
            }
            Message_Type::MESSAGE => None
        }
    }

    pub fn from_result(result: String) -> Post {
        Post::from_text("".to_string(), vec![result])
    }

    pub fn from_text(author: String, text: Vec<String>) -> Post {
        let mut proto = Post::default();
        proto.set_author(author);
        proto.set_text(protobuf::RepeatedField::from_vec(text));
        proto.set_field_type(Message_Type::MESSAGE);
        proto
    }

    pub fn from_command(text: String) -> Post {
        let mut proto = Post::default();
        proto.set_text(protobuf::RepeatedField::from_vec(vec![text]));
        proto.set_field_type(Message_Type::COMMAND);
        proto
    }

    pub fn take(mut self) -> (String, Vec<String>) {
        (self.take_author(), self.take_text().into_vec())
    }

    pub fn to_bytes(&self) -> Vec<u8> {
        let size = self.compute_size();
        let mut buf = ByteBuf::mut_with_capacity((size + 5) as usize);
        self.write_length_delimited_to_writer(&mut buf).unwrap();
        buf.flip().bytes().iter().map(|&i| i).collect()
    }

}
