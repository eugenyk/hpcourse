import java.util.ArrayList;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

class StressTestUtil {
    public static final int DO_ADD = 1;
    public static final int DO_REMOVE = 2;
    public static final int DO_CONTAINS = 4;
    public static final int DO_ALL = DO_ADD | DO_REMOVE | DO_CONTAINS;

    private ArrayList<Integer> operations;
    private boolean checkEmpty;

    StressTestUtil(int operations, boolean checkEmpty) {
        this.operations = new ArrayList<>();
        for (int i = 0; i < 3; i++)
            if ((operations & (1 << i)) != 0) {
                this.operations.add(1 << i);
            }
        this.checkEmpty = checkEmpty;
    }

    void stressTest(LockFreeSet<Integer> s, int minVal, int valsCount, int steps, int seed) {
        Random rnd = new Random(seed);
        Set<Integer> stupidS = new HashSet<>();

        for (int i = 0; i < steps; i++) {
            int op = operations.get(rnd.nextInt(operations.size()));
            int value = rnd.nextInt(valsCount) + minVal;
            if (op == DO_ADD) {
                assertEquals(stupidS.add(value), s.add(value));
            } else if (op == DO_REMOVE) {
                assertEquals(stupidS.remove(value), s.remove(value));
            } else if (op == DO_CONTAINS) {
                assertEquals(stupidS.contains(value), s.contains(value));
            } else {
                fail("Invalid operation generated");
            }
            if (checkEmpty) {
                assertEquals(stupidS.isEmpty(), s.isEmpty());
            }
        }
    }
}
