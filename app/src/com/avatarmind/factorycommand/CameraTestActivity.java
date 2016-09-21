package com.avatarmind.factorycommand;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.net.Uri;
import android.os.Bundle;

import android.os.ConditionVariable;
import android.os.Handler;
import android.os.Message;
import android.view.MenuItem;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.media.MediaActionSound;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;

import com.avatarmind.factorycommand.service.FacCmdClient;
import com.avatarmind.factorycommand.service.ICameraActivity;

public class CameraTestActivity extends Activity implements SurfaceHolder.Callback {
    private static final String TAG = "CameraTestActivity";
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private ImageView mViewPic;
    private FrameLayout mViewLayout;
    private TextView mPicName;
    private Camera mCameraDevice;
    private boolean mPaused = false;
    private boolean mIsPreview = false;

    private ConditionVariable mSignal;
    private MediaActionSound mActionSound;
    private FacCmdClient mFacCmd;

    private final CameraTestClient mCameraClient = new CameraTestClient();

    private final int MSG_START_PREVIEW = 0;
    private final int MSG_VIEW_PICTURE  = 1;

    private int mAsyncResult;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_START_PREVIEW:
                    mAsyncResult = cameraStartPreview();
                    break;

                case MSG_VIEW_PICTURE:
                    mAsyncResult = pictureView((String)msg.obj);
                    break;
            }
            mSignal.open();
        }
    };

    public class CameraTestClient extends ICameraActivity.Stub {
        public CameraTestClient() {}

        @Override
        public int openCamera() {
            Utils.Logd(TAG, "native:openCamera");
            return cameraOpen();
        }

        @Override
        public int closeCamera() {
            Utils.Logd(TAG, "native:closeCamera");
            return cameraClose();
        }

        @Override
        public int takePicture(int storage, String filename) {
            Utils.Logd(TAG, "native:takePicture");
            return cameraTakePicture();
        }

        @Override
        public int viewPicture(int storage, String filename) {
            Utils.Logd(TAG, "native:viewPicture");

            mSignal.close();
            mHandler.obtainMessage(MSG_VIEW_PICTURE, new String("capture.jpg")).sendToTarget();
            mSignal.block();
            return mAsyncResult;
        }

        @Override
        public int preview() {
            Utils.Logd(TAG, "native:preview");
            mSignal.close();
            mHandler.obtainMessage(MSG_START_PREVIEW).sendToTarget();
            mSignal.block();
            return mAsyncResult;
        }

        @Override
        public int compareImage(int pattern, int storage, String filename) {
            Utils.Logd(TAG, "native:compareImage");
            return -1;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);

        mSurfaceView = (SurfaceView)findViewById(R.id.camera_preview);
        mViewLayout = (FrameLayout)findViewById(R.id.view_layout);
        mViewPic = (ImageView)findViewById(R.id.view_picture);
        mPicName = (TextView)findViewById(R.id.pic_name);

        mActionSound = new MediaActionSound();
        mActionSound.load(MediaActionSound.SHUTTER_CLICK);

        mSignal = new ConditionVariable();

        mSurfaceView.getHolder().addCallback(this);

        mFacCmd = new FacCmdClient();
        mFacCmd.registerCameraClient(mCameraClient);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mPaused = false;
    }

    @Override
    protected void onNewIntent(Intent intent)    {
        super.onNewIntent(intent);
    }

    @Override
    protected void onPause() {
        mPaused = true;
        super.onPause();

        cameraClose();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mActionSound.release();
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Utils.Logd(TAG, "surfaceCreated");
        mSurfaceHolder = holder;
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mSurfaceHolder = null;
    }

    private int cameraOpen() {
        int result = 0;
        int width = 0, height = 0;
        int devices = Camera.getNumberOfCameras();
        if (devices <= 0) return -1;
        if (mCameraDevice != null) return 0;

        try {
            if (mPaused) return -1;
            Camera camera = Camera.open(0);
            Camera.Parameters parameters = camera.getParameters();
            List<Camera.Size> previewSizes = parameters.getSupportedPreviewSizes();
            for (Camera.Size sz : previewSizes) {
                if (sz.width > width) {
                    width = sz.width;
                    height = sz.height;
                }
            }
            parameters.setPreviewSize(previewSizes.get(0).width, previewSizes.get(0).height);

            List<Camera.Size> pictureSizes = parameters.getSupportedPictureSizes();
            for (Camera.Size sz : pictureSizes) {
                if (sz.width > width) {
                    width = sz.width;
                    height = sz.height;
                }
            }
            parameters.setPictureSize(pictureSizes.get(0).width, pictureSizes.get(0).height);

            parameters.setPictureFormat(PixelFormat.JPEG);
            parameters.set("orientation", "landscape");

            camera.setParameters(parameters);
            Utils.Logd(TAG, "Camera Resolution: " + width + "x" + height);
            mCameraDevice = camera;
            result = 0;
        } catch (Exception e) {
            Utils.Logd(TAG, e.getMessage());
            e.printStackTrace();
        }

        return result;
    }

    private int cameraStartPreview() {
        int result = 0;
        Camera camera = mCameraDevice;
        if (camera == null) return -1;
        if (mIsPreview) return 0;

        try {
            if (mPaused) return -1;

            mSurfaceView.setVisibility(View.VISIBLE);
            mViewLayout.setVisibility(View.GONE);

            camera.setPreviewDisplay(mSurfaceHolder);
            camera.startPreview();
            mIsPreview = true;
            result = 0;
            Utils.Logd(TAG, "preview success");
        } catch (Exception e) {
            Utils.Logd(TAG, e.getMessage());
            e.printStackTrace();
        }

        return result;
    }

    private int cameraStopPreview() {
        Camera camera = mCameraDevice;
        if (camera == null) return -1;

        camera.stopPreview();
        mIsPreview = false;
        return 0;
    }

    private int cameraTakePicture() {
        Camera camera = mCameraDevice;
        if (camera == null) return -1;
        if (!mIsPreview) return -1;

        camera.takePicture(mShutterCallback, null, mPictureCallback);

        Utils.Logd(TAG, "takepicture success");
        return 0;
    }

    private int pictureView(final String fileName) {
        if (fileName.isEmpty()) {
            return -1;
        }

        File file = new File(Utils.PIC_DIR + "/" + fileName);
        if (!file.exists()) {
            return -1;
        }

        if (mIsPreview) {
            cameraStopPreview();
        }

        mSurfaceView.setVisibility(View.GONE);
        mViewLayout.setVisibility(View.VISIBLE);
        mViewPic.setImageURI(null); // clear buffer

        Uri picUri = Uri.fromFile(file);
        mPicName.setText("名称：" + file.getName());
        mViewPic.setImageURI(picUri);

        Utils.Logd(TAG, "view picture success");
        return 0;
    }

    private int cameraClose() {
        Camera camera = mCameraDevice;
        if (camera == null) return 0;

        cameraStopPreview();
        camera.release();
        mCameraDevice = null;
        Utils.Logd(TAG, "camera closed");

        if (!mPaused) {
            //clearSurfaceView();
            //Utils.Logd(TAG, "reset surfaceView");
        }

        return 0;
    }

    private Camera.ShutterCallback mShutterCallback = new Camera.ShutterCallback() {
        @Override
        public void onShutter() {
            mActionSound.play(MediaActionSound.SHUTTER_CLICK);
        }
    };

    private Camera.PictureCallback mPictureCallback = new Camera.PictureCallback() {
        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            Utils.Logd(TAG, "onPictureTaken");

            final byte[] imgData = data;

            new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        String path = Utils.PIC_DIR + "/capture.jpg";
                        FileOutputStream out = new FileOutputStream(path);
                        out.write(imgData);
                        out.close();

                        Utils.Logd(TAG, "Save picture success");
                    } catch (FileNotFoundException e) {
                        Utils.Logd(TAG, e.getMessage());
                        e.printStackTrace();
                    } catch (IOException e) {
                        Utils.Logd(TAG, e.getMessage());
                        e.printStackTrace();
                    } finally {

                        if (!mPaused)
                            mCameraDevice.startPreview();
                    }
                }
            }).start();
        }
    };

    private void clearSurfaceView() {
        SurfaceHolder holder = mSurfaceHolder;
        Canvas canvas = holder.lockCanvas();
        canvas.drawColor(Color.GRAY);
        holder.unlockCanvasAndPost(canvas);
    }
}






