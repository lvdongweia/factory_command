package com.avatarmind.factorycommand.service;

import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.Process;

import com.avatarmind.factorycommand.Utils;


public class FacCmdClient {
    private static final String TAG = "FacCmdClient";
    private static IActivityTestService sService;

    public FacCmdClient() {}

    private static IActivityTestService getService() {
        if (sService != null) {
            return sService;
        }

        IBinder b = ServiceManager.getService("IActivityTestService");
        if (b != null) {
            sService = IActivityTestService.Stub.asInterface(b);
        }
        return sService;
    }

    public void registerCameraClient (ICameraActivity client) {
        IActivityTestService service = getService();
        try {
            service.registerCameraActivity(Process.myPid(), client);
        } catch (RemoteException e) {
            Utils.Logd(TAG, e.getMessage());
        }
    }

    public void unregisterCameraClient(ICameraActivity client) {
        IActivityTestService service = getService();
        try {
            service.unregisterCameraActivity(client);
        } catch (RemoteException e) {
            Utils.Logd(TAG, e.getMessage());
        }
    }

    public void registerLcdClient(ILcdActivity client) {
        IActivityTestService service = getService();
        try {
            service.registerLcdActivity(Process.myPid(), client);
        } catch (RemoteException e) {
            Utils.Logd(TAG, e.getMessage());
        }
    }

    public void unregisterLcdClient(ILcdActivity client) {
        IActivityTestService service = getService();
        try {
            service.unregisterLcdActivity(client);
        } catch (RemoteException e) {
            Utils.Logd(TAG, e.getMessage());
        }
    }

    public ITouchPanelActivity getTouchPannel() {
        IActivityTestService service = getService();

        ITouchPanelActivity touch = null;
        try {
            touch = service.getTouchPanelActivity();
        } catch (RemoteException e) {
            Utils.Logd(TAG, e.getMessage());
        }

        return touch;
    }

}
