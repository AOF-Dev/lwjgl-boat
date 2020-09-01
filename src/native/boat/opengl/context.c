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
 * Include file to access public window features
 *
 * @author elias_naur <elias_naur@users.sourceforge.net>
 * @version $Revision$
 */

#include <jni.h>

#include "extgl_egl.h"
#include "context.h"

#include <boat.h>


EGLConfig getEGLConfigFromPeerInfo(JNIEnv *env, BoatPeerInfo *peer_info) {
        /*
	EGLint attribs[] = {EGL_CONFIG_ID, peer_info->config.egl_config.config_id, 0};
	int num_elements;
	EGLConfig config;
	__android_log_print(ANDROID_LOG_ERROR, "Boat-LWJGL", "Trying to lwjgl_eglChooseConfig.");
	lwjgl_eglChooseConfig(peer_info->display, attribs, &config, 1, &num_elements);
	*/
	EGLConfig config = peer_info->config;
	if (config == NULL) {
		throwException(env, "Could not find EGL config from peer info");
		return NULL;
	}
	
	return config;
}
EGLConfig getVisualInfoFromPeerInfo(JNIEnv *env, BoatPeerInfo *peer_info) {

	return getEGLConfigFromPeerInfo(env, peer_info);

}
/*
static int convertToBPE(int bpp) {
	int bpe;
	switch (bpp) {
		case 0:
			bpe = 0;
			break;
		case 32:
		case 24:
			bpe = 8;
			break;
		case 16: // Fall through 
		default:
			bpe = 4;
			break;
	}
	return bpe;
}
*/
static EGLConfig chooseVisualEGLFromBPP(JNIEnv *env, EGLDisplay disp, int screen, jobject pixel_format, int bpp, int drawable_type, bool double_buffer) {
	/*
	jclass cls_pixel_format = (*env)->GetObjectClass(env, pixel_format);
	
	int alpha = (int)(*env)->GetIntField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "alpha", "I"));
	int depth = (int)(*env)->GetIntField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "depth", "I"));
	int stencil = (int)(*env)->GetIntField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "stencil", "I"));
	int samples = (int)(*env)->GetIntField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "samples", "I"));
	int colorSamples = (int)(*env)->GetIntField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "colorSamples", "I"));
	int num_aux_buffers = (int)(*env)->GetIntField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "num_aux_buffers", "I"));
	int accum_bpp = (int)(*env)->GetIntField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "accum_bpp", "I"));
	int accum_alpha = (int)(*env)->GetIntField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "accum_alpha", "I"));

	bool stereo = (bool)(*env)->GetBooleanField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "stereo", "Z"));
	bool sRGB = (bool)(*env)->GetBooleanField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "sRGB", "Z"));

	int bpe = convertToBPE(bpp);
	int accum_bpe = convertToBPE(accum_bpp);
	
	attrib_list_t attrib_list;
	initAttribList(&attrib_list);
	putAttrib(&attrib_list, GLX_RGBA);
	putAttrib(&attrib_list, GLX_DOUBLEBUFFER);
	putAttrib(&attrib_list, GLX_DEPTH_SIZE); putAttrib(&attrib_list, depth);
	putAttrib(&attrib_list, GLX_RED_SIZE); putAttrib(&attrib_list, bpe);
	putAttrib(&attrib_list, GLX_GREEN_SIZE); putAttrib(&attrib_list, bpe);
	putAttrib(&attrib_list, GLX_BLUE_SIZE); putAttrib(&attrib_list, bpe);
	putAttrib(&attrib_list, GLX_ALPHA_SIZE); putAttrib(&attrib_list, alpha);
	putAttrib(&attrib_list, GLX_STENCIL_SIZE); putAttrib(&attrib_list, stencil);
	putAttrib(&attrib_list, GLX_AUX_BUFFERS); putAttrib(&attrib_list, num_aux_buffers);
	putAttrib(&attrib_list, GLX_ACCUM_RED_SIZE); putAttrib(&attrib_list, accum_bpe);
	putAttrib(&attrib_list, GLX_ACCUM_GREEN_SIZE); putAttrib(&attrib_list, accum_bpe);
	putAttrib(&attrib_list, GLX_ACCUM_BLUE_SIZE); putAttrib(&attrib_list, accum_bpe);
	putAttrib(&attrib_list, GLX_ACCUM_ALPHA_SIZE); putAttrib(&attrib_list, accum_alpha);
	*/
	/*
	const EGLint attrib_list[] = {
	        EGL_SURFACE_TYPE, EGL_WINDOW_BIT, 
	        EGL_DEPTH_SIZE, depth,
	        EGL_RED_SIZE, bpe,
	        EGL_GREEN_SIZE, bpe,
	        EGL_BLUE_SIZE, bpe,
	        EGL_ALPHA_SIZE, alpha,
	        EGL_STENCIL_SIZE, stencil,
	        EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
	        EGL_SAMPLE_BUFFERS, 1,
	        EGL_SAMPLES, samples,
	        EGL_NONE
	};
	*/
	const EGLint attrib_list[] = {
	        EGL_SURFACE_TYPE, EGL_WINDOW_BIT, 
	        EGL_DEPTH_SIZE, 16,
	        EGL_RED_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 8,
	        EGL_ALPHA_SIZE, 0,
	        EGL_STENCIL_SIZE, 0,
	        EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
	        EGL_NONE
	};
	
	EGLConfig config;
	EGLint useless;
	
	lwjgl_eglChooseConfig(disp, attrib_list, &config, 1, &useless);
	return config;
}

EGLConfig chooseVisualEGL(JNIEnv *env, EGLDisplay disp, int screen, jobject pixel_format, bool use_display_bpp, int drawable_type, bool double_buffer) {
	jclass cls_pixel_format = (*env)->GetObjectClass(env, pixel_format);
	int bpp = (int)(*env)->GetIntField(env, pixel_format, (*env)->GetFieldID(env, cls_pixel_format, "bpp", "I"));
	return chooseVisualEGLFromBPP(env, disp, screen, pixel_format, bpp, drawable_type, double_buffer);
}

static void dumpVisualInfo(JNIEnv *env, EGLDisplay display, EGLConfig vis_info) {
	int alpha, depth, stencil, r, g, b;
	int sample_buffers = 0;
	int samples = 0;
	lwjgl_eglGetConfigAttrib(display, vis_info, EGL_RED_SIZE, &r);
	lwjgl_eglGetConfigAttrib(display, vis_info, EGL_GREEN_SIZE, &g);
	lwjgl_eglGetConfigAttrib(display, vis_info, EGL_BLUE_SIZE, &b);
	lwjgl_eglGetConfigAttrib(display, vis_info, EGL_ALPHA_SIZE, &alpha);
	lwjgl_eglGetConfigAttrib(display, vis_info, EGL_DEPTH_SIZE, &depth);
	lwjgl_eglGetConfigAttrib(display, vis_info, EGL_STENCIL_SIZE, &stencil);
	
	printfDebugJava(env, "Pixel format info: r = %d, g = %d, b = %d, a = %d, depth = %d, stencil = %d, sample buffers = %d, samples = %d", r, g, b, alpha, depth, stencil, sample_buffers, samples);
}

bool initPeerInfo(JNIEnv *env, jobject peer_info_handle, EGLDisplay display, int screen, jobject pixel_format, bool use_display_bpp, int drawable_type, bool double_buffered, bool force_glx13) {

        //EGLDisplay display = lwjgl_eglGetDispaly(disp);
        
	if ((*env)->GetDirectBufferCapacity(env, peer_info_handle) < sizeof(BoatPeerInfo)) {
		throwException(env, "Handle too small");
		return false;
	}
	BoatPeerInfo *peer_info = (*env)->GetDirectBufferAddress(env, peer_info_handle);
	if (!extgl_InitEGL(display)) {
		throwException(env, "Could not init EGL");
		return false;
	}
	
	EGLConfig config = chooseVisualEGL(env, display, screen, pixel_format, use_display_bpp, drawable_type, double_buffered);
	if (config == NULL) {
		throwException(env, "Could not choose EGL config");
		return false;
	}
		
	dumpVisualInfo(env, display, config);
	peer_info->config = config;
	
	peer_info->display = display;
	peer_info->drawable = EGL_NO_SURFACE;
	return true;
}
