package com.nidium.android;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {
    private ListView mNMLListView;
    private TextView mNMLListTitle;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        final Button b = (Button)findViewById(R.id.load_remote_nml);
        final EditText url = (EditText)findViewById(R.id.nml_url);

        b.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                start(url.getText().toString());
            }
        });

        mNMLListView = (ListView) findViewById(R.id.nml_list);
        mNMLListTitle = (TextView) findViewById(R.id.nml_list_title);
        mNMLListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String file = (String) parent.getAdapter().getItem(position);
                start(file);
            }
        });

        updateNMLs();
    }

    private void updateNMLs()
    {
        ArrayList<String> nmls = this.getNMLS();
        if (nmls == null) {
            nmls = new ArrayList<String>();
            mNMLListTitle.setText("No \"nidium\" directory on your phone");
        } else {
            mNMLListTitle.setText("Found " + nmls.size() + " NML file ou your phone : ");
        }

        ArrayAdapter<String> arrayAdapter = new ArrayAdapter<String>(
                this,
                android.R.layout.simple_list_item_1,
                nmls);
        mNMLListView.setAdapter(arrayAdapter);
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateNMLs();
    }


    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            updateNMLs();
        }
    }

    private ArrayList<String> getNMLS() {
        File f = new File(Nidroid.getUserDirectory() + "/nidium/");
        if (!f.isDirectory()) {
            return null;
        } else {
            File files[] = f.listFiles();
            ArrayList<String> ret = new ArrayList<String>();
            for (int i = 0; i < files.length; i++) {
                ret.add(files[i].toString());
            }
            return ret;
        }
    }

    private void start(String nml) {
        Log.d("NidiumLauncher", "Loading file " + nml);
        Intent intent = new Intent(this, NidiumActivity.class);
        intent.putExtra("nml", nml);
        this.startActivity(intent);
        setContentView(R.layout.activity_main);
    }
}
