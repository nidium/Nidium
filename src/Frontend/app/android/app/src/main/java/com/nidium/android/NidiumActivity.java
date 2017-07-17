package com.nidium.android;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.view.GestureDetectorCompat;
import android.util.Log;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.IOException;
import java.security.Key;

/**
 * Created by efyx on 2/7/17.
 */

public class NidiumActivity extends SDLActivity {

    private final static String TAG = "NidiumActivity";
    private String mNml;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "Starting NidiumActivity. BundleApp=" + BuildConfig.BundleApp);

        if (BuildConfig.BundleApp) {
            Extractor e = new Extractor(this);
            if (!e.setup()) {
                finish();
            }

            try {
                mNml = getFilesDir().getCanonicalPath() + "/nidium/" + BuildConfig.AppIndex;
            } catch (IOException err) {
                err.printStackTrace();
                finish();
            }
        } else {
            mNml = getIntent().getStringExtra("nml");
            if (mNml == null) {
                startNMLPicker();
            }
        }

        super.onCreate(savedInstanceState);

        // Override SDL text edit with our own implementation
        SDLActivity.mTextEdit = new Keyboard(this, SDLActivity.mLayout);
    }

    public Keyboard getKeyboard()
    {
        return (Keyboard)SDLActivity.mTextEdit;
    }

    private void startNMLPicker() {
        Intent myIntent = new Intent(this, DevelopmentActivity.class);
        this.startActivity(myIntent);
        //finish(); // Disabled for now, as this will crash nidium that is half initialized
    }

    @Override
    protected String[] getArguments() {
        return new String[] {mNml};
    }

    static {
        System.loadLibrary("nidium_android");
    }
}
