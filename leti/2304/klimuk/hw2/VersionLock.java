/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package hw2;
import java.util.concurrent.atomic.AtomicLong;
/**
 *
 * @author Kiril
 */
public class VersionLock {
    private AtomicLong lockValue = new AtomicLong(0);
    
    public long getVersion(){
        return lockValue.get() & 0xffffffffL; 
    }
    
    public boolean tryLockAtVersion(long ver){
        return lockValue.compareAndSet(ver, ver+1);
    }
    
    public void lockAtCurrentVersion(){
        boolean success = false;
        while (!success){
            long ver = getVersion();
            success = this.tryLockAtVersion(ver);
        }
    }
    
    public void unlockAndIncrementVersion(){
        long val = lockValue.get();
        lockValue.compareAndSet(val, val+1);
    }
}
