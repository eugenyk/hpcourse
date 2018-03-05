/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package lab2_lockfree;


public class Lab2_LockFree {


    public static void main(String[] args) {
        
        final LockFreeSetImpl<Integer> mySet = new LockFreeSetImpl<>();
         
        Thread t1 = new Thread(new Runnable() {
            @Override
            public void run() {
                for (int i = 0; i < 5; i++) {
                     System.out.println("isEmpty " + mySet.isEmpty());
                     System.out.println("add 1 " + mySet.add(1));
                     System.out.println("add 3 " + mySet.add(3));
                     System.out.println("add 2 " + mySet.add(2));
                     System.out.println("remove 4 " + mySet.remove(4));
                     System.out.println("contains 2 " + mySet.contains(2));
                     System.out.println("isEmpty " + mySet.isEmpty());
                     System.out.println("remove 1 " + mySet.remove(1));
                     System.out.println("add 7 " + mySet.add(7));
                     System.out.println("isEmpty " + mySet.isEmpty());
                     System.out.println("contains 1 " + mySet.contains(1));
                     System.out.println("remove 3 " + mySet.remove(3));
                     System.out.println("remove 2 " + mySet.remove(2));
                     System.out.println("isEmpty " + mySet.isEmpty());  
                }        
            }
        });
        
         
         Thread t2 = new Thread(new Runnable() {
            @Override
            public void run() {
                for (int i = 0; i < 5; i++) {
                     System.out.println("isEmpty " + mySet.isEmpty());
                     System.out.println("add 1 " + mySet.add(1));
                     System.out.println("add 8 " + mySet.add(8));
                     System.out.println("add 4 " + mySet.add(4));
                     System.out.println("remove 4 " + mySet.remove(4));
                     System.out.println("contains 2 " + mySet.contains(2));
                     System.out.println("isEmpty " + mySet.isEmpty());
                     System.out.println("remove 1 " +mySet.remove(1));
                     System.out.println("add 7 " + mySet.add(7));
                     System.out.println("isEmpty " + mySet.isEmpty());
                     System.out.println("contains 2 " + mySet.contains(2));
                     System.out.println("remove 3 " + mySet.remove(3));
                     System.out.println("remove 8 " + mySet.remove(8));
                     System.out.println("isEmpty " +mySet.isEmpty());   
                }       
            }
        });
         
        
          Thread t3 = new Thread(new Runnable() {
            @Override
            public void run() {
                for (int i = 0; i < 10; i++) {
                 
                     System.out.println("add 16 " + mySet.add(16));
                     System.out.println("add 87 " + mySet.add(87));
                     System.out.println("remove 4 " + mySet.remove(4));
                     System.out.println("contains 22 " + mySet.contains(22));
                     System.out.println("isEmpty " + mySet.isEmpty());
                     System.out.println("remove 16 " +mySet.remove(16));
                     System.out.println("add 7 " + mySet.add(7));
                     System.out.println("contains 87 " + mySet.contains(87));
                     System.out.println("remove 3 " + mySet.remove(3));
                     System.out.println("remove 87 " + mySet.remove(87));
                     System.out.println("isEmpty " +mySet.isEmpty());   
                }       
            }
        });
         
         
        t1.start();
        t2.start();
        t3.start();
        
        try {
            t1.join();
            t2.join();
            t3.join();
        } catch (InterruptedException e) {}
          

   
    }
    
}
