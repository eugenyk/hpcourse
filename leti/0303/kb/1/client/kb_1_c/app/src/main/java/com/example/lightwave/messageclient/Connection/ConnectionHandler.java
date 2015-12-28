package com.example.lightwave.messageclient.Connection;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import com.example.lightwave.messageclient.MainActivity;
import com.example.lightwave.messageclient.Views.ConnectionIndicator;
import com.example.lightwave.messageclient.proto.MessageBody;

import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ConnectionHandler extends Service {
    Intent connectedIntent;
    Intent newMessageIntent;
    Intent killIntent;
    SocketChannel channel;
    ExecutorService executor;
    boolean isStayAlive = true;

    public ConnectionHandler() {
        connectedIntent = new Intent(MainActivity.CONNECTEDACTION);
        newMessageIntent = new Intent(MainActivity.NEWMESSAGE);
        killIntent = new Intent(MainActivity.CONNECTIONKILL);
        executor = Executors.newFixedThreadPool(10);
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String action = intent.getAction();
        if(intent == null) {
            stopSelf(startId);
        }

        if(action.equals(MainActivity.CONNECTEDACTION)) {
            String address = intent.getStringExtra("address");
            String[] inet = address.split(":");
            Log.i("myinfo", "Address to connect " + address);
            if(inet.length < 2)
                return super.onStartCommand(intent, flags, startId);


            executor.execute(new ConnectionHolder(inet[0], Integer.parseInt(inet[1])));
        }

        if(action.equals(MainActivity.NEWMESSAGE)) {
            Log.i("myinfo", "New message");
            executor.execute(new Sender(intent.getStringExtra("Sender"), intent.getStringExtra("Text")));
        }

        if(action.equals(MainActivity.CONNECTIONKILL)) {
            stopSelf(startId);
        }



        if(action.equals(MainActivity.SPAM)) {
            Sender sender = new Sender("Spammer", "Spam-spam-spam!Spam-spam-spam!Spam-spam-spam!Spam-spam-spam!Spam-spam-spam!Spam-spam-spam!Spam-spam-spam!Spam-spam-spam!Spam-spam-spam!");
            sender.setRepeats(10);
            executor.execute(sender);
        }
        if(action.equals(MainActivity.BIGMESSAGE)) {
            String text = "";
            for(int i = 0; i< 5000; i++)
                text += "Spam";

            Sender sender = new Sender("Spammer", text);
            executor.execute(sender);
        }



        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i("myinfo", "Dropping connection in onDestroy");
        if(channel != null) {
            try {
                if (channel.isConnected()) {
                    channel.finishConnect();
                    channel.close();
                }
                Log.i("MessageClient", "Connection dropped");
            } catch (Exception e) {
            }
            channel = null;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        return new MyBinder();
    }

    public class MyBinder extends Binder {
        public ConnectionHandler getService() {
            return ConnectionHandler.this;
        }
    }

    public class Sender implements Runnable {

        String sender;
        String text;
        int repeats = 1;

        public void setRepeats(int repeats) {
            this.repeats = repeats;
        }

        public Sender(String sender, String text) {
            this.sender = sender;
            this.text = text;
        }

        @Override
        public void run() {
        if(repeats > 1) {
            for(int i =0; i < repeats; i++ ) {
                send();
                try {
                    Thread.currentThread().sleep(30);
                } catch (Exception e) {
                    Log.e("myinfo", "Wait error" + e.toString());
                }
            }
        } else {
            send();
        }

        }

        private void send() {
            Log.i("myinfo", "Writing");
            MessageBody.Message msg = MessageBody.Message.newBuilder().setSender(sender).setText(text).build();
            byte[] data = msg.toByteArray();
            ByteBuffer bf = ByteBuffer.allocate(data.length + 4);

            bf.putInt(data.length);
            bf.put(data);
            bf.position(0);
            try {
                channel.write(bf);
            }
            catch (Exception e) {
                Log.e("myinfo", "Write error:" + e.toString());
            }
        }
    }

    public class ConnectionHolder implements Runnable {
        MessageBuffer messageBuffer = null;
        String ip;
        int port;

        public ConnectionHolder(String ip, int port) {
            this.ip = ip;
            this.port = port;
        }

        @Override
        public void run() {
            if(channel != null)
                return;
            establishConnection();
        }
        public void logArray(String msg, byte[] array) {
            String test = "";
            for(int i=0; i< array.length; i++)
                test += Byte.toString(array[i]) + ',';
            Log.i("myinfo", msg + test);
        }
        public void establishConnection() {
            Log.e("MessageClient", "Service thread " + Thread.currentThread().getId());
            try {
                channel = SocketChannel.open();

                channel.connect(new InetSocketAddress(ip, port));


                sendBroadcast(connectedIntent);
                waitForMessageForever();
            }
            catch (Exception e) {
                Log.e("MessageClient", "Server socket channel error " + e.toString());
            }
        }

        protected void waitForMessageForever() {
            ByteBuffer buffer = ByteBuffer.allocate(1500);
            while(isStayAlive) {
                int recievedLength = 0;
                try {
                    if(channel.isConnected())
                        recievedLength = channel.read(buffer);
                    else
                        Log.e("MessageClient", "Connection dead");
                    Log.i("MessageClient", "Message recieved");
                }
                catch (Exception e) {
                    Log.e("MessageClient", "Socket read exception: " + e.toString());
                    isStayAlive = false;
                }
                if(recievedLength > 0) {
                    MessageBody.Message message = null;
                    if (messageBuffer == null) {
                        message = tryReadNewMessage(buffer, recievedLength);
                    } else {
                        message = tryReadBufferiezedMessage(buffer, recievedLength);
                    }
                    if(message != null) {
                        notifyAboutMessage(message);
                    }
                }
                buffer = ByteBuffer.allocate(1500);
            }
            if(!isStayAlive && channel != null) {
                try {
                    if (channel.isConnected())
                        channel.finishConnect();
                    Log.i("MessageClient", "Connection dropped");
                } catch (Exception e) {  }
                channel = null;
                sendBroadcast(killIntent);
            }
        }

        protected MessageBody.Message tryReadNewMessage(ByteBuffer buf, int length) {
            MessageBody.Message message = null;
            buf.position(0);
            int maxLenght = buf.getInt();
            byte[] rawMessage = new byte[length - 4];
            buf.get(rawMessage);
            if( maxLenght <= length-4) {
                message = parseMessage(rawMessage);
            } else {
                messageBuffer = new MessageBuffer(rawMessage, maxLenght);
            }
            return message;
        }

        protected MessageBody.Message tryReadBufferiezedMessage(ByteBuffer buf, int length) {
            MessageBody.Message message = null;
            buf.position(0);
            byte[] rawPart = new byte[length - 4];
            if(messageBuffer.getLength() <= messageBuffer.getCurrentLength() + length ) {
                messageBuffer.append(rawPart);
                message = parseMessage(messageBuffer.getBuffer());
                messageBuffer = null;
            } else {
                messageBuffer.append(rawPart);
            }
            return message;
        }

        protected MessageBody.Message parseMessage(byte[] rawMessage) {
            MessageBody.Message message = null;
            try {
                message = MessageBody.Message.parseFrom(rawMessage);
            }
            catch (Exception e) {
                Log.e("MessageClient", "Protocol buffer parse error: " + e.toString());
            }
            Log.i("MessageClient", "Message parsed. Sender:" + message.getSender() + " Text: " + message.getText());
            return message;
        }

        protected void notifyAboutMessage(MessageBody.Message message) {
            newMessageIntent.putExtra("Sender", message.getSender());
            newMessageIntent.putExtra("Text", message.getText());
            sendBroadcast(newMessageIntent);
        }

    }
}
