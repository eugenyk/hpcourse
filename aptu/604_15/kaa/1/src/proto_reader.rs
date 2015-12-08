use std::marker::PhantomData;
use std::io::{self, Read};

use protobuf;
use protobuf::stream::WithCodedInputStream;


pub struct ProtoReader<M: protobuf::MessageStatic> {
    is_reading_length: bool,
    len_buffer: Vec<u8>,
    msg_buffer: Vec<u8>,
    msg_ptr: usize,
    m: PhantomData<M>,
}

impl<M: protobuf::MessageStatic> ProtoReader<M> {
    pub fn new() -> ProtoReader<M> {
        ProtoReader {
            is_reading_length: true,
            len_buffer: Vec::new(),
            msg_buffer: Vec::new(),
            msg_ptr: 0,
            m: PhantomData,
        }
    }

    pub fn read<R: Read>(&mut self, source: &mut R) -> io::Result<M> {
        let mut byte = [0u8; 1];
        loop {
            if self.is_reading_length {
                try!(source.read(&mut byte));
                let byte = byte[0];
                self.len_buffer.push(byte);

                if byte.leading_zeros() > 0 {
                    let msg_len = match self.len_buffer.with_coded_input_stream(|is| {
                        is.read_raw_varint32()
                    }) {
                        Ok(n) => n as usize,
                        Err(e) => return Err(io::Error::new(io::ErrorKind::InvalidData, e))
                    };
                    self.len_buffer.truncate(0);
                    self.is_reading_length = false;
                    self.msg_buffer.resize(msg_len, 0);
                    self.msg_ptr = 0;
                }
            } else {
                let n = try!(source.read(&mut self.msg_buffer[self.msg_ptr..]));
                self.msg_ptr += n;

                if self.msg_ptr == self.msg_buffer.len() {
                    self.is_reading_length = true;

                    return match protobuf::parse_from_bytes::<M>(&self.msg_buffer) {
                        Ok(m) => Ok(m),
                        Err(e) => Err(io::Error::new(io::ErrorKind::InvalidData, e))
                    }
                }
            }
        }
    }
}
