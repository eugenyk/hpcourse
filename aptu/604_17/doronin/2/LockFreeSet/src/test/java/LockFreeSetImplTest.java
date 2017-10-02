import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class LockFreeSetImplTest {

    private LockFreeSet<Integer> set;

    @Before
    public void setUp() {
        set = new LockFreeSetImpl<>();
    }

    @Test
    public void add() throws Exception {
        Assert.assertTrue(set.add(10));
        Assert.assertTrue(set.contains(10));
        Assert.assertTrue(set.add(20));
        Assert.assertTrue(set.contains(20));
        Assert.assertTrue(set.add(5));
        Assert.assertTrue(set.contains(5));
        Assert.assertFalse(set.add(5));
        Assert.assertFalse(set.add(10));
        Assert.assertFalse(set.add(20));
        Assert.assertTrue(set.contains(5));
        Assert.assertTrue(set.contains(10));
        Assert.assertTrue(set.contains(20));
    }

    @Test
    public void remove() throws Exception {
        add();
        Assert.assertTrue(set.add(40));
        Assert.assertTrue(set.contains(40));
        Assert.assertTrue(set.add(1));
        Assert.assertTrue(set.contains(1));
        Assert.assertTrue(set.remove(10));
        Assert.assertFalse(set.contains(10));
        Assert.assertTrue(set.remove(1));
        Assert.assertFalse(set.contains(1));
        Assert.assertTrue(set.remove(40));
        Assert.assertFalse(set.contains(40));
        Assert.assertTrue(set.remove(5));
        Assert.assertFalse(set.contains(5));
        Assert.assertTrue(set.remove(20));
        Assert.assertFalse(set.contains(20));
        Assert.assertFalse(set.remove(20));
    }

    @Test
    public void contains() throws Exception {
        Assert.assertFalse(set.contains(0));
    }

    @Test
    public void isEmpty() throws Exception {
        Assert.assertTrue(set.isEmpty());
        add();
        Assert.assertFalse(set.isEmpty());
        Assert.assertTrue(set.remove(5));
        Assert.assertTrue(set.remove(10));
        Assert.assertTrue(set.remove(20));
        Assert.assertTrue(set.isEmpty());
        remove();
        Assert.assertTrue(set.isEmpty());
    }

}