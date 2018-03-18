package LockFreeSet;

/**
 * Hello world!
 *
 */
public class App 
{
    public static void main( String[] args )
    {
        LockFreeSetContainer<Integer> container = new LockFreeSetContainer<Integer>(0);
        container.add(1);
        container.add(2);
        container.add(2);

        System.out.println(container.isEmpty());
    }
}
