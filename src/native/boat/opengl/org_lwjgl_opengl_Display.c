/*
 * Copyright (c) 2002-2008 LWJGL Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of 'LWJGL' nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * $Id$
 *
 * Boat specific display functions.
 *
 * @author cosine
 * @version $Revision$
 */

#include <boat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <jni.h>
#include <jawt_md.h>
#include "common_tools.h"
#include "extgl.h"
#include "extgl_egl.h"
#include "context.h"
#include "org_lwjgl_opengl_BoatDisplay.h"
#include "org_lwjgl_opengl_BoatDisplayPeerInfo.h"
#include "org_lwjgl_BoatSysImplementation.h"

static EGLSurface egl_window = EGL_NO_SURFACE;

static jlong openDisplay(JNIEnv *env) {
	EGLDisplay display_connection = lwjgl_eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (display_connection == NULL) {
		throwException(env, "Could not open EGL display connection");
		return (intptr_t)NULL;
	}
	return (intptr_t)display_connection;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_DefaultSysImplementation_getJNIVersion
  (JNIEnv *env, jobject ignored) {
	return org_lwjgl_BoatSysImplementation_JNI_VERSION;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatDisplay_openDisplay(JNIEnv *env, jclass clazz) {
	return openDisplay(env);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatDisplay_closeDisplay(JNIEnv *env, jclass clazz, jlong display) {
	EGLDisplay disp = (EGLDisplay)(intptr_t)display;
	lwjgl_eglTerminate(disp);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatDisplayPeerInfo_initDrawable(JNIEnv *env, jclass clazz, jobject peer_info_handle) {
	BoatPeerInfo *peer_info = (*env)->GetDirectBufferAddress(env, peer_info_handle);
	peer_info->drawable = egl_window;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatDisplayPeerInfo_initDefaultPeerInfo(JNIEnv *env, jclass clazz, jlong display, jobject peer_info_handle, jobject pixel_format) {
	EGLDisplay disp = (EGLDisplay)(intptr_t)display;
	initPeerInfo(env, peer_info_handle, disp, pixel_format, true, EGL_WINDOW_BIT);
}

static void destroyWindow(JNIEnv *env, EGLDisplay disp, ANativeWindow* window) {
	if (egl_window != EGL_NO_SURFACE) {
		lwjgl_eglDestroySurface(disp, egl_window);
		egl_window = EGL_NO_SURFACE;
	}
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGetX(JNIEnv *env, jclass unused, jlong window_ptr) {
	return 0;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGetY(JNIEnv *env, jclass unused, jlong window_ptr) {
	return 0;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGetWidth(JNIEnv *env, jclass unused, jlong window_ptr) {
	ANativeWindow* win = (ANativeWindow*)(intptr_t)window_ptr;

	return ANativeWindow_getWidth(win);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGetHeight(JNIEnv *env, jclass unused, jlong window_ptr) {
	ANativeWindow* win = (ANativeWindow*)(intptr_t)window_ptr;

	return ANativeWindow_getHeight(win);
}

static ANativeWindow* createWindow(JNIEnv* env, EGLDisplay disp, jint window_mode, BoatPeerInfo *peer_info, int x, int y, int width, int height, jboolean resizable) {
	ANativeWindow* win;

	win = boatGetNativeWindow();
//	printfDebugJava(env, "Created window");

	return win;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatDisplay_nCreateWindow(JNIEnv *env, jclass clazz, jlong display, jobject peer_info_handle, jobject mode, jint window_mode, jint x, jint y, jboolean resizable) {
	EGLDisplay disp = (EGLDisplay)(intptr_t)display;
	BoatPeerInfo *peer_info = (*env)->GetDirectBufferAddress(env, peer_info_handle);
	EGLConfig *fb_config = NULL;
	fb_config = getFBConfigFromPeerInfo(env, peer_info);
	if (fb_config == NULL)
		return 0;
	jclass cls_displayMode = (*env)->GetObjectClass(env, mode);
	jfieldID fid_width = (*env)->GetFieldID(env, cls_displayMode, "width", "I");
	jfieldID fid_height = (*env)->GetFieldID(env, cls_displayMode, "height", "I");
	int width = (*env)->GetIntField(env, mode, fid_width);
	int height = (*env)->GetIntField(env, mode, fid_height);
	ANativeWindow* win = createWindow(env, disp, window_mode, peer_info, x, y, width, height, resizable);
	if ((*env)->ExceptionOccurred(env)) {
		return 0;
	}
	egl_window = lwjgl_eglCreateWindowSurface(disp, *fb_config, win, NULL);
	free(fb_config);
	return win;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatDisplay_nDestroyWindow(JNIEnv *env, jclass clazz, jlong display, jlong window_ptr) {
	EGLDisplay disp = (EGLDisplay)(intptr_t)display;
	ANativeWindow* window = (ANativeWindow*)(intptr_t)window_ptr;
	destroyWindow(env, disp, window);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGrabPointer(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr, jlong cursor_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = (Window)window_ptr;
	Cursor cursor = (Cursor)cursor_ptr;
	int grab_mask = PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
	return XGrabPointer(disp, win, False, grab_mask, GrabModeAsync, GrabModeAsync, win, cursor, CurrentTime);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nUngrabPointer(JNIEnv *env, jclass unused, jlong display_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	return XUngrabPointer(disp, CurrentTime);
}
