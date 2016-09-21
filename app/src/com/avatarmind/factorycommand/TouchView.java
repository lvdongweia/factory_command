package com.avatarmind.factorycommand;


import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PointF;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.MotionEvent.PointerCoords;
import android.widget.TextView;


import java.util.List;
import java.util.ArrayList;


public class TouchView extends View {
    private static final String TAG = "TouchView";

    private List<PointerState> mPoints = new ArrayList<PointerState>();
    private OnTouchListener mCallback;

    private int mCurNumPoints;
    private boolean mCurDown;

    private Paint mPointPaint;

    private class PointerState {
        private boolean mDown;
        private PointerCoords mCorrds = new PointerCoords();

        public PointerState() {};
    }

    public interface OnTouchListener {
        void onTouchPointChange(PointF[] coords);
    }

    public TouchView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setFocusableInTouchMode(true);

        init();
    }


    private void init() {
        mPointPaint = new Paint();
        mPointPaint.setAntiAlias(true);
        mPointPaint.setARGB(255, 255, 0, 0);
        mPointPaint.setStyle(Paint.Style.FILL);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        pointEventChange(event);

        if (event.getAction() == MotionEvent.ACTION_DOWN && !isFocused()) {
            requestFocus();
        }
        return true;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        synchronized (mPoints) {
            final int NP = mPoints.size();
            for (int i = 0; i < NP; i++) {
                final PointerState ps = mPoints.get(i);
                if (mCurDown && ps.mDown) {
                    canvas.drawLine(0, ps.mCorrds.y, 1920, ps.mCorrds.y, mPointPaint);
                    canvas.drawLine(ps.mCorrds.x, 0, ps.mCorrds.x, 1080, mPointPaint);
                    canvas.drawPoint(ps.mCorrds.x, ps.mCorrds.y, mPointPaint);
                }
            }
        }
    }

    private void pointEventChange(MotionEvent event) {
        final int action = event.getAction();
        final int actionMasked = event.getActionMasked();

        if (actionMasked != MotionEvent.ACTION_DOWN
                && actionMasked != MotionEvent.ACTION_POINTER_DOWN
                && actionMasked != MotionEvent.ACTION_UP
                && actionMasked != MotionEvent.ACTION_POINTER_UP
                && actionMasked != MotionEvent.ACTION_CANCEL) {
            return;
        }

        final int index = event.getActionIndex();
        final int id = event.getPointerId(index);
        //Utils.Logd(TAG, "================================");
        //Utils.Logd(TAG, "id:" + id + " index:" + index + " actionMasked:" + actionMasked);

        synchronized (mPoints) {
            int nPt = mPoints.size();
            while (nPt < id+1) {
                PointerState p = new PointerState();
                mPoints.add(p);
                nPt++;
            }

            final PointerState ps = mPoints.get(id);
            if (actionMasked == MotionEvent.ACTION_DOWN ||
                    actionMasked == MotionEvent.ACTION_POINTER_DOWN) {

                if (actionMasked == MotionEvent.ACTION_DOWN) {
                    mCurDown = true;
                    mCurNumPoints = 0;
                }
                mCurNumPoints += 1;
                ps.mDown = true;
                final PointerCoords coords = ps.mCorrds;
                event.getPointerCoords(index, coords); // must use index
            } else if (actionMasked == MotionEvent.ACTION_UP ||
                    actionMasked == MotionEvent.ACTION_CANCEL ||
                    actionMasked == MotionEvent.ACTION_POINTER_UP) {

                if (actionMasked == MotionEvent.ACTION_UP ||
                        actionMasked == MotionEvent.ACTION_CANCEL) {
                    mCurDown = false;
                }
                mCurNumPoints -= 1;
                ps.mDown = false;
            }
        }

        invalidate();
        notifyListener();
    }


    public void setOnTouchListener(OnTouchListener callback) {
        mCallback = callback;
    }

    public PointF[] getTouchPoints() {
        synchronized (mPoints) {
            int num = mCurNumPoints;
            PointF[] pts = new PointF[num];
            PointerState sp;
            int index = 0;

            final int size = mPoints.size();
            for (int i = 0; i < size; i++) {
                sp = mPoints.get(i);
                if (sp.mDown) {
                    pts[index++] = new PointF(sp.mCorrds.x, sp.mCorrds.y);
                }
            }
            return pts;
        }
    }

    private void notifyListener() {
        if (mCallback != null) {
            PointF[] coords = getTouchPoints();
            mCallback.onTouchPointChange(coords);
        }
    }

}
