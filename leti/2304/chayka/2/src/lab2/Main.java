package lab2;

import javax.swing.plaf.synth.SynthSpinnerUI;

public class Main {

	public static void main(String[] args) {
		LockFreeSetImpl<Integer> set = new LockFreeSetImpl<>();
		System.out.println(set);
		System.out.println("add 10 > " + set.add(10));
		System.out.println("add 10 > " + set.add(20));
		System.out.println("add 10 > " + set.add(30));
		System.out.println("add 10 > " + set.add(40));
		System.out.println("add 10 > " + set.add(25));
		System.out.println("add 10 > " + set.add(5));
		System.out.println(set);
		System.out.println("c: "+ set.contains(0));
		//System.out.println("remove: "+ set.remove(5));
		//System.out.println("remove: "+ set.remove(10));
		int i=0;
		while(!set.isEmpty()) {
			System.out.println("remove [" + i +"]: "+ set.remove(i));
			i++;
		}
		System.out.println(set);
		
		
		
		//System.out.println("c: "+ set.contains(123));
		//System.out.println(">"+set.isEmpty());
		//set.add(123);
		//System.out.println("c: "+ set.contains(123));
		//System.out.println(">"+set.isEmpty());
		//set.head.next.getReference().next.set(set.head.next.getReference().next.getReference(), true);
		//System.out.println("c: "+ set.contains(123));
		//System.out.println(">"+set.isEmpty());
		//System.out.println("find: " + set.findPosition(1110).cur);
		//System.out.println("add 10 > " + set.add(10));
		//System.out.println("add 20 > " + set.add(20));
		//System.out.println("test 20 > " + set.contains(20));
		//System.out.println("test 100 > " + set.contains(100));
		//System.out.println("test 10 > " + set.contains(10));

	}

}
