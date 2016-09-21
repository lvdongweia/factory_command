package com.avatarmind.factorycommand;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.avatarmind.factorycommand.service.FacCmdClient;
import com.avatarmind.factorycommand.service.ILcdActivity;

public class LCDActivity extends Activity {
    private static final String TAG = "LCDActivity";
    private ImageView mPureColor;
    private LinearLayout mGradientColor;
    private boolean mPaused;
    private int mTaskId;
    private Context mContext;

    private final int[] mColorArray = { Color.RED, Color.BLUE, Color.GREEN,
            Color.BLACK, Color.WHITE};

    private FacCmdClient mFacCmd;
    private final LCDTestClient mLcdClient = new LCDTestClient();

    private class LCDTestClient extends ILcdActivity.Stub {
        @Override
        public int display(int pattern) {
            Utils.Logd(TAG, "native:display - " + pattern);
            if (mPaused) {
                Utils.moveTaskToFront(mContext, mTaskId);
            }
            setBkColor(pattern);
            return 0;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_lcd);

        mPureColor = (ImageView)findViewById(R.id.lcd_color);
        mGradientColor = (LinearLayout)findViewById(R.id.color_gradient);

        mTaskId = getTaskId();
        mContext = this;

        mFacCmd = new FacCmdClient();
        mFacCmd.registerLcdClient(mLcdClient);
    }

    @Override
    protected void onResume() {
        super.onResume();

        mPaused = false;
        Intent intent = getIntent();
        int index = intent.getIntExtra(Utils.COLOR_INDEX, 0);
        setBkColor(index);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Utils.Logd(TAG, "onNewIntent");

        int index = intent.getIntExtra(Utils.COLOR_INDEX, 0);
        setBkColor(index);
    }

    @Override
    protected void onPause() {
        mPaused = true;
        super.onPause();
    }

    public boolean setBkColor(int index) {
        if (index < 1 || index > 7)
            return false;

        Utils.Logd(TAG, "Color index:" + index);
        final int id = index;

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (id <= 5) {
                    mPureColor.setVisibility(View.VISIBLE);
                    mGradientColor.setVisibility(View.GONE);
                    mPureColor.setBackgroundColor(mColorArray[id-1]);
                } else if (id == 6) {
                    mPureColor.setVisibility(View.GONE);
                    mGradientColor.setVisibility(View.VISIBLE);
                } else if (id == 7) {
                    mPureColor.setVisibility(View.VISIBLE);
                    mGradientColor.setVisibility(View.GONE);
                    mPureColor.setBackground(generateWhiteRectangle());
                }
            }
        });

        return true;
    }

    private Drawable generateWhiteRectangle() {
        Bitmap bitmap = Bitmap.createBitmap(1920, 1080, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        Rect rect = new Rect(0, 0, 1920, 1920);

        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setColor(Color.BLACK);
        paint.setStyle(Paint.Style.FILL);
        canvas.drawRect(rect, paint);

        paint.reset();
        paint.setColor(Color.WHITE);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(10);
        canvas.drawRect(rect, paint);

        return new BitmapDrawable(bitmap);
    }
}
