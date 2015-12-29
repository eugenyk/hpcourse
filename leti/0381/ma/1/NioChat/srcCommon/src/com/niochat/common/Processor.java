package com.niochat.common;

import java.nio.ByteBuffer;

public interface Processor {
    void process(ByteBuffer bf, Client client);
}
