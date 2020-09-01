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
 * @author elias_naur <elias_naur@users.sourceforge.net>
 * @version $Revision$
 */


#include <boat.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <jni.h>

#include "common_tools.h"
#include "extgl.h"
#include "extgl_egl.h"
#include "context.h"
#include "org_lwjgl_opengl_BoatDisplay.h"
#include "org_lwjgl_opengl_BoatDisplayPeerInfo.h"
#include "org_lwjgl_BoatSysImplementation.h"

#define ERR_MSG_SIZE 1024

static EGLSurface egl_surface = EGL_NO_SURFACE;

static bool checkBoatError(JNIEnv *env, EGLDisplay disp) {
	return (*env)->ExceptionCheck(env) == JNI_FALSE;
}

static int processEvent() {
	JNIEnv *env = attachCurrentThread();
	if (env != NULL) {
		jclass org_lwjgl_BoatDisplay_class = (*env)->FindClass(env, "org/lwjgl/opengl/BoatDisplay");
		if (org_lwjgl_BoatDisplay_class == NULL) {
			// Don't propagate error
			(*env)->ExceptionClear(env);
			detachCurrentThread();
			return 0;
		}
		jmethodID handler_method = (*env)->GetStaticMethodID(env, org_lwjgl_BoatDisplay_class, "processEvent", "()I");
		if (handler_method == NULL) {
		        detachCurrentThread();
			return 0;
		}
		return (*env)->CallStaticIntMethod(env, org_lwjgl_BoatDisplay_class, handler_method);
	} else {
	        detachCurrentThread();
		return 0;
	}
}
static jlong openDisplay(JNIEnv *env) {

	if (lwjgl_eglGetDisplay == NULL) {
		throwException(env, "eglGetDisplay() not available!");
		return (intptr_t)NULL;
	}
	EGLDisplay display_connection = lwjgl_eglGetDisplay(boatGetNativeDisplay());

	return (intptr_t)display_connection;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_DefaultSysImplementation_getJNIVersion
  (JNIEnv *env, jobject ignored) {
	return org_lwjgl_BoatSysImplementation_JNI_VERSION;
}

JNIEXPORT jstring JNICALL Java_org_lwjgl_opengl_BoatDisplay_getErrorText(JNIEnv *env, jclass unused, jlong display_ptr, jlong error_code) {
	char* err_msg_buffer = "Boat Error!";
	return NewStringNativeWithLength(env, err_msg_buffer, strlen(err_msg_buffer));
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_callErrorHandler(JNIEnv *env, jclass unused, jlong handler_ptr, jlong display_ptr, jlong event_ptr) {
	
	return 0;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatDisplay_setErrorHandler(JNIEnv *env, jclass unused) {
	
	return 0;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatDisplay_resetErrorHandler(JNIEnv *env, jclass unused, jlong handler_ptr) {
	
	return 0;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatDisplay_openDisplay(JNIEnv *env, jclass clazz) {
	return openDisplay(env);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatDisplay_closeDisplay(JNIEnv *env, jclass clazz, jlong display) {
        
	lwjgl_eglTerminate( (EGLDisplay)(intptr_t)display );
	
}


JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatDisplayPeerInfo_initDrawable(JNIEnv *env, jclass clazz, jlong window, jobject peer_info_handle) {
	BoatPeerInfo *peer_info = (*env)->GetDirectBufferAddress(env, peer_info_handle);
	peer_info->drawable = egl_surface;
	
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatDisplayPeerInfo_initDefaultPeerInfo(JNIEnv *env, jclass clazz, jlong display, jint screen, jobject peer_info_handle, jobject pixel_format) {
	EGLDisplay disp = (EGLDisplay)(intptr_t)display;
	initPeerInfo(env, peer_info_handle, disp, screen, pixel_format, true, EGL_WINDOW_BIT, true, false);
}

static void destroyWindow(JNIEnv *env, EGLSurface disp, ANativeWindow* window) {
	
	if (egl_surface != EGL_NO_SURFACE) {
		lwjgl_eglDestroySurface(disp, egl_surface);
		egl_surface = EGL_NO_SURFACE;
	}
	
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGetX(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	
	return 0;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGetY(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	
	return 0;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGetWidth(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	
	ANativeWindow* win = (ANativeWindow*)(intptr_t)window_ptr;
	return ANativeWindow_getWidth(win);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGetHeight(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
        
	ANativeWindow* win = (ANativeWindow*)(intptr_t)window_ptr;
	return ANativeWindow_getHeight(win);
}

static ANativeWindow* createWindow(JNIEnv* env, EGLDisplay disp, int screen, jint window_mode, BoatPeerInfo *peer_info, int x, int y, int width, int height, jboolean undecorated, long parent_handle, jboolean resizable) {
	
	ANativeWindow* win = boatGetNativeWindow();
	
	return win;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatDisplay_nCreateWindow(JNIEnv *env, jclass clazz, jlong display, jint screen, jobject peer_info_handle, jobject mode, jint window_mode, jint x, jint y, jboolean undecorated, jlong parent_handle, jboolean resizable) {
	
	
	EGLDisplay disp = (EGLDisplay)(intptr_t)display;
	BoatPeerInfo *peer_info = (*env)->GetDirectBufferAddress(env, peer_info_handle);
	EGLConfig config = getEGLConfigFromPeerInfo(env, peer_info);
	jclass cls_displayMode = (*env)->GetObjectClass(env, mode);
	jfieldID fid_width = (*env)->GetFieldID(env, cls_displayMode, "width", "I");
	jfieldID fid_height = (*env)->GetFieldID(env, cls_displayMode, "height", "I");
	int width = (*env)->GetIntField(env, mode, fid_width);
	int height = (*env)->GetIntField(env, mode, fid_height);
	ANativeWindow* win = createWindow(env, disp, screen, window_mode, peer_info, x, y, width, height, undecorated, parent_handle, resizable);
	if ((*env)->ExceptionOccurred(env)) {
		return 0;
	}
	
	egl_surface = lwjgl_eglCreateWindowSurface(disp, config, win, NULL);

	boatSetCurrentEventProcessor(processEvent);
	
	if (!checkBoatError(env, disp)) {
		lwjgl_eglDestroySurface(disp, egl_surface);
		destroyWindow(env, disp, win);
	}
	return (jlong)(intptr_t)win;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatDisplay_nDestroyWindow(JNIEnv *env, jclass clazz, jlong display, jlong window_ptr) {
	EGLDisplay disp = (EGLDisplay)(intptr_t)display;
	ANativeWindow* window = (ANativeWindow*)(intptr_t)window_ptr;
	destroyWindow(env, disp, window);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGrabPointer(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr, jlong cursor_ptr) {
	
	boatSetCursorMode(CursorDisabled);
	return 0;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatDisplay_nUngrabPointer(JNIEnv *env, jclass unused, jlong display_ptr) {
	boatSetCursorMode(CursorEnabled);
	return 0;
}

