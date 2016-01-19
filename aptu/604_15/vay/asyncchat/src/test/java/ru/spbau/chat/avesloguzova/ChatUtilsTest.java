package ru.spbau.chat.avesloguzova;

import org.junit.Test;

import java.nio.ByteBuffer;
import java.util.List;

import static org.junit.Assert.*;

public class ChatUtilsTest {

    @Test
    public void testRunCommand() throws Exception {
        assertEquals(ChatUtils.runCommand("echo OK").get(0), "OK");
    }
//@Test
//    public void testRunCatCommand() throws Exception {
//    List<String> strings = ChatUtils.runCommand("cat /Volumes/Home/av/Documents/war_and_peace.txt");
//    assertFalse(strings.isEmpty());
//}

    @Test
    public void testConcatBuffers() throws Exception {
        byte[] bytes = ByteBuffer.allocate(4).putInt(42).array();
        ByteBuffer testBuffer1 = ByteBuffer.allocate(2);
        testBuffer1.put(bytes, 0, 2);
        ByteBuffer testBuffer2 = ByteBuffer.allocate(2);
        testBuffer2.put(bytes, 2, 2);

        ByteBuffer byteBuffer = ChatUtils.concatBuffers(testBuffer1, testBuffer2);
        byteBuffer.flip();
        assertEquals(42, byteBuffer.getInt());
    }
}