package net.vasalf.hw.lockfree;

import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.*;
import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.*;

class LockFreeSetUnitTest {
    private static Stream<Arguments> provideSets() {
        return Stream.of(
                Arguments.of(new SynchronizedSet<Integer>()),
                Arguments.of(new LockFreeSet<Integer>())
        );
    }

    @ParameterizedTest
    @MethodSource("provideSets")
    public void testConstruction(MySet<Integer> set) {
        assertTrue(set.isEmpty());
    }

    @ParameterizedTest
    @MethodSource("provideSets")
    public void testAddOne(MySet<Integer> set) {
        set.add(179);
        assertTrue(set.contains(179));
        assertFalse(set.contains(0));
    }

    @ParameterizedTest
    @MethodSource("provideSets")
    public void testAddAndRemove(MySet<Integer> set) {
        set.add(179);
        assertTrue(set.contains(179));
        set.remove(179);
        assertFalse(set.contains(179));
        set.add(179);
        assertTrue(set.contains(179));
    }

    @ParameterizedTest
    @MethodSource("provideSets")
    public void testAddAllAndRemove(MySet<Integer> set) {
        for (int i = 179; i < 239; i++) {
            set.add(i);
        }
        for (int i = 179; i < 239; i++) {
            assertTrue(set.contains(i));
        }
        for (int i = 179; i < 239; i++) {
            set.remove(i);
        }
        for (int i = 179; i < 239; i++) {
            assertFalse(set.contains(i));
        }
    }

    @ParameterizedTest
    @MethodSource("provideSets")
    public void iterateThroughEmpty(MySet<Integer> set) {
        assertFalse(set.iterator().hasNext());
    }

    @ParameterizedTest
    @MethodSource("provideSets")
    public void iterateThroughSetOfInts(MySet<Integer> set) {
        for (int i = 179; i < 239; i++) {
            set.add(i);
        }
        Iterator<Integer> it = set.iterator();
        List<Integer> ints = new ArrayList<>();
        while (it.hasNext()) {
            ints.add(it.next());
        }
        Collections.sort(ints);
        int i = 179;
        for (int u : ints) {
            assertEquals(i, u);
            i++;
        }
        assertEquals(239, i);
    }

    @ParameterizedTest
    @MethodSource("provideSets")
    public void justRandom(MySet<Integer> tested) {
        Random random = new Random(179);
        List<Integer> correct = new ArrayList<>();
        for (int i = 0; i < 100000; i++) {
            int op = random.nextInt(3);
            if (op == 0) {
                int elem = random.nextInt(20);
                boolean result = !correct.contains(elem);
                if (result) {
                    correct.add(elem);
                }
                assertEquals(result, tested.add(elem));
            } else if (op == 1 && !correct.isEmpty()) {
                int elem = random.nextInt(20);
                boolean result = correct.contains(elem);
                if (result) {
                    correct.remove(correct.indexOf(elem));
                }
                assertEquals(result, tested.remove(elem));
            } else if (op == 2) {
                List<Integer> actual = new ArrayList<>();
                Iterator<Integer> iterator = tested.iterator();
                while (iterator.hasNext()) {
                    actual.add(iterator.next());
                }
                Collections.sort(correct);
                Collections.sort(actual);
                assertEquals(correct, actual);
            }
        }
    }
}