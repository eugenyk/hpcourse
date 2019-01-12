package ru.etu.osi.test;

import ru.etu.osi.queue.LockFreePriorityQueue;
import ru.etu.osi.queue.PriorityQueue;

import java.util.NoSuchElementException;

import org.junit.Test;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;

@Param(name = "val", gen = IntGen.class)
@StressCTest
public class LinCheckTest {
	private PriorityQueue<Integer> q = new LockFreePriorityQueue<>();

 	@Operation
	public Integer poll() {
		return q.poll();
	}
	
	@Operation
	public Integer peek() {
		return q.peek();
	}

 	@Operation
	public boolean isEmpty() {
		return q.isEmpty();
	}
	/*
	@Operation
	public int size() {
		return q.size();
	}
	*/
	@Operation
	public boolean add(@Param(name = "val") Integer val) {
		return q.add(val);
	}

	@Operation(handleExceptionsAsResult = NoSuchElementException.class)
	public Integer remove()
	{
		return q.remove();
	}

	@Test
	public void runTest() {
		LinChecker.check(LinCheckTest.class);
	}
}
