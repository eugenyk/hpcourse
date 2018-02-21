package com.gfuf;

public class TestThreadRemove implements Runnable {
	LinkedLockFreeSet<Integer> set;
	int value;
	public boolean finish;
	
	public TestThreadRemove(LinkedLockFreeSet<Integer> set, int value) 
	{
	      this.value = value;
	      this.set = set;
	}

	@Override
	public void run() {
		finish = false;
		assert set.remove(value);
		assert !set.contains(value);
        finish = true;    
	}
}
