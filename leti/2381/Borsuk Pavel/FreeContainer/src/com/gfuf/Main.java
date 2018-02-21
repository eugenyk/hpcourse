package com.gfuf;

public class Main {
	public static void main(String args[])
	{
		LinkedLockFreeSet<Integer> set = new LinkedLockFreeSet<Integer>();

		assert set.add(1);
		assert set.add(2);
		assert set.add(3);
		assert set.contains(1);
		assert set.remove(1);
		assert !set.contains(1);
		assert set.remove(2);
		assert !set.remove(1);
		assert !set.add(3);
		assert set.remove(3);
		assert set.isEmpty();
		
		TestThreadAdd[] runnablesAdd = new TestThreadAdd[100];
		Thread[] threads = new Thread[100];
		
		/*
		 * TEST ADD
		 */
		for(int i = 0; i < 100; i++)
		{
			runnablesAdd[i] = new TestThreadAdd(set, i);
			threads[i] = new Thread(runnablesAdd[i]);
		}
		
		for(int i = 0; i < 100; i++)
		{
			threads[i].start();
		}
		
		for(int i = 0; i < 100; i++)
		{
			while(!runnablesAdd[i].finish)
			{
				try {
					threads[i].join();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
		
		/*
		 * TEST REMOVE
		 */
		
		TestThreadRemove[] runnablesRemove = new TestThreadRemove[100];
	
		for(int i = 0; i < 100; i++)
		{
			runnablesRemove[i] = new TestThreadRemove(set, i);
			threads[i] = new Thread(runnablesRemove[i]);
		}
		
		for(int i = 0; i < 100; i++)
		{
			threads[i].start();
		}
		
		for(int i = 0; i < 100; i++)
		{
			while(!runnablesRemove[i].finish)
			{
				try {
					threads[i].join();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
		
		assert set.isEmpty();
		
		
		
		
	        
		
		
	}
}
