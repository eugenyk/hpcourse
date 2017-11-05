package com.gfuf;

public class TestThreadAdd implements Runnable{

	LinkedLockFreeSet<Integer> set;
	int value;
	public boolean finish;
	
	public TestThreadAdd(LinkedLockFreeSet<Integer> set, int value) 
	{
	      this.value = value;
	      this.set = set;
	}

	@Override
	public void run() {
		finish = false;
		assert set.add(value);
		assert set.contains(value);
        finish = true;    
	}

}
