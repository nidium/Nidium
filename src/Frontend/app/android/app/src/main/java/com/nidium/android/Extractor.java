package com.nidium.android;

import android.app.Activity;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by efyx on 4/25/17.
 */

public class Extractor {
    private static final String TAG = "Extractor";
    private Activity mActivity;

    public Extractor(Activity a) {
        mActivity = a;
    }

    public boolean setup() {
        return setupApp();
    }

    private boolean setupApp() {
        try {
            File dest = null;
            dest = new File(mActivity.getFilesDir().getCanonicalPath() + "/nidium");
            if (!dest.exists()) {
                dest.mkdir();
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        try {
            File dest = null;
            dest = new File(mActivity.getFilesDir().getCanonicalPath() + "/nidium/" + BuildConfig.VERSION_NAME);
            if (dest.exists()) {
                return true;
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        boolean success = unpackAssets("nidium");
        if (success) {
            try {
                File dest = new File(mActivity.getFilesDir().getCanonicalPath() + "/nidium/" + BuildConfig.VERSION_NAME);
                dest.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
                return false;
            }
        }

        return success;
    }

    private boolean unpackAssets(String path) {
        AssetManager assetManager = mActivity.getAssets();

        Log.d(TAG, "Unpacking assets from " + path);

        String [] list;
        InputStream in = null;
        OutputStream out = null;
        try {
            list = mActivity.getAssets().list(path);
            if (list.length > 0) {
                for (String file : list) {
                    String src = path + "/" + file;

                    if (file.contains(".")) {
                        File dest = new File(mActivity.getFilesDir().getCanonicalPath() + "/" + path + "/" + file);

                        Log.d(TAG, "Source=" + src + " Destination=" + dest);

                        in = assetManager.open(src);
                        out = new FileOutputStream(new File(dest.getCanonicalPath()));
                        copyFile(in, out);
                    } else {
                        File dest = new File(mActivity.getFilesDir().getCanonicalPath() + "/" + path + "/" + file);
                        Log.d(TAG, "New directory " + dest);
                        if (!dest.exists()) {
                            if (!dest.mkdir()) {
                                Log.e(TAG, "Failed to create directory " + dest);
                                return false;
                            }
                        }
                        if (!unpackAssets(path + "/" + file))
                            return false;
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    // NOOP
                }
            }
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    // NOOP
                }
            }
        }

        return true;
    }

    private void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while((read = in.read(buffer)) != -1){
            out.write(buffer, 0, read);
        }
    }
}
