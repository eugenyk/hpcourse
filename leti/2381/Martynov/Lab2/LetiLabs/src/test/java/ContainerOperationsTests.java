import org.junit.jupiter.api.Test;

import java.util.stream.IntStream;

import static org.junit.jupiter.api.Assertions.assertTrue;

@SuppressWarnings("unchecked")
class ContainerOperationsTests {

    private LockFreeContainer lockFreeContainer;

    @Test
    void addTest() {
        lockFreeContainer = new LockFreeContainer();
        IntStream.range(0, 5).forEach(i -> lockFreeContainer.add(i));
        assertTrue(lockFreeContainer.getSize() == 5);
        IntStream.range(0, 5).forEach(i -> assertTrue(lockFreeContainer.contains(i)));
    }

    @Test
    void removeTest() {
        lockFreeContainer = new LockFreeContainer();
        IntStream.range(0, 5).forEach(i -> lockFreeContainer.add(i));
        lockFreeContainer.remove(0);
        lockFreeContainer.remove(2);
        lockFreeContainer.remove(4);
        assertTrue(lockFreeContainer.getSize() == 3);
        assertTrue(lockFreeContainer.contains(1));
        assertTrue(lockFreeContainer.contains(3));
    }

    @Test
    void containsTest() {
        lockFreeContainer = new LockFreeContainer();
        IntStream.range(0, 5).forEach(i -> lockFreeContainer.add(i));
        assertTrue(lockFreeContainer.getSize() == 5);
        IntStream.range(0, 5).forEach(i -> assertTrue(lockFreeContainer.contains(i)));
    }
}
