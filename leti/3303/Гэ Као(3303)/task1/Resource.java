package com.demo.ProducerConsumer;

/**
 * Rescource
 */
public class Resource {

    /**
     * Rescource number
     */
    private int number = 0;
    /**
     * Rescource maker
     */
    private boolean flag = false;

    /**
     * produce Recource
     */
    public synchronized void create() {
        while (flag) {

            try {
                wait();//thread waiting
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        number++;
        System.out.println(Thread.currentThread().getName() + "producer--------------" + number);
        flag = true;
        notifyAll();

    }

    /**
     * consume resource
     */
    public synchronized void destroy() {
        while (!flag) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

        }

        System.out.println(Thread.currentThread().getName() + "consumer*****" + number);
        flag = false;
        notifyAll();

    }

}
