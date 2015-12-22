package com.example.lightwave.messageclient;

import android.app.Activity;
import android.content.SharedPreferences;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;


public class OptionsActivity extends Activity implements View.OnClickListener  {
    EditText address;
    EditText name;
    Button save;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_options);
        address = (EditText) findViewById(R.id.address);
        name = (EditText) findViewById(R.id.name);
        save = (Button) findViewById(R.id.save);
        save.setOnClickListener(this);

        SharedPreferences sPref;
        sPref = getSharedPreferences("serverdata", MODE_PRIVATE);
        address.setText(sPref.getString("address", "192.168.1.103:6061"));
        name.setText(sPref.getString("name", "AndroidApp"));
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_options, menu);
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
            savePrefs();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private  void savePrefs() {
        Log.i("myinfo", "Saving preferences.");
        SharedPreferences sPref;
        sPref = getSharedPreferences("serverdata", MODE_PRIVATE);
        SharedPreferences.Editor ed = sPref.edit();
        ed.putString("address", address.getText().toString());
        ed.putString("name", name.getText().toString());
        ed.apply();
    }
    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.save) {
            savePrefs();
        }

    }
}
