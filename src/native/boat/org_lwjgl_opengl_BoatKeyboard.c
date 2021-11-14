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
 *
 * Boat keyboard handling.
 *
 * @author cosine
 */

#include <boat.h>
#include "common_tools.h"
#include "org_lwjgl_opengl_BoatKeyboard.h"

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxKeyboard_getModifierMapping(JNIEnv *env, jclass unused, jlong display_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	XModifierKeymap *modifier_map = XGetModifierMapping(disp);
	return (intptr_t)modifier_map;
}

JNIEXPORT jboolean JNICALL Java_org_lwjgl_opengl_LinuxKeyboard_nSetDetectableKeyRepeat(JNIEnv *env, jclass unused, jlong display_ptr, jboolean set_enabled) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	Bool enabled = set_enabled == JNI_TRUE ? True : False;
	Bool result = XkbSetDetectableAutoRepeat(disp, enabled, NULL);
	return result == enabled ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxKeyboard_freeModifierMapping(JNIEnv *env, jclass unused, jlong mapping_ptr) {
	XModifierKeymap *modifier_map = (XModifierKeymap *)(intptr_t)mapping_ptr;
	XFreeModifiermap(modifier_map);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxKeyboard_getMaxKeyPerMod(JNIEnv *env, jclass unsused, jlong mapping_ptr) {
	XModifierKeymap *modifier_map = (XModifierKeymap *)(intptr_t)mapping_ptr;
	return modifier_map->max_keypermod;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxKeyboard_lookupModifierMap(JNIEnv *env, jclass unused, jlong mapping_ptr, jint index) {
	XModifierKeymap *modifier_map = (XModifierKeymap *)(intptr_t)mapping_ptr;
	KeyCode key_code = modifier_map->modifiermap[index];
	return key_code;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxKeyboard_keycodeToKeySym(JNIEnv *env, jclass unused, jlong display_ptr, jint key_code) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	KeySym key_sym = XKeycodeToKeysym(disp, key_code, 0);
	return key_sym;
}
