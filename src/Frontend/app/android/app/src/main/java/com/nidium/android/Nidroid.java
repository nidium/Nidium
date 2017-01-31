package com.nidium.android;

import android.app.AlertDialog;
import android.app.NotificationManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.Environment;
import android.os.Handler;
import android.support.v4.app.NotificationCompat;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.SurfaceView;

import org.libsdl.app.SDLActivity;

import java.io.IOException;
import java.util.Locale;

public class Nidroid {
    Context mCx = null;
    SurfaceView mSurface = null;
    Handler mMainHandler;
    static final String TAG = "Nidroid";

    public Nidroid(SurfaceView v) {
        mCx = v.getContext();
        mSurface = v;
        mMainHandler = new Handler(mCx.getMainLooper());
    }

    static double getPixelRatio()
    {
        DisplayMetrics metrics = Resources.getSystem().getDisplayMetrics();
        return metrics.density;
    }

    static String getUserDirectory() {
        try {
            return Environment.getExternalStorageDirectory().getCanonicalPath();
        } catch (IOException e) {
            Log.e(TAG, "Failed to get external storage directory");
            return "/";
        }
    }

    static String getLanguage() {
        return Locale.getDefault().toString();
    }

    int getSurfaceWidth() {
        return mSurface.getWidth();

    }
    int getSurfaceHeight() {
        return mSurface.getHeight();
    }

    void alert(String message, int level) {
        Log.d(TAG, "Nidium alert(" + level + ") : " + message);
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                new AlertDialog.Builder(mCx)
                        .setMessage("Are you sure you want to delete this entry?")
                        .setNeutralButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                dialog.cancel();
                            }
                        })
                        .setIcon(android.R.drawable.ic_dialog_alert)
                        .show();
            }
        });
    }

    void notify(final String title, final String message) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                NotificationCompat.Builder mBuilder =
                        new NotificationCompat.Builder(Nidroid.this.mCx)
                                .setSmallIcon(android.R.drawable.ic_dialog_info)
                                .setContentTitle(title)
                                .setContentText(message);

                NotificationManager mNotificationManager =
                        (NotificationManager) Nidroid.this.mCx.getSystemService(Context.NOTIFICATION_SERVICE);
                mNotificationManager.notify(0, mBuilder.build());
            }
        });
    }

    String getCacheDirectory() {
        try {
            return mCx.getFilesDir().getCanonicalPath();
        } catch (IOException e) {
            Log.e(TAG, "Failed to get cache directory");
            return "/";
        }
    }

    public static native void nidiumInit(Nidroid n);
}
