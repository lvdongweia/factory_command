/*
 * WARNING! Update ITouchPanelActivity.h and ITouchPanelActivity.cpp if you change this
 * file. In particular, the ordering of the methods below must match the
 * TRANSACTION enum in ITouchPanelActivity.cpp
 */

/**
 * package name keep in sync with Factory APP
 */
package com.avatarmind.factorycommand.service;

interface ITouchPanelActivity {
    int display(int num, in int[] points);
}
