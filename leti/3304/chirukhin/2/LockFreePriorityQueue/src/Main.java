import main.priorityqueue.LockFreePriorityQueue;

public class Main {

    public static void main(String[] args) {
        LockFreePriorityQueue<Integer> que = new LockFreePriorityQueue<>();
        que.add(1);
        que.add(10);
        que.add(2);
        que.add(20);
        que.add(3);
        que.add(30);

        System.out.println(que.poll());
        System.out.println(que.poll());
        System.out.println(que.poll());
        System.out.println(que.poll());
        System.out.println(que.poll());
        System.out.println(que.poll());
    }
}
