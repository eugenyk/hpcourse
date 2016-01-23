package com.example.lightwave.messageclient;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.IBinder;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;

import com.example.lightwave.messageclient.Connection.ConnectionHandler;
import com.example.lightwave.messageclient.Views.ConnectionIndicator;

import java.net.InetSocketAddress;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.List;


public class MainActivity extends Activity implements View.OnClickListener {
    SocketChannel channel;
    ServiceConnection sConn;
    ConnectionHandler handler;
    LayoutInflater inflater;
    Button send;
    Button options;

    LinearLayout list;
    EditText sender;
    EditText message;
    TextView indicator;

    BroadcastReceiver br;
    boolean connected = false;

    public static final String CONNECTEDACTION = "lightwave.connection.connected";
    public static final String NEWMESSAGE = "lightwave.connection.message";
    public static final String CONNECTIONKILL = "lightwave.connection.kill";
    public static final String SPAM = "lightwave.connection.spam";
    public static final String BIGMESSAGE = "lightwave.connection.big";


    Intent intent;
    Intent killerIntent;
    Intent spamIntent;
    Intent bigIntent;
    IntentFilter broadcastIntents;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        inflater = getLayoutInflater();
        send = (Button) findViewById(R.id.send);
        options = (Button) findViewById(R.id.options);
        send.setOnClickListener(this);
        indicator = (TextView) findViewById(R.id.indicator);
        list = (LinearLayout) findViewById(R.id.listMessages);

        message = (EditText) findViewById(R.id.message);
        indicator.setOnClickListener(this);
        options.setOnClickListener(this);

        intent = new Intent(this, ConnectionHandler.class);
        killerIntent = new Intent(this, ConnectionHandler.class);
        killerIntent.setAction(CONNECTIONKILL);
        spamIntent = new Intent(this, ConnectionHandler.class);
        spamIntent.setAction(SPAM);
        bigIntent = new Intent(this, ConnectionHandler.class);
        bigIntent.setAction(BIGMESSAGE);

        sConn = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                handler = ((ConnectionHandler.MyBinder)service).getService();
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {
                handler = null;
            }
        };

        br = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Log.i("myinfo", "Broadcast recieved");
                if(intent.getAction().equals(CONNECTEDACTION)) {
                    toggleConnectionStatus();
                }

                if(intent.getAction().equals(NEWMESSAGE)) {
                    addMessage(intent.getStringExtra("Sender"), intent.getStringExtra("Text"));
                }

                if(intent.getAction().equals(CONNECTIONKILL)) {
                    handleKillRequest();
                }
            }
        };

        broadcastIntents = new IntentFilter();
        broadcastIntents.addAction(CONNECTEDACTION);
        broadcastIntents.addAction(NEWMESSAGE);
        broadcastIntents.addAction(CONNECTIONKILL);
        registerReceiver(br, broadcastIntents);

    }

    protected void addMessage(String sender, String text) {
        Log.i("myinfo", "Trying to show message");
        View view = inflater.inflate(R.layout.message, null);
        ((TextView)view.findViewById(R.id.sender)).setText(sender);
        ((TextView)view.findViewById(R.id.text)).setText(text);
        list.addView(view);
    }

    protected void handleKillRequest() {
        Log.i("myinfo", "Asking service to die");
        startService(killerIntent);
        toggleConnectionStatus();
    }

    protected void toggleConnectionStatus() {
        int color;
        if(connected) {
            color = Color.GREEN;
            connected = false;
        } else {
            color = Color.RED;
            connected = true;
        }

        Paint temp = new Paint();
        temp.setColor(color);
        indicator.setBackgroundColor(connected ? getResources().getColor(R.color.connected) : getResources().getColor(R.color.notConnected));
        indicator.invalidate();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            startOptions();
            return true;
        }
        if (id == R.id.drop_connection) {
            handleKillRequest();
            return true;
        }

        if (id == R.id.spam) {
            startService(spamIntent);
            return true;
        }
        if (id == R.id.big) {
            startService(bigIntent);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onStart() {
        super.onStart();
        bindService(intent, sConn, 0);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        handleKillRequest();
        unregisterReceiver(br);
        unbindService(sConn);
    }

    private void startOptions() {
        Intent options = new Intent(this, OptionsActivity.class);
        startActivity(options);
    }


    @Override
    public void onClick(View v) {
        SharedPreferences sPref;
        sPref = getSharedPreferences("serverdata", MODE_PRIVATE);
        if(v.getId() == R.id.options) {
            startOptions();
            return;
        }
        if(v.getId() == R.id.send) {


            intent.setAction(MainActivity.NEWMESSAGE);
            intent.putExtra("Sender", sPref.getString("name", "AndroidApp"));
            intent.putExtra("Text", message.getText().toString());
            startService(intent);
            addMessage(sPref.getString("name", "AndroidApp"), message.getText().toString());
        } else {
            intent.setAction(MainActivity.CONNECTEDACTION);
            intent.putExtra("address", sPref.getString("address", "192.168.1.103:6061"));
            startService(intent);
        }

    }
}
