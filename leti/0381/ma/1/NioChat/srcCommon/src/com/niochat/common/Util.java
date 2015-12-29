package com.niochat.common;

import sun.nio.ch.DirectBuffer;

import java.lang.ref.SoftReference;
import java.nio.ByteBuffer;
import java.util.concurrent.CopyOnWriteArraySet;

class Util {
    // Per-thread soft cache a temporary direct buffer. Only one need at any one time, the buffer has the capacity of the largest one need recently.
    private static final ThreadLocal<SoftReference<ByteBuffer>> softBufferPool = new ThreadLocal<SoftReference<ByteBuffer>>();

    static ByteBuffer getTemporaryDirectBuffer(int size) {
        // Grab a buffer if available
        SoftReference<ByteBuffer> ref = softBufferPool.get();
        ByteBuffer buf = null;
        if (ref != null && (buf = ref.get()) != null && buf.capacity() >= size) {
            buf.rewind();
            buf.limit(size);
            return buf;
        }
        if (buf != null)
            // release memory
            ((DirectBuffer) buf).cleaner().clean();

        // create a new one
        ByteBuffer buffer = ByteBuffer.allocateDirect(size);
        // replace the smallest one.
        softBufferPool.set(new SoftReference<ByteBuffer>(buffer));

        return buffer;
    }
}