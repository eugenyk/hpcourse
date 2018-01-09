package my.lockfree;

import java.util.List;

public class Tester {

    public static void main(String[] args) {

        LockFreeSetImpl<Integer> testSet = new LockFreeSetImpl<>();

        int count = 100;

        // add 1,2 ... , count
        new Thread(() -> {
            for (int i=0; i<count; i++) {testSet.add(i);}
        }).start();


        // rm 2,4,6, ... , count
        new Thread(() -> {
            for (int i=0; i<count; i+=2) {testSet.remove(i);}
        }).start();


        // rm count, count-3, ... , 0
        new Thread(() -> {
            for (int i=count; i>0; i-=3) {testSet.remove(i);}
        }).start();

        // add count, count - 5 , ... , 0
        new Thread(() -> {
            for (int i=count; i>0; i-=5) {testSet.add(i);}
        }).start();

        try { Thread.sleep(count ); } catch (Exception e) { }

        for (Integer n : testSet.getAll()) {
            System.out.println(n);
        }
        System.out.println("Test is finished");
    }
}