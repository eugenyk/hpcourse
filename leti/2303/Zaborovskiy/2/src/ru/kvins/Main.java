package ru.kvins;

public class Main {

    public static void main(String[] args) {
        //test1
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(1);
        set.add(2);
        set.add(3);
        assert !set.isEmpty();
        assert set.contains(1);
        assert set.contains(2);
        assert set.contains(3);
        set.remove(3);
        set.remove(2);
        set.remove(1);
        assert set.isEmpty();

        //test2 проверка на идентичные значения
        set.add(2);
        assert !set.add(2);

        //test3 добавляем в наш сет из двух тредов, третьим потоком удаляем

        final LockFreeSet<Integer> eFSet = new LockFreeSetImpl<>();
        boolean[] finish = new boolean[3];
        final Thread thread1 = new Thread(() -> {
            int i = 0;
            while (i < 100) {
                eFSet.add(i);
                i++;
            }
            finish[0] = true;
        });
        final Thread thread2 = new Thread(() -> {
            int i = 0;
            while (i < 100) {
                eFSet.add(i);
                i++;
            }
            finish[1] = true;
        });
        final Thread thread3 = new Thread(() -> {
            int i = 0;
            while (i < 100) {
                eFSet.remove(i);
                i++;
            }
            finish[2] = true;
        });

        thread1.start();
        thread2.start();
        thread3.start();

        try {
            while (!finish[0]) {
                thread1.join();
            }
            while (!finish[1]) {
                thread2.join();
            }
            while (!finish[2]) {
                thread3.join();
            }
        } catch (InterruptedException ignored) {

        }

        assert !eFSet.isEmpty();
        
        System.out.println("Passed");
    }
}
