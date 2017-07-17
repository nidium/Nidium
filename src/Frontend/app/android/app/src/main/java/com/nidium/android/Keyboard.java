package com.nidium.android;

import android.content.Context;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.CorrectionInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.RelativeLayout;

import org.libsdl.app.SDLActivity;

/**
 * Created by efyx on 4/20/17.
 */

public class Keyboard extends View implements View.OnKeyListener {
    private static final String TAG = "Keyboard";
    // Always keep in sync with src/Frontend/InputEvent.h Type enum
    public static class KeyboardEvent {
        public static final int COMPOSITION_START  = 15;
        public static final int COMPOSITION_UPDATE = 16;
        public static final int COMPOSITION_END    = 17;
        public static final int KEY_UP             = 18;
        public static final int KEY_DOWN           = 19;
    }

    // Always keep in sync with src/Binding/JSKeyboard.h KeyboradOptions enum
    public class KeyboardOptions {
        public static final int NORMAL      = 1 << 0;
        public static final int NUMERIC     = 1 << 1;
        public static final int TEXT_ONLY   = 1 << 2;
    }

    private Context mCx;
    private ViewGroup mLayout;
    private int mKeyboardFlags = KeyboardOptions.NORMAL;
    private InputConnection mIC;

    public Keyboard(Context cx, ViewGroup layout) {
        super(cx);
        setFocusableInTouchMode(true);
        setFocusable(true);
        setOnKeyListener(this);

        mCx = cx;
        mLayout = layout;
        mLayout.addView(this, new RelativeLayout.LayoutParams(0, 0));
    }

    public void setFlags(int flags) {
        mKeyboardFlags = flags;
    }

    @Override
    public boolean onCheckIsTextEditor() {
        return true;
    }

    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        Log.e(TAG, "onKey");
        // This handles the hardware keyboard input
        if (event.isPrintingKey() || keyCode == KeyEvent.KEYCODE_SPACE) {
            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                mIC.commitText(String.valueOf((char) event.getUnicodeChar()), 1);
            }
            return true;
        }

        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            Nidroid.onKeyboardKey(keyCode, Keyboard.KeyboardEvent.KEY_DOWN);
            return true;
        } else if (event.getAction() == KeyEvent.ACTION_UP) {
            Nidroid.onKeyboardKey(keyCode, Keyboard.KeyboardEvent.KEY_UP);
            return true;
        }

        return false;
    }

    @Override
    public boolean onKeyPreIme (int keyCode, KeyEvent event) {
        Log.e(TAG, "onKeyPreIme");
        // As seen on StackOverflow: http://stackoverflow.com/questions/7634346/keyboard-hide-event
        // FIXME: Discussion at http://bugzilla.libsdl.org/show_bug.cgi?id=1639
        // FIXME: This is not a 100% effective solution to the problem of detecting if the keyboard is showing or not
        // FIXME: A more effective solution would be to assume our Layout to be RelativeLayout or LinearLayout
        // FIXME: And determine the keyboard presence doing this: http://stackoverflow.com/questions/2150078/how-to-check-visibility-of-software-keyboard-in-android
        // FIXME: An even more effective way would be if Android provided this out of the box, but where would the fun be in that :)
        if (event.getAction()==KeyEvent.ACTION_UP && keyCode == KeyEvent.KEYCODE_BACK) {
            if (this.getVisibility() == View.VISIBLE) {
                SDLActivity.onNativeKeyboardFocusLost();
            }
        }
        return super.onKeyPreIme(keyCode, event);
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        mIC = new NdmInputConnection(this, true);

        outAttrs.inputType = SDLActivity.mKeyboardFlags;
        outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI
                | 33554432 /* API 11: EditorInfo.IME_FLAG_NO_FULLSCREEN */;

        return mIC;
    }

}
class NdmInputConnection extends BaseInputConnection {
    private static final String TAG = "NdmInputConnection";
    private boolean mIsComposing = false;
    public NdmInputConnection(View targetView, boolean fullEditor) {
        super(targetView, fullEditor);
    }

    @Override
    public boolean sendKeyEvent(KeyEvent event) {
        Log.e(TAG, "keyEvent");
        /*
         * This handles the keycodes from soft keyboard (and IME-translated
         * input from hardkeyboard)
         */
        int keyCode = event.getKeyCode();
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            Nidroid.onKeyboardKey(keyCode, Keyboard.KeyboardEvent.KEY_DOWN);
            return true;
        } else if (event.getAction() == KeyEvent.ACTION_UP) {
            Nidroid.onKeyboardKey(keyCode, Keyboard.KeyboardEvent.KEY_UP);
            return true;
        }

        return super.sendKeyEvent(event);
    }

    @Override
    public boolean commitCompletion(CompletionInfo text) {
        Log.e(TAG, "Commit compleation " + text.getText());
        return super.commitCompletion(text);
    }

    @Override
    public boolean commitCorrection(CorrectionInfo correctionInfo) {
        Log.e(TAG, "Commit correction" + correctionInfo.toString());
        return super.commitCorrection(correctionInfo);
    }

    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {
        Log.e(TAG, "commit /" + text + "/");
        mIsComposing = false;
        
        Nidroid.onComposition(null, Keyboard.KeyboardEvent.COMPOSITION_END);

        // Androids send th
        //Nidroid.onComposition(null, Keyboard.KeyboardEvent.COMPOSITION_START);
        //Nidroid.onComposition(text, Keyboard.KeyboardEvent.COMPOSITION_UPDATE);
        //Nidroid.onComposition(null, Keyboard.KeyboardEvent.COMPOSITION_END);

        return super.commitText(text, newCursorPosition);
    }

    @Override
    public boolean setComposingText(CharSequence text, int newCursorPosition) {
        if (!mIsComposing) {
            mIsComposing = true;
            Nidroid.onComposition(null, Keyboard.KeyboardEvent.COMPOSITION_START);
        }
        Nidroid.onComposition(text.toString(), Keyboard.KeyboardEvent.COMPOSITION_UPDATE);

        return super.setComposingText(text, newCursorPosition);
    }

    @Override
    public boolean deleteSurroundingText(int beforeLength, int afterLength) {
        // Workaround to capture backspace key. Ref: http://stackoverflow.com/questions/14560344/android-backspace-in-webview-baseinputconnection
        if (beforeLength == 1 && afterLength == 0) {
            // backspace
            return super.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL))
                    && super.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL));
        }

        return super.deleteSurroundingText(beforeLength, afterLength);
    }
}