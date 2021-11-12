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

#define ERR_MSG_SIZE 1024

typedef struct {
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long input_mode;
	unsigned long status;
} MotifWmHints;

#define MWM_HINTS_DECORATIONS   (1L << 1)

static EGLSurface egl_window = EGL_NO_SURFACE;

static Colormap cmap;
static int current_depth;

static Visual *current_visual;

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
	return org_lwjgl_LinuxSysImplementation_JNI_VERSION;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetDefaultScreen(JNIEnv *env, jclass unused, jlong display_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	return XDefaultScreen(disp);
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nInternAtom(JNIEnv *env, jclass unused, jlong display_ptr, jstring atom_name_obj, jboolean only_if_exists) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	char *atom_name = GetStringNativeChars(env, atom_name_obj);
	if (atom_name == NULL)
		return 0;
	Atom atom = XInternAtom(disp, atom_name, only_if_exists ? True : False);
	free(atom_name);
	return atom;
}

static bool isLegacyFullscreen(jint window_mode) {
	return window_mode == org_lwjgl_opengl_LinuxDisplay_FULLSCREEN_LEGACY;
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

static void destroyWindow(JNIEnv *env, Display *disp, Window window) {
	if (glx_window != None) {
		lwjgl_glXDestroyWindow(disp, glx_window);
		glx_window = None;
	}
	XDestroyWindow(disp, window);
	XFreeColormap(disp, cmap);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nReshape(JNIEnv *env, jclass clazz, jlong display, jlong window_ptr, jint x, jint y, jint width, jint height) {
	Display *disp = (Display *)(intptr_t)display;
	Window window = (Window)window_ptr;
	XMoveWindow(disp, window, x, y);
	XResizeWindow(disp, window, width, height);
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_getRootWindow(JNIEnv *env, jclass clazz, jlong display, jint screen) {
	Display *disp = (Display *)(intptr_t)display;
	return RootWindow(disp, screen);
}

static Window getCurrentWindow(JNIEnv *env, jlong display_ptr, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;

	Window parent = (Window)window_ptr;
	Window win, root;

	Window *children;
	unsigned int nchildren;

	do {
		win = parent;

		if (XQueryTree(disp, win, &root, &parent, &children, &nchildren) == 0) {
			throwException(env, "XQueryTree failed");
			return 0;
		}

		if (children != NULL) XFree(children);
	} while (parent != root);

	return win;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetX(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = getCurrentWindow(env, display_ptr, window_ptr);

	XWindowAttributes win_attribs;
	XGetWindowAttributes(disp, win, &win_attribs);

	return win_attribs.x;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetY(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = getCurrentWindow(env, display_ptr, window_ptr);

	XWindowAttributes win_attribs;
	XGetWindowAttributes(disp, win, &win_attribs);

	return win_attribs.y;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetWidth(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = (Window)window_ptr;
	XWindowAttributes win_attribs;

	XGetWindowAttributes(disp, win, &win_attribs);

	return win_attribs.width;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetHeight(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = (Window)window_ptr;
	XWindowAttributes win_attribs;

	XGetWindowAttributes(disp, win, &win_attribs);

	return win_attribs.height;
}

static void updateWindowHints(JNIEnv *env, Display *disp, Window window) {
	XWMHints* win_hints = XAllocWMHints();
	if (win_hints == NULL) {
		throwException(env, "XAllocWMHints failed");
		return;
	}

	win_hints->flags = InputHint;
	win_hints->input = True;

	XSetWMHints(disp, window, win_hints);
	XFree(win_hints);
	XFlush(disp);
}

static void updateWindowBounds(Display *disp, Window win, int x, int y, int width, int height, jboolean position, jboolean resizable) {
	XSizeHints *window_hints = XAllocSizeHints();

	if (position) {
		window_hints->flags |= PPosition;
		window_hints->x = x;
		window_hints->y = y;
	}

	if (!resizable) {
		window_hints->flags |= PMinSize | PMaxSize;
		window_hints->min_width = width;
		window_hints->max_width = width;
		window_hints->min_height = height;
		window_hints->max_height = height;
	}

	XSetWMNormalHints(disp, win, window_hints);
	XFree(window_hints);
}

static Window createWindow(JNIEnv* env, Display *disp, int screen, jint window_mode, BoatPeerInfo *peer_info, int x, int y, int width, int height, long parent_handle, jboolean resizable) {
	Window parent = (Window)parent_handle;
	Window win;
	XSetWindowAttributes attribs;
	int attribmask;

	XVisualInfo *vis_info = getVisualInfoFromPeerInfo(env, peer_info);
	if (vis_info == NULL)
		return false;
	cmap = XCreateColormap(disp, parent, vis_info->visual, AllocNone);
	attribs.colormap = cmap;
	attribs.border_pixel = 0;
	attribs.event_mask = ExposureMask | FocusChangeMask | VisibilityChangeMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask| EnterWindowMask | LeaveWindowMask;
	attribmask = CWColormap | CWEventMask | CWBorderPixel;
	if (isLegacyFullscreen(window_mode)) {
		attribmask |= CWOverrideRedirect;
		attribs.override_redirect = True;
	}
	win = XCreateWindow(disp, parent, x, y, width, height, 0, vis_info->depth, InputOutput, vis_info->visual, attribmask, &attribs);

	current_depth = vis_info->depth;
	current_visual = vis_info->visual;

	XFree(vis_info);
//	printfDebugJava(env, "Created window");

	if (RootWindow(disp, screen) == parent_handle) { // only set hints when Display.setParent isn't used
		updateWindowBounds(disp, win, x, y, width, height, JNI_TRUE, resizable);
		updateWindowHints(env, disp, win);
	}

#define NUM_ATOMS 1
	Atom protocol_atoms[NUM_ATOMS] = {XInternAtom(disp, "WM_DELETE_WINDOW", False)/*, XInternAtom(disp, "WM_TAKE_FOCUS", False)*/};
	XSetWMProtocols(disp, win, protocol_atoms, NUM_ATOMS);
	if (window_mode == org_lwjgl_opengl_LinuxDisplay_FULLSCREEN_NETWM) {
		Atom fullscreen_atom = XInternAtom(disp, "_NET_WM_STATE_FULLSCREEN", False);
		XChangeProperty(disp, win, XInternAtom(disp, "_NET_WM_STATE", False),
						XInternAtom(disp, "ATOM", False), 32, PropModeReplace, (const unsigned char*)&fullscreen_atom, 1);
	}
	return win;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_reparentWindow(JNIEnv *env, jclass unused, jlong display, jlong window_ptr, jlong parent_ptr, jint x, jint y) {
	Display *disp = (Display *)(intptr_t)display;
	Window window = (Window)window_ptr;
	Window parent = (Window)parent_ptr;
	XReparentWindow(disp, window, parent, x, y);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_mapRaised(JNIEnv *env, jclass unused, jlong display, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display;
	Window window = (Window)window_ptr;
	XMapRaised(disp, window);
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_getParentWindow(JNIEnv *env, jclass unused, jlong display, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display;
	Window window = (Window)window_ptr;
	Window root, parent;
	Window *children;
	unsigned int nchildren;
	if (XQueryTree(disp, window, &root, &parent, &children, &nchildren) == 0) {
		throwException(env, "XQueryTree failed");
		return None;
	}
	if (children != NULL)
		XFree(children);
	return parent;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_getChildCount(JNIEnv *env, jclass unused, jlong display, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display;
	Window window = (Window)window_ptr;
	Window root, parent;
	Window *children;
	unsigned int nchildren;
	if (XQueryTree(disp, window, &root, &parent, &children, &nchildren) == 0) {
		throwException(env, "XQueryTree failed");
		return None;
	}
	if (children != NULL)
		XFree(children);

	return nchildren;
}

JNIEXPORT jboolean JNICALL Java_org_lwjgl_opengl_LinuxDisplay_hasProperty(JNIEnv *env, jclass unusued, jlong display, jlong window_ptr, jlong property_ptr) {
	Display *disp = (Display *)(intptr_t)display;
	Window window = (Window)window_ptr;
	Atom property = (Atom)property_ptr;
	int num_props;
	Atom *properties = XListProperties(disp, window, &num_props);
	if (properties == NULL)
		return JNI_FALSE;
	jboolean result = JNI_FALSE;
	for (int i = 0; i < num_props; i++) {
		if (properties[i] == property) {
			result = JNI_TRUE;
			break;
		}
	}
	XFree(properties);
	return result;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_BoatDisplay_nCreateWindow(JNIEnv *env, jclass clazz, jlong display, jint screen, jobject peer_info_handle, jobject mode, jint window_mode, jint x, jint y, jlong parent_handle, jboolean resizable) {
	Display *disp = (Display *)(intptr_t)display;
	X11PeerInfo *peer_info = (*env)->GetDirectBufferAddress(env, peer_info_handle);
	GLXFBConfig *fb_config = NULL;
	if (peer_info->glx13) {
		fb_config = getFBConfigFromPeerInfo(env, peer_info);
		if (fb_config == NULL)
			return 0;
	}
	jclass cls_displayMode = (*env)->GetObjectClass(env, mode);
	jfieldID fid_width = (*env)->GetFieldID(env, cls_displayMode, "width", "I");
	jfieldID fid_height = (*env)->GetFieldID(env, cls_displayMode, "height", "I");
	int width = (*env)->GetIntField(env, mode, fid_width);
	int height = (*env)->GetIntField(env, mode, fid_height);
	Window win = createWindow(env, disp, screen, window_mode, peer_info, x, y, width, height, parent_handle, resizable);
	if ((*env)->ExceptionOccurred(env)) {
		return 0;
	}
	if (peer_info->glx13) {
		glx_window = lwjgl_glXCreateWindow(disp, *fb_config, win, NULL);
		XFree(fb_config);
	}
	return win;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nSetWindowSize(JNIEnv *env, jclass clazz, jlong display, jlong window_ptr, jint width, jint height, jboolean resizable) {
	Display *disp = (Display *)(intptr_t)display;
	Window win = (Window)window_ptr;
	updateWindowBounds(disp, win, 0, 0, width, height, JNI_FALSE, resizable);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nDestroyWindow(JNIEnv *env, jclass clazz, jlong display, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display;
	Window window = (Window)window_ptr;
	destroyWindow(env, disp, window);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nUngrabKeyboard(JNIEnv *env, jclass unused, jlong display_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	return XUngrabKeyboard(disp, CurrentTime);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGrabKeyboard(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = (Window)window_ptr;
	return XGrabKeyboard(disp, win, False, GrabModeAsync, GrabModeAsync, CurrentTime);
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

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nDefineCursor(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr, jlong cursor_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = (Window)window_ptr;
	Cursor cursor = (Cursor)cursor_ptr;
	XDefineCursor(disp, win, cursor);
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nCreateBlankCursor(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = (Window)window_ptr;
	unsigned int best_width, best_height;
	if (XQueryBestCursor(disp, win, 1, 1, &best_width, &best_height) == 0) {
		throwException(env, "Could not query best cursor size");
		return false;
	}
	Pixmap mask = XCreatePixmap(disp, win, best_width, best_height, 1);
	XGCValues gc_values;
	gc_values.foreground = 0;
	GC gc = XCreateGC(disp, mask, GCForeground, &gc_values);
	XFillRectangle(disp, mask, gc, 0, 0, best_width, best_height);
	XFreeGC(disp, gc);
	XColor dummy_color;
	Cursor cursor = XCreatePixmapCursor(disp, mask, mask, &dummy_color, &dummy_color, 0, 0);
	XFreePixmap(disp, mask);
	return cursor;
}
