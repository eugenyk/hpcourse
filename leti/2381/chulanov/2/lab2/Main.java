package lab2;

public class Main {

    public static void main(String[] args) {

        LockFreeSetImpl<Integer> testSet = new LockFreeSetImpl<>();

        int count = 100;
        //steps
        int a = 3;
        int b = 4;
        int c = 5;
        int d = 6;
        
        // add from 0
        new Thread(() -> {
            for (int i=0; i<count; i+=a) {testSet.add(i);}
        }).start();

        // remove from 0
        new Thread(() -> {
            for (int i=0; i<count; i+=b) {testSet.remove(i);}
        }).start();

        // remove from end
        new Thread(() -> {
            for (int i=count; i>=0; i-=c) {testSet.remove(i);}
        }).start();

        // add from end
        new Thread(() -> {
            for (int i=count; i>=0; i-=d) {testSet.add(i);}
        }).start();
        
        try { Thread.sleep(1000 ); } catch (Exception e) { }
        
        
        testSet.printResult();
      
    }
}
