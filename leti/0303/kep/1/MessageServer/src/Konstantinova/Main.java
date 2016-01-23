package Konstantinova;

import Konstantinova.Server.Daemon;
import Konstantinova.Server.Server;

import java.nio.channels.AsynchronousChannelGroup;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;

/**
 * Created by User on 13.12.2015.
 */
public class Main {

    public static void main (String args[]){

        System.out.println("Server has been started " + args[0]);

        int threadNum = Integer.parseInt(args[0]);
        try {
            AsynchronousChannelGroup channelGroup = AsynchronousChannelGroup.withFixedThreadPool(threadNum, Executors.defaultThreadFactory());
            Daemon d = new Daemon();
            Thread t = new Thread(d);
            t.setDaemon(true);
            t.start();
            Server s = new Server(channelGroup, d);
            s.start();
        } catch (Exception e){e.printStackTrace();}

    }
}
