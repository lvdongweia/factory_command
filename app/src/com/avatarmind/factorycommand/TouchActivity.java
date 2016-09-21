package com.avatarmind.factorycommand;

import android.app.Activity;
import android.graphics.PointF;
import android.os.Bundle;
import android.os.RemoteException;
import android.widget.TextView;

import com.avatarmind.factorycommand.service.FacCmdClient;
import com.avatarmind.factorycommand.service.ITouchPanelActivity;

public class TouchActivity extends Activity implements TouchView.OnTouchListener {
    private static final String TAG = "TouchActivity";

    private FacCmdClient mFacCmd;
    private ITouchPanelActivity mTouchClient;

    private TouchView mTouchView;
    private TextView mInfoText;

    private boolean mIsTouched;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_touch);

        mTouchView = (TouchView) findViewById(R.id.touch_view);
        mInfoText = (TextView) findViewById(R.id.infotext);

        mFacCmd = new FacCmdClient();
        mTouchClient = mFacCmd.getTouchPannel();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mTouchView.setOnTouchListener(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    private void reportTouchEvent() {
        if (!mIsTouched) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    mIsTouched = true;
                    try {
                        Thread.sleep(500);
                        if (mTouchClient != null) {
                            PointF[] pts = mTouchView.getTouchPoints();
                            final int count = pts.length;
                            if (count > 0) {
                                int[] points = new int[count*2];
                                for (int i = 0; i < count; i++) {
                                    points[i*2] = (int)pts[i].x;
                                    points[i*2+1] = (int)pts[i].y;
                                }
                                // sort points


                                Utils.Logd(TAG, "Report touch count:" + count);
                                mTouchClient.display(count, points);
                            }
                        }
                    } catch (InterruptedException e) {
                        // continue
                    } catch (RemoteException e) {
                        // ignore
                    }
                }
            }).start();
        }
    }

    @Override
    public void onTouchPointChange(PointF[] coords) {
        int count = coords.length;
        StringBuilder info = new StringBuilder();
        info.append("检测到触点:" + count);

        reportTouchEvent();

        if (count > 0) {
            for (int i = 0; i < count; i++) {
                info.append("\n");
                info.append("(")
                    .append(coords[i].x)
                    .append(", ")
                    .append(coords[i].y)
                    .append(")");
            }

        } else {
            mIsTouched = false;
        }

        mInfoText.setText(info.toString());
        //Utils.Logd(TAG, info.toString());
    }
}
