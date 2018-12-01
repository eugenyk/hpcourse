package com.demo.ProducerConsumer;

/**
 * producer
 */
public class Producer implements Runnable {

    public Resource resource;
    public Producer(Resource resource){
        this.resource = resource;
    }

    @Override
    public void run() {
        while (true){
            try {
                Thread.sleep(10);

            }catch (InterruptedException e){
                e.printStackTrace();
            }
            resource.create();
        }
    }
}
