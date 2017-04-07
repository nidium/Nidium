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
        void onFlingerUpdate(int x, int y, boolean finished);
    }
    private static final String TAG = "Flinger";
    private Scroller mScroller;
    private Listener mListener;
    private Handler mHandler;
    private int mLastX;
    private int mLastY;

    Flinger(Context cx, Handler handler) {
        mScroller = new Scroller(cx);
        mHandler = handler;
    }

    void start(int x, int y, int velocityX, int velocityY) {
        mScroller.fling(x, y, velocityX, velocityY, Integer.MIN_VALUE, Integer.MAX_VALUE, Integer.MIN_VALUE, Integer.MAX_VALUE);
        mHandler.post(this);
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    public void run() {
        if (mScroller.isFinished()) {
            mListener.onFlingerUpdate(mScroller.getCurrX(), mScroller.getCurrY(), true /* finished */);
            return;
        }

        boolean more = mScroller.computeScrollOffset();

        int x = mScroller.getCurrX();
        int y = mScroller.getCurrY();

        if (mListener != null && x != mLastX || y != mLastY) {
            mListener.onFlingerUpdate(mScroller.getCurrX(), mScroller.getCurrY(), !more);
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