use mio::buf::ByteBuf;
use protobuf;


pub fn to_buf<P: protobuf::Message>(msg: &P) -> ByteBuf {
    let size = msg.compute_size();
    let mut buf = ByteBuf::mut_with_capacity((size + 5) as usize);
    msg.write_length_delimited_to_writer(&mut buf).unwrap();
    buf.flip()
}

