package com.nidium.android;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.NotificationManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.media.AudioManager;
import android.os.Environment;
import android.os.Handler;
import android.support.v4.app.NotificationCompat;
import android.support.v4.view.GestureDetectorCompat;
import android.text.InputType;
import android.text.method.Touch;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Scroller;

import org.libsdl.app.SDLActivity;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.security.Key;
import java.util.Locale;

class ScrollPosition {
    public float x = 0;
    public float y = 0;
}

public class Nidroid implements Flinger.Listener {
    private GestureDetectorCompat mDetector;
    private boolean mIsScrolling = false;
    private Flinger mFlinger = null;
    private ScrollPosition mScrollPosition = new ScrollPosition();
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

    // Always keep in sync with src/Binding/JSKeyboard.h KeyboradOptions enum
    public class KeyboardOptions {
        public static final int NORMAL      = 1 << 0;
        public static final int NUMERIC     = 1 << 1;
        public static final int TEXT_ONLY   = 1 << 2;
    }

    public Nidroid(Activity a, SurfaceView v, final Method sdlOnTouch) {
        mCx = a.getApplicationContext();
        mActivity = a;
        mSurface = v;

        mMainHandler = new Handler(mCx.getMainLooper());

        v.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                int ev;

                if (event.getAction() == KeyEvent.ACTION_UP) {
                    // For now, only fire events on down action
                    return false;
                }

                switch (keyCode) {
                    case KeyEvent.KEYCODE_BACK:
                        ev = 15;
                        break;
                    case KeyEvent.KEYCODE_VOLUME_UP:
                        ev = 16;
                        break;
                    case KeyEvent.KEYCODE_VOLUME_DOWN:
                        ev = 17;
                        break;
                    default:
                        return false;
                }

                Nidroid.onHardwareKey(ev, event);

                return true;
            }
        });

        // Override SDL onTouch listener so we can process scroll/fling gesture
        v.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                boolean consumed = mDetector.onTouchEvent(event);

                // Invoke original SDL onTouch method
                try {
                    sdlOnTouch.invoke(mSurface, v, event);
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                }

                // Android does not send us an end motion when scrolling gesture end without a fling. Workaround that.
                if ((event.getAction() == MotionEvent.ACTION_UP || event.getAction() == MotionEvent.ACTION_CANCEL) && mIsScrolling && !consumed) {
                    Nidroid.this.scroll(event.getX(), event.getY(), 0, 0, TouchState.END);
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
                            Nidroid.this.scroll(e1.getX(), e1.getY(), 0, 0, TouchState.START);
                        }

                        Nidroid.this.scroll(e2.getX(), e2.getY(), 0, 0, TouchState.MOVE);

                        mIsScrolling = true;

                        return true;
                    }

                    @Override
                    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
                        int state = TouchState.END;

                        if (velocityX != 0 || velocityY != 0) {
                            if (mFlinger != null) mFlinger.forceFinished();

                            state = TouchState.MOVE;

                            mFlinger = new Flinger(mCx, Nidroid.this.getPixelRatio(), mMainHandler);
                            mFlinger.setListener(Nidroid.this);
                            mFlinger.start(e2.getX(), e2.getY(), (int)velocityX, (int)velocityY);

                            Nidroid.this.scroll(e2.getX(), e2.getY(), velocityX, velocityY, TouchState.START);
                        }

                        Nidroid.this.scroll(e2.getX(), e2.getY(), velocityX, velocityY, state);

                        return true;
                    }
                });
            }
        });
    }

    public void dispatchEvent(final KeyEvent ev) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                switch (ev.getKeyCode()) {
                    case KeyEvent.KEYCODE_BACK:
                        mActivity.onBackPressed();
                        break;
                    case KeyEvent.KEYCODE_VOLUME_UP:
                    case KeyEvent.KEYCODE_VOLUME_DOWN:
                        AudioManager am = (AudioManager) mActivity.getSystemService(Context.AUDIO_SERVICE);
                        am.adjustVolume(ev.getKeyCode() == KeyEvent.KEYCODE_VOLUME_UP
                                ? AudioManager.ADJUST_RAISE
                                : AudioManager.ADJUST_LOWER, AudioManager.FLAG_SHOW_UI);
                        break;
                }
            }
        });
    }

    public void stopScrolling() {
        if (mFlinger != null) {
            mFlinger.forceFinished();
        }
    }

    @Override
    public void onFlingerUpdate(Flinger f, int relX, int relY, boolean finished) {
        int startX = f.getStartX();
        int startY = f.getStartY();

        if (finished) {
            mIsScrolling = false;
            mFlinger = null;
        }

        Nidroid.onScroll(startX, startY,
                         relX, relY,
                         0, 0, finished ? TouchState.END: TouchState.MOVE);
    }

    private void scroll(float  x, float y, float velocityX, float velocityY, int state)
    {
        float ratio = (float)this.getPixelRatio();

        if (state == TouchState.START) {
            mScrollPosition.x = x;
            mScrollPosition.y = y;
            Nidroid.onScroll(x / ratio, y / ratio, 0, 0, velocityX, velocityY, state);
            return;
        }


        int relX = (int)((mScrollPosition.x - x) / ratio);
        int relY = (int)((mScrollPosition.y - y) / ratio);

        if (relX == 0 && relY == 0) {
            // Position hasn't changed enough, ignore it
            return;
        }

        mScrollPosition.x = x;
        mScrollPosition.y = y;

        Nidroid.onScroll(x / ratio, y / ratio, relX, relY, velocityX, velocityY, state);

    }

    static void SetKeyboardOptions(int flags)
    {
        int outFlags = InputType.TYPE_CLASS_TEXT;

        if ((flags & KeyboardOptions.NUMERIC) == KeyboardOptions.NUMERIC) {
            outFlags = outFlags | InputType.TYPE_CLASS_NUMBER;
        }

        if ((flags & KeyboardOptions.TEXT_ONLY) == KeyboardOptions.TEXT_ONLY) {
            outFlags = outFlags | InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD;
        }

        SDLActivity.mKeyboardFlags = outFlags;
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
    public static native boolean onHardwareKey(int keyCode, KeyEvent ev);
    public static native void onScroll(float x, float y, float relX, float relY, float velocityX, float velocityY, int state);
}
