package ru.spbau.chat.avesloguzova;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class ChatUtils {
    public static List<String> runCommand(String command) {
        try {
            Process process = Runtime.getRuntime().exec(command);
            try (BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                Stream<String> lines = bufferedReader.lines();
                return lines.collect(Collectors.toList());
            }
        } catch (IOException e) {
            return Collections.singletonList(e.getMessage());
        }
    }

    public static ByteBuffer concatBuffers(ByteBuffer oldBuffer, ByteBuffer buffer) {
        oldBuffer.flip();
        buffer.flip();
        ByteBuffer newBuffer = ByteBuffer.allocate(oldBuffer.capacity() + buffer.capacity());
        newBuffer.put(oldBuffer)
                .put(buffer);
        return newBuffer;
    }
}
