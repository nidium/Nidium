package com.nidium.android;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.NotificationManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.Environment;
import android.os.Handler;
import android.support.v4.app.NotificationCompat;
import android.support.v4.view.GestureDetectorCompat;
import android.text.method.Touch;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Scroller;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Locale;

public class Nidroid implements Flinger.Listener {
    private GestureDetectorCompat mDetector;
    private boolean mIsScrolling = false;
    private Flinger mFlinger = null;
    Context mCx = null;
    Activity mActivity = null;
    SurfaceView mSurface = null;
    Handler mMainHandler;
    static final String TAG = "Nidroid";


    public class TouchState {
        public static final int START = 0;
        public static final int MOVE = 1;
        public static final int END = 2;
    };

    public Nidroid(Activity a, SurfaceView v, final Method sdlOnTouch) {
        mCx = a.getApplicationContext();
        mActivity = a;
        mSurface = v;

        mMainHandler = new Handler(mCx.getMainLooper());

        // Override SDL onTouch listener so we can process scroll/fling gesture
        v.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (mFlinger != null) {
                    mFlinger.forceFinished();
                    mFlinger = null;
                }

                boolean consumed = mDetector.onTouchEvent(event);

                // Invoke original SDL onTouch method
                try {
                    sdlOnTouch.invoke(mSurface, v, event);
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                }

                if ((event.getAction() == MotionEvent.ACTION_UP || event.getAction() == MotionEvent.ACTION_CANCEL) && mIsScrolling && !consumed) {
                    Nidroid.onScroll(event.getX(), event.getY(), 0, 0, TouchState.END);
                    mIsScrolling = false;
                }

                return true;
            }
        });

        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                mDetector = new GestureDetectorCompat(mCx, new GestureDetector.SimpleOnGestureListener() {
                    @Override
                    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
                        if (!mIsScrolling) {
                            Nidroid.onScroll(e1.getX(), e1.getY(), 0, 0, TouchState.START);
                        }

                        Nidroid.onScroll(e2.getX(), e2.getY(), 0, 0, TouchState.MOVE);

                        mIsScrolling = true;

                        return true;
                    }

                    @Override
                    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
                        int state = TouchState.END;

                        if (velocityX != 0 || velocityY != 0) {
                            if (mFlinger != null) mFlinger.forceFinished();

                            state = TouchState.MOVE;

                            mFlinger = new Flinger(mCx, mMainHandler);
                            mFlinger.setListener(Nidroid.this);
                            mFlinger.start((int)e2.getX(), (int)e2.getY(), (int)velocityX, (int)velocityY);
                        }

                        Nidroid.onScroll(e2.getX(), e2.getY(), velocityX, velocityY, state);

                        return true;
                    }
                });
            }
        });
    }


    @Override
    public void onFlingerUpdate(int x, int y, boolean finished) {
        if (finished) {
            mIsScrolling = false;
            mFlinger = null;
        }

        Nidroid.onFlingUpdate(x, y, finished);
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

    void alert(final String message, int level) {
        Log.d(TAG, "Nidium alert(" + level + ") : " + message);
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                new AlertDialog.Builder(mActivity)
                        .setMessage(message)
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
    public static native void onScroll(float x, float y, float velocityX, float velocityY, int state);
    public static native void onFlingUpdate(int scrollX, int scrollY, boolean finished);
}
