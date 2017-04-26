package com.nidium.android;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.renderscript.ScriptGroup;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;

import static android.R.id.list;

public class DevelopmentActivity extends Activity {
    private static final String TAG = "MainActivity";
    private ListView mNMLListView;
    private TextView mNMLListTitle;
    private static final String nidiumDirName = "nidium";
    private static final int PERMISSION_STORAGE = 0;
    private boolean mHasStoragePermission = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
                    PERMISSION_STORAGE);
        } else {
            mHasStoragePermission = true;
        }

        final Button b = (Button) findViewById(R.id.load_remote_nml);
        final EditText url = (EditText) findViewById(R.id.nml_url);

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

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        switch (requestCode) {
            case PERMISSION_STORAGE: {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    mHasStoragePermission = true;
                }
            }
        }
    }

    private void updateNMLs()
    {
        final String dirName = Nidroid.getUserDirectory() + '/' + DevelopmentActivity.nidiumDirName + '/';
        if (!mHasStoragePermission) {
            mNMLListTitle.setText("Application does not have permission to read " + dirName);
            return;
        }

        ArrayList<String> nmls = this.getNMLS(dirName);
        if (nmls == null) {
            nmls = new ArrayList<String>();
            mNMLListTitle.setText("No '" + dirName + "' directory on your phone");
        } else {
            if (nmls.size() > 0) {
                mNMLListTitle.setText("Found " + nmls.size() + " .nml files in directory '" + dirName + "'. ");
            } else {
                mNMLListTitle.setText("No .nml files in directory '" + dirName + "'.");
            }
        }

        ListView listView = (ListView) findViewById(R.id.nml_list);
        NMLFileAdapter arrayAdapter = new NMLFileAdapter(
                this,
                R.layout.nml_list_item_row,
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

    private ArrayList<String> getNMLS(String dirName) {
        final File f = new File(dirName);
        if (!f.isDirectory()) {
            return null;
        } else {
            ArrayList<String> ret = new ArrayList<String>();
            recurseFindNMLS(f, ret);
            return ret;
        }
    }

    private void recurseFindNMLS(File f, ArrayList<String> nmls) {
        File files[] = f.listFiles();
        if (files != null ) {
            for (int i = 0; i < files.length; i++) {
                String file = files[i].toString();
                if (files[i].isDirectory()) {
                    recurseFindNMLS(files[i], nmls);
                } else if (file.endsWith(".nml")) {
                    nmls.add(files[i].toString());
                }
            }
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
