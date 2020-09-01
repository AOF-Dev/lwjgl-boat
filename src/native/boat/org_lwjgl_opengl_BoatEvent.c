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
 * $Id: org_lwjgl_opengl_LinuxEvent.c 2598 2006-10-24 08:33:09Z elias_naur $
 *
 * @author elias_naur <elias_naur@users.sourceforge.net>
 * @version $Revision: 2598 $
 */


#include <boat.h>

#include <jni.h>
#include "common_tools.h"
#include "org_lwjgl_opengl_BoatEvent.h"

JNIEXPORT jobject JNICALL Java_org_lwjgl_opengl_BoatEvent_createEventBuffer(JNIEnv *env, jclass unused) {
	return newJavaManagedByteBuffer(env, sizeof(BoatInputEvent));
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatEvent_nSendEvent(JNIEnv *env, jclass unused, jobject event_buffer, jlong display_ptr, jlong window_ptr, jboolean propagate, jlong eventmask) {
	
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_BoatEvent_nNextEvent(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	boatGetCurrentEvent(event);
	
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetType(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->type;
}


JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetWindow(JNIEnv *env, jclass unused, jobject event_buffer) {
        
	return 0L; //(jlong)(intptr_t)boatGetCurrentWindow();
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetButtonTime(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->time;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetButtonType(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->type;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetButtonButton(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->mouse_button;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetButtonX(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->x;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetButtonY(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->y;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetKeyAddress(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return (jlong)(intptr_t)event;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetKeyTime(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->time;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetKeyType(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->type;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetKeyKeyCode(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->keycode;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_BoatEvent_nGetKeyKeyChar(JNIEnv *env, jclass unused, jobject event_buffer) {
	BoatInputEvent *event = (BoatInputEvent *)(*env)->GetDirectBufferAddress(env, event_buffer);
	return event->keychar;
}

