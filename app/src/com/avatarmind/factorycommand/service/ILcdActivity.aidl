/*
 * WARNING! Update ILcdActivity.h and ILcdActivity.cpp if you change this
 * file. In particular, the ordering of the methods below must match the
 * TRANSACTION enum in ILcdActivity.cpp
 */

/**
 * package name keep in sync with Factory APP
 */
package com.avatarmind.factorycommand.service;

interface ILcdActivity {
    int display(int pattern);
}
