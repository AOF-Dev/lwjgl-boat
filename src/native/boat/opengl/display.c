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
 * Linux specific library for display handling.
 *
 * @author elias_naur <elias_naur@users.sourceforge.net>
 * @version $Revision$
 */

#include <boat.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common_tools.h"
#include "org_lwjgl_opengl_BoatDisplay.h"


#define NUM_XRANDR_RETRIES 5

typedef struct {
	int width;
	int height;
	int freq;
	
} mode_info;

static mode_info *getBoatDisplayModes(int *num_modes) {
        
        ANativeWindow* window = boatGetNativeWindow();
        int width = ANativeWindow_getWidth(window);
        int height = ANativeWindow_getHeight(window);

	mode_info *avail_modes = (mode_info *)malloc(sizeof(mode_info) * 1);
	if (avail_modes == NULL) {
		return NULL;
	}
	avail_modes->width = width;
	avail_modes->height = height;
	avail_modes->freq = 0; // No frequency support in Boat
	*num_modes = 1;
	return avail_modes;
}


static mode_info *getDisplayModes(jint extension, int *num_modes) {
	switch (extension) {
		case org_lwjgl_opengl_BoatDisplay_BOAT:
			return getBoatDisplayModes(num_modes);
		case org_lwjgl_opengl_BoatDisplay_NONE:
			// fall through
		default:
			return NULL;
	}
}

static jobjectArray getAvailableDisplayModes(JNIEnv * env, jint extension) {
	int num_modes, i;
	mode_info *avail_modes;
	int bpp = 24;
	avail_modes = getDisplayModes(extension, &num_modes);
	if (avail_modes == NULL) {
		printfDebugJava(env, "Could not get display modes");
		return NULL;
	}
	// Allocate an array of DisplayModes big enough
	jclass displayModeClass = (*env)->FindClass(env, "org/lwjgl/opengl/DisplayMode");
	jobjectArray ret = (*env)->NewObjectArray(env, num_modes, displayModeClass, NULL);
	jmethodID displayModeConstructor = (*env)->GetMethodID(env, displayModeClass, "<init>", "(IIII)V");

	for (i = 0; i < num_modes; i++) {
		jobject displayMode = (*env)->NewObject(env, displayModeClass, displayModeConstructor, avail_modes[i].width, avail_modes[i].height, bpp, avail_modes[i].freq);
		(*env)->SetObjectArrayElement(env, ret, i, displayMode);
	}
	free(avail_modes);
	return ret;
}

static bool setMode(JNIEnv *env, jint extension, int width, int height, int freq) {
	int num_modes, i;
	mode_info *avail_modes = getDisplayModes(extension, &num_modes);
	if (avail_modes == NULL) {
		printfDebugJava(env, "Could not get display modes");
		return false;
	}
	bool result = false;
	for (i = 0; i < num_modes; ++i) {
		printfDebugJava(env, "Mode %d: %dx%d @%d", i, avail_modes[i].width, avail_modes[i].height, avail_modes[i].freq);
		if (avail_modes[i].width == width && avail_modes[i].height == height && avail_modes[i].freq == freq) {
			switch (extension) {
				case org_lwjgl_opengl_BoatDisplay_BOAT:
					break;
				case org_lwjgl_opengl_BoatDisplay_NONE: // Should never happen, since NONE imply no available display modes
				default:   // Should never happen
					continue;
			}
			result = true;
			break;
		}
	}
	free(avail_modes);
	return result;
}
static bool switchDisplayMode(JNIEnv * env, jint extension, jobject mode) {
	if (mode == NULL) {
		throwException(env, "mode must be non-null");
		return false;
	}
	jclass cls_displayMode = (*env)->GetObjectClass(env, mode);
	jfieldID fid_width = (*env)->GetFieldID(env, cls_displayMode, "width", "I");
	jfieldID fid_height = (*env)->GetFieldID(env, cls_displayMode, "height", "I");
	jfieldID fid_freq = (*env)->GetFieldID(env, cls_displayMode, "freq", "I");
	int width = (*env)->GetIntField(env, mode, fid_width);
	int height = (*env)->GetIntField(env, mode, fid_height);
	int freq = (*env)->GetIntField(env, mode, fid_freq);
	if (!setMode(env, extension, width, height, freq)) {
		throwException(env, "Could not switch mode.");
		return false;
	}
	return true;
}
JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatDisplay_nSwitchDisplayMode(JNIEnv *env, jclass clazz, jlong display, jint screen, jint extension, jobject mode) {
	switchDisplayMode(env, extension, mode);
}

JNIEXPORT jobjectArray JNICALL Java_org_lwjgl_opengl_BoatDisplay_nGetAvailableDisplayModes(JNIEnv *env, jclass clazz, jlong display, jint screen, jint extension) {
	return getAvailableDisplayModes(env, extension);
}
