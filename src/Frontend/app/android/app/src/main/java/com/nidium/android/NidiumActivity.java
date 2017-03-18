package com.nidium.android;

import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;

import org.libsdl.app.SDLActivity;

import java.io.File;

/**
 * Created by efyx on 2/7/17.
 */

public class NidiumActivity extends SDLActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        mNml = getIntent().getStringExtra("nml");
        if (mNml == null) finish();

        super.onCreate(savedInstanceState);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            finish();
            return false;
        }

        return super.dispatchKeyEvent(event);
    }

    @Override
    protected String[] getArguments() {
        return new String[] {mNml};
    }

    private String mNml;

    static {
        System.loadLibrary("nidium_android");
    }
}
