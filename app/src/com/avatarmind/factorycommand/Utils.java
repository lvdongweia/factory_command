package com.avatarmind.factorycommand;


import android.app.ActivityManager;
import android.content.Context;
import android.util.Log;

public class Utils {
    private static final String TAG = "FACTORY_COMMAND_APP";

    public static final String PIC_NAME = "pic_name";
    public static final String PIC_DIR = "/data/factory_test";
    public static final String COLOR_INDEX = "color_index";

    public Utils() {
    }

    public static void Logd(String tag, String msg) {

        Log.d(TAG, "[" + tag + "]:" + msg);

    }

    public static void moveTaskToFront(Context context, int taskId) {
        ActivityManager am = (ActivityManager)context.getSystemService(Context.ACTIVITY_SERVICE);
        am.moveTaskToFront(taskId, ActivityManager.MOVE_TASK_WITH_HOME);
    }
}
