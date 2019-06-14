import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

@Param(name = "value", gen = IntGen.class)
@StressCTest
public class LinCheckTest {

    private final LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public boolean contains(@Param(name = "value") final Integer value) {
        return set.contains(value);
    }

    @Operation
    public boolean add(@Param(name = "value") final Integer value) {
        return set.add(value);
    }

    @Operation
    public boolean remove(@Param(name = "value") final Integer value) {
        return set.remove(value);
    }

    @Operation
    public List<Integer> iterator() {
        List<Integer> list = new ArrayList<>();
        Iterator<Integer> it = set.iterator();
        while (it.hasNext()) {
            list.add(it.next());
        }
        return list;
    }

    @Test
    public void test() {
        LinChecker.check(LinCheckTest.class);
    }
}
