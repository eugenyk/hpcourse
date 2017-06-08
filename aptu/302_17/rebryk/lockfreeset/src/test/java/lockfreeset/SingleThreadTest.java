package lockfreeset;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertTrue;

public class SingleThreadTest {
    private static final int MAX_NUM = 10000;

    @Test
    public void emptySetTest() {
        final Set<Integer> set = new Set<>();
        assertTrue(set.isEmpty());
    }

    @Test
    public void fullTest() {
        final Set<Integer> set = new Set<>();

        final List<Integer> data = new ArrayList<>();
        for (int i = 1; i <= MAX_NUM; ++i) {
            data.add(i);
        }

        Collections.shuffle(data);

        data.stream().forEach(x -> assertTrue(set.add(x)));
        assertFalse(set.isEmpty());

        data.stream().forEach(x -> assertFalse(set.add(x)));
        assertFalse(set.isEmpty());

        data.stream().forEach(x -> assertTrue(set.contains(x)));
        assertFalse(set.isEmpty());

        data.stream().forEach(x -> assertTrue(set.remove(x)));
        assertTrue(set.isEmpty());

        data.stream().forEach(x -> assertFalse(set.remove(x)));
        assertTrue(set.isEmpty());
    }
}
