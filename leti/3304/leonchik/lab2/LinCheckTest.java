package com.leti.lab2;

import org.junit.Test;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LinCheckTest
{
	private LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue();

	@Operation
	public boolean offer(@Param(name = "key") int key)
	{
		return queue.offer(key);
	}

	@Operation
	public Integer poll()
	{
		return queue.poll();
	}

	@Operation
	public boolean isEmpty()
	{
		return queue.isEmpty();
	}

	@Operation
	public Integer peek()
	{
		return queue.peek();
	}

	@Operation
	public void clear()
	{
		queue.clear();
	}

	@Operation
	public boolean contains(@Param(name = "key") int key)
	{
		return queue.contains(key);
	}

	@Operation
	public int size()
	{
		return queue.size();
	}

	@Test
	public void runTest()
	{
		LinChecker.check(LinCheckTest.class);
	}

}
