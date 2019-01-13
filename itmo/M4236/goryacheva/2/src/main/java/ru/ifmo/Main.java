package ru.ifmo;

import ru.ifmo.priorityqueue.LockFreePriorityQueue;

import java.util.Arrays;
import java.util.List;

public class Main {

    public static void main(String[] args) {
        LockFreePriorityQueue<Integer> pq = new LockFreePriorityQueue<>();
        List<Integer> elements = Arrays.asList(1327, 193,  19, 3, 3, 3, 0, -15);
        for (Integer i: elements) {
            pq.offer(i);
            System.out.println(pq.peek().equals(i));
        }
    }
}
