package com.demo.ProducerConsumer;

public class ProducerConsumerTest {
    public static void main(String[] args) {
        Resource resource = new Resource();
        new Thread(new Producer(resource)).start();//producer thread
        new Thread(new Producer(resource)).start();//producer thread
        new Thread(new Consumer(resource)).start();//consumer thread
        new Thread(new Consumer(resource)).start();//consumer thread

    }
}
