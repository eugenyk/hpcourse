import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.stream.IntStream;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

@SuppressWarnings("unchecked")
class ContainerOperationsTests {

    private LockFreeContainer lockFreeContainer;

    @BeforeEach
    void prepare() {
        lockFreeContainer = new LockFreeContainer();
    }

    @Test
    void addAndContainsTest() {
        IntStream.range(0, 5).forEach(i -> lockFreeContainer.add(i));
        IntStream.range(0, 5).forEach(i -> assertTrue(lockFreeContainer.contains(i)));
    }

    @Test
    void removeTest() {
        IntStream.range(0, 5).forEach(i -> lockFreeContainer.add(i));
        lockFreeContainer.remove(0);
        lockFreeContainer.remove(2);
        lockFreeContainer.remove(4);
        assertFalse(lockFreeContainer.contains(0));
        assertTrue(lockFreeContainer.contains(1));
        assertFalse(lockFreeContainer.contains(2));
        assertTrue(lockFreeContainer.contains(3));
        assertFalse(lockFreeContainer.contains(4));
    }

    @Test
    void isEmptyTest() {
        assertTrue(lockFreeContainer.isEmpty());
        IntStream.range(0, 5).forEach(i -> lockFreeContainer.add(i));
        assertFalse(lockFreeContainer.isEmpty());
        IntStream.range(0, 5).forEach(i -> assertTrue(lockFreeContainer.remove(i)));
        assertTrue(lockFreeContainer.isEmpty());
    }
}
