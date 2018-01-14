/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package lb2;

/**
 *
 * @author andre
 */
public class Main {

    public static void main(String[] args) {

        LockFreeSetImpl<Integer> testSet = new LockFreeSetImpl<>();

        //test
        new Thread(() -> {
            for (int i=0; i<10000; i++) {testSet.add(i);}
        }).start();

        new Thread(() -> {
            for (int i=0; i<10000; i+=2) {testSet.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i=10000; i>0; i--) {testSet.contains(i);}
        }).start();

        new Thread(() -> {
            for (int i=10000; i>0; i-=2) {testSet.add(i);}
        }).start();
        
        new Thread(() -> {
            for (int i=0; i<1000; i++) {testSet.add(i);}
        }).start();

        new Thread(() -> {
            for (int i=0; i<1000; i+=2) {testSet.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i=1000; i>0; i--) {testSet.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i=1000; i>0; i-=2) {testSet.contains(i);}
        }).start();
         new Thread(() -> {
            for (int i=0; i<10000; i++) {testSet.add(i);}
        }).start();

        new Thread(() -> {
            for (int i=0; i<10000; i+=2) {testSet.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i=10000; i>0; i--) {testSet.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i=10000; i>0; i-=2) {testSet.add(i);}
        }).start();
        
        new Thread(() -> {
            for (int i=0; i<10000; i++) {testSet.add(i);}
        }).start();

        new Thread(() -> {
            for (int i=0; i<1000; i+=2) {testSet.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i=10000; i>0; i--) {testSet.contains(i);}
        }).start();

        new Thread(() -> {
            for (int i=1000; i>0; i-=2) {testSet.add(i);}
        }).start();


        try { Thread.sleep(1000); } catch (InterruptedException e) { }
        System.out.print(testSet.isEmpty());
    }
}  
