import org.junit.Test;

/**
 * Created by nadya-bu on 28/05/2017.
 */

public class LockFreeSetImplTests {
    @Test
    public void oneFlowSimpleTest() {
        Integer[] data = {1, 2, 3, 4, 5};
        LockFreeSetImpl<Integer> set = new LockFreeSetImpl<Integer>();
        assert (set.isEmpty());
        assert (set.add(data[0]));
        assert (!set.isEmpty());
        assert (set.contains(data[0]));
        assert (!set.contains(data[1]));
        assert (!set.remove(data[1]));
        assert (!set.add(data[0]));
        assert (set.remove(data[0]));

        assert (set.isEmpty());
        assert (set.add(data[0]));
        assert (set.add(data[1]));
        assert (set.contains(data[1]));
        assert (set.contains(data[0]));
        assert (!set.contains(data[2]));
        assert (set.remove(data[0]));
        assert (!set.contains(data[0]));
        assert (set.contains(data[1]));

        assert (set.add(data[0]));
        assert (set.add(data[2]));
        assert (set.add(data[3]));
        assert (set.add(data[4]));
        assert (!set.add(data[1]));
        assert (set.remove(data[1]));
        assert (!set.contains(data[1]));
        assert (set.remove(data[3]));
        assert (!set.contains(data[1]));
        assert (set.add(data[1]));
        assert (!set.contains(data[3]));
    }

}
