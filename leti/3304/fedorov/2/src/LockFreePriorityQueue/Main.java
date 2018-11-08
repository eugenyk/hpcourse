package LockFreePriorityQueue;
import java.util.Random;
public class Main 
{
    private static void TestLockFreePriorityQueue(LockFreePriorityQueue<Integer> queue, int number)
    {
        System.out.println("Thread "+String.valueOf(number)+": "+queue.isEmpty());
        queue.insert(1);
        System.out.println("Thread "+String.valueOf(number)+": insert 1");
        System.out.println("Thread "+String.valueOf(number)+": "+queue.isEmpty());
        queue.insert(2);
        System.out.println("Thread "+String.valueOf(number)+": insert 2");
        queue.insert(1);
        System.out.println("Thread "+String.valueOf(number)+": insert 1");
        queue.insert(5);
        final Random random = new Random();
        try 
        {
            Thread.sleep(random.nextInt(1000));
        } 
        catch(InterruptedException e)
        {
            throw new Error();
        }
        System.out.println("Thread "+String.valueOf(number)+": insert 5");
        queue.insert(3);
        System.out.println("Thread "+String.valueOf(number)+": insert 3");
        queue.insert(2);
        System.out.println("Thread "+String.valueOf(number)+": insert 2");
        queue.insert(5);
        System.out.println("Thread "+String.valueOf(number)+": insert 5");
        queue.insert(3);
        System.out.println("Thread "+String.valueOf(number)+": insert 3");
        queue.insert(2);
        System.out.println("Thread "+String.valueOf(number)+": insert 2");
        queue.insert(1);
        System.out.println("Thread "+String.valueOf(number)+": insert 1");
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": "+queue.isEmpty());
        System.out.println("Thread "+String.valueOf(number)+": extract "+queue.extractMinimum());
        System.out.println("Thread "+String.valueOf(number)+": "+queue.isEmpty());
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) 
    {
        final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue();
        for(int i=0; i<10; i++)
        {
            final int number=i+1;
            new Thread(() -> TestLockFreePriorityQueue(queue, number)).start();
        }
    }   
}