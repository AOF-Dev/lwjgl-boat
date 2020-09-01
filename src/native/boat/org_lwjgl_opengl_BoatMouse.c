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
 * $Id: org_lwjgl_input_Keyboard.c 2399 2006-06-30 19:28:00Z elias_naur $
 *
 * Linux mouse handling.
 *
 * @author elias_naur <elias_naur@users.sourceforge.net>
 * @version $Revision: 2399 $
 */


#include <boat.h>

#include "common_tools.h"
#include "org_lwjgl_opengl_BoatMouse.h"



JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatMouse_nGetWindowHeight(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
        
	return ANativeWindow_getHeight((ANativeWindow*)(intptr_t)window_ptr);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatMouse_nGetWindowWidth(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	return ANativeWindow_getWidth((ANativeWindow*)(intptr_t)window_ptr);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatMouse_nWarpCursor(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr, jint x, jint y) {
	
}


JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatMouse_nGetButtonCount(JNIEnv *env, jclass unused, jlong display_ptr) {
        
	return 3;
}
