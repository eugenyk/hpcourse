package ru.eltech.diogenova.lockfree;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import org.openjdk.jcstress.infra.runners.TestConfig;
import org.openjdk.jcstress.infra.collectors.TestResultCollector;
import org.openjdk.jcstress.infra.runners.Runner;
import org.openjdk.jcstress.infra.runners.StateHolder;
import org.openjdk.jcstress.util.Counter;
import org.openjdk.jcstress.vm.WhiteBoxSupport;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Callable;
import java.util.Collections;
import java.util.List;
import ru.eltech.diogenova.lockfree.JCStressCorrectTest;
import org.openjdk.jcstress.infra.results.III_Result_jcstress;

public class JCStressCorrectTest_jcstress extends Runner<III_Result_jcstress> {

    volatile StateHolder<JCStressCorrectTest, III_Result_jcstress> version;

    public JCStressCorrectTest_jcstress(TestConfig config, TestResultCollector collector, ExecutorService pool) {
        super(config, collector, pool, "ru.eltech.diogenova.lockfree.JCStressCorrectTest");
    }

    @Override
    public Counter<III_Result_jcstress> sanityCheck() throws Throwable {
        Counter<III_Result_jcstress> counter = new Counter<>();
        sanityCheck_API(counter);
        sanityCheck_Footprints(counter);
        return counter;
    }

    private void sanityCheck_API(Counter<III_Result_jcstress> counter) throws Throwable {
        final JCStressCorrectTest s = new JCStressCorrectTest();
        final III_Result_jcstress r = new III_Result_jcstress();
        Collection<Future<?>> res = new ArrayList<>();
        res.add(pool.submit(() -> s.actor1()));
        res.add(pool.submit(() -> s.actor2()));
        for (Future<?> f : res) {
            try {
                f.get();
            } catch (ExecutionException e) {
                throw e.getCause();
            }
        }
        try {
            pool.submit(() ->s.arbiter(r)).get();
        } catch (ExecutionException e) {
            throw e.getCause();
        }
        counter.record(r);
    }

    private void sanityCheck_Footprints(Counter<III_Result_jcstress> counter) throws Throwable {
        config.adjustStrides(size -> {
            version = new StateHolder<>(new JCStressCorrectTest[size], new III_Result_jcstress[size], 2, config.spinLoopStyle);
            for (int c = 0; c < size; c++) {
                III_Result_jcstress r = new III_Result_jcstress();
                JCStressCorrectTest s = new JCStressCorrectTest();
                version.rs[c] = r;
                version.ss[c] = s;
                s.actor1();
                s.actor2();
                s.arbiter(r);
                counter.record(r);
            }
        });
    }

    @Override
    public Counter<III_Result_jcstress> internalRun() {
        version = new StateHolder<>(new JCStressCorrectTest[0], new III_Result_jcstress[0], 2, config.spinLoopStyle);

        control.isStopped = false;

        List<Callable<Counter<III_Result_jcstress>>> tasks = new ArrayList<>();
        tasks.add(this::actor1);
        tasks.add(this::actor2);
        Collections.shuffle(tasks);

        Collection<Future<Counter<III_Result_jcstress>>> results = new ArrayList<>();
        for (Callable<Counter<III_Result_jcstress>> task : tasks) {
            results.add(pool.submit(task));
        }

        try {
            TimeUnit.MILLISECONDS.sleep(config.time);
        } catch (InterruptedException e) {
        }

        control.isStopped = true;

        waitFor(results);

        Counter<III_Result_jcstress> counter = new Counter<>();
        for (Future<Counter<III_Result_jcstress>> f : results) {
            try {
                counter.merge(f.get());
            } catch (Throwable e) {
                throw new IllegalStateException(e);
            }
        }
        return counter;
    }

    public final void jcstress_consume(StateHolder<JCStressCorrectTest, III_Result_jcstress> holder, Counter<III_Result_jcstress> cnt, int a, int actors) {
        JCStressCorrectTest[] ss = holder.ss;
        III_Result_jcstress[] rs = holder.rs;
        int len = ss.length;
        int left = a * len / actors;
        int right = (a + 1) * len / actors;
        for (int c = left; c < right; c++) {
            III_Result_jcstress r = rs[c];
            JCStressCorrectTest s = ss[c];
            s.arbiter(r);
            ss[c] = new JCStressCorrectTest();
            cnt.record(r);
            r.r1 = 0;
            r.r2 = 0;
            r.r3 = 0;
        }
    }

    public final void jcstress_updateHolder(StateHolder<JCStressCorrectTest, III_Result_jcstress> holder) {
        if (!holder.tryStartUpdate()) return;
        JCStressCorrectTest[] ss = holder.ss;
        III_Result_jcstress[] rs = holder.rs;
        int len = ss.length;

        int newLen = holder.updateStride ? Math.max(config.minStride, Math.min(len * 2, config.maxStride)) : len;

        JCStressCorrectTest[] newS = ss;
        III_Result_jcstress[] newR = rs;
        if (newLen > len) {
            newS = Arrays.copyOf(ss, newLen);
            newR = Arrays.copyOf(rs, newLen);
            for (int c = len; c < newLen; c++) {
                newR[c] = new III_Result_jcstress();
                newS[c] = new JCStressCorrectTest();
            }
         }

        version = new StateHolder<>(control.isStopped, newS, newR, 2, config.spinLoopStyle);
        holder.finishUpdate();
   }

    public final Counter<III_Result_jcstress> actor1() {

        Counter<III_Result_jcstress> counter = new Counter<>();
        while (true) {
            StateHolder<JCStressCorrectTest,III_Result_jcstress> holder = version;
            if (holder.stopped) {
                return counter;
            }

            JCStressCorrectTest[] ss = holder.ss;
            III_Result_jcstress[] rs = holder.rs;
            int size = ss.length;

            holder.preRun();

            for (int c = 0; c < size; c++) {
                JCStressCorrectTest s = ss[c];
                s.actor1();
            }

            holder.postRun();

            jcstress_consume(holder, counter, 0, 2);
            jcstress_updateHolder(holder);

            holder.postUpdate();
        }
    }

    public final Counter<III_Result_jcstress> actor2() {

        Counter<III_Result_jcstress> counter = new Counter<>();
        while (true) {
            StateHolder<JCStressCorrectTest,III_Result_jcstress> holder = version;
            if (holder.stopped) {
                return counter;
            }

            JCStressCorrectTest[] ss = holder.ss;
            III_Result_jcstress[] rs = holder.rs;
            int size = ss.length;

            holder.preRun();

            for (int c = 0; c < size; c++) {
                JCStressCorrectTest s = ss[c];
                s.actor2();
            }

            holder.postRun();

            jcstress_consume(holder, counter, 1, 2);
            jcstress_updateHolder(holder);

            holder.postUpdate();
        }
    }

}
