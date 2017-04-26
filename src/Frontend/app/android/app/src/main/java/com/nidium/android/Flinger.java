package com.nidium.android;

import android.content.Context;
import android.os.Handler;
import android.util.Log;
import android.widget.Scroller;

/**
 * Created by efyx on 4/7/17.
 */

public class Flinger implements Runnable {
    public interface Listener {
        void onFlingerUpdate(Flinger f, int relX, int relY, boolean finished);
    }
    private static final String TAG = "Flinger";
    private Scroller mScroller;
    private Listener mListener;
    private Handler mHandler;
    private int mLastX;
    private int mLastY;
    private int mStartX;
    private int mStartY;
    private double mPixelRatio;

    Flinger(Context cx, double pixelRatio, Handler handler) {
        mScroller = new Scroller(cx);
        mPixelRatio = pixelRatio;
        mHandler = handler;
    }

    void start(float x, float y, int velocityX, int velocityY) {
        mLastX = mStartX = (int)(x / mPixelRatio);
        mLastY = mStartY = (int)(y / mPixelRatio);
        mScroller.fling((int)x, (int)y, velocityX, velocityY, Integer.MIN_VALUE, Integer.MAX_VALUE, Integer.MIN_VALUE, Integer.MAX_VALUE);
        mHandler.post(this);
    }

    int getStartX() {
        return mStartX;
    }

    int getStartY() {
        return mStartY;
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    public void run() {
        if (mScroller.isFinished()) {
            mListener.onFlingerUpdate(this, 0, 0, true /* finished */);
            return;
        }

        boolean more = mScroller.computeScrollOffset();

        int x = (int)(mScroller.getCurrX() / mPixelRatio);
        int y = (int)(mScroller.getCurrY() / mPixelRatio);

        if (mListener != null && x != mLastX || y != mLastY) {
            mListener.onFlingerUpdate(this, mLastX - x, mLastY - y, !more);
        }

        mLastX = x;
        mLastY = y;

        if (more) {
            mHandler.post(this);
        }
    }


    void forceFinished() {
        if (!mScroller.isFinished()) {
            mScroller.forceFinished(true);
        }
    }
}