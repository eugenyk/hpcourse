package com.demo.ProducerConsumer;

/**
 * consumer
 */
public class Consumer implements Runnable {
    private Resource resource;
    public Consumer(Resource resource){
        this.resource = resource;
    }

    @Override
    public void run() {
        while (true){
            try{
                Thread.sleep(10);
            }catch (InterruptedException e){
                e.printStackTrace();
            }
            resource.destroy();
        }
    }
}
