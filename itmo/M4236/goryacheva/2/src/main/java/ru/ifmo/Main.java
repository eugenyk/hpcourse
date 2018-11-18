package ru.ifmo;

import ru.ifmo.priorityqueue.LockFreePriorityQueue;

import java.util.Arrays;
import java.util.List;
import java.util.PriorityQueue;

public class Main {

    public static void main(String[] args) {
        LockFreePriorityQueue<Integer> pq = new LockFreePriorityQueue<>();
        PriorityQueue<Integer> pq_java = new PriorityQueue<>();
        List<Integer> elements = Arrays.asList(43, 3, 3, 17, 22, 1, 128, -15, 17, 89, 90, -5);
        for (Integer i : elements) {
            pq.offer(i);
            pq_java.offer(i);
        }
        for (int i = 0; i < elements.size(); i++) {
            assert (pq_java.poll().equals(pq.poll()));
        }
    }
}
