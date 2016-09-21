/*
 * WARNING! Update ICameraActivity.h and ICameraActivity.cpp if you change this
 * file. In particular, the ordering of the methods below must match the
 * TRANSACTION enum in ICameraActivity.cpp
 */

/**
 * package name keep in sync with Factory APP
 */
package com.avatarmind.factorycommand.service;

interface ICameraActivity {
    int openCamera();
    int closeCamera();

    int takePicture(int storage, String filename);
    int viewPicture(int storage, String filename);
    int preview();
    int compareImage(int pattern, int storage, String filename);
}
