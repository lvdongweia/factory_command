/*
 * WARNING! Update IActivityTestService.h and IActivityTestService.cpp if you change this
 * file. In particular, the ordering of the methods below must match the
 * TRANSACTION enum in IActivityTestService.cpp
 */

/**
 * package name keep in sync with Factory APP
 */
package com.avatarmind.factorycommand.service;

import com.avatarmind.factorycommand.service.ILcdActivity;
import com.avatarmind.factorycommand.service.ICameraActivity;
import com.avatarmind.factorycommand.service.ITouchPanelActivity;

interface IActivityTestService {
    void registerCameraActivity(int pid, in ICameraActivity cameraActivity);
    void unregisterCameraActivity(in ICameraActivity cameraActivity);

    void registerLcdActivity(int pid, in ILcdActivity lcdActivity);
    void unregisterLcdActivity(in ILcdActivity lcdActivity);

    ITouchPanelActivity getTouchPanelActivity();
}
