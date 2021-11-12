/*
 * Copyright (c) 2002-2010 LWJGL Project
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
package org.lwjgl.opengl;

/**
 * This is the Display implementation interface. Display delegates
 * to implementors of this interface. There is one DisplayImplementation
 * for each supported platform.
 * @author cosine
 */

import java.awt.Canvas;
import java.awt.event.FocusListener;
import java.awt.event.FocusEvent;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteOrder;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.lang.reflect.InvocationTargetException;

import org.lwjgl.BufferUtils;
import org.lwjgl.LWJGLException;
import org.lwjgl.LWJGLUtil;
import org.lwjgl.MemoryUtil;
import org.lwjgl.opengl.XRandR.Screen;
import org.lwjgl.opengles.EGL;

import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.List;

final class BoatDisplay implements DisplayImplementation {
	/* X11 constants */
	public static final int CurrentTime = 0;
	public static final int GrabSuccess = 0;
	public static final int AutoRepeatModeOff  = 0;
	public static final int AutoRepeatModeOn = 1;
	public static final int AutoRepeatModeDefault = 2;
	public static final int None = 0;

	private static final int KeyPressMask = 1 << 0;
	private static final int KeyReleaseMask = 1 << 1;
	private static final int ButtonPressMask = 1 << 2;
	private static final int ButtonReleaseMask = 1 << 3;

	private static final int NotifyAncestor = 0;
	private static final int NotifyNonlinear = 3;
	private static final int NotifyPointer = 5;
	private static final int NotifyPointerRoot = 6;
	private static final int NotifyDetailNone = 7;

	private static final int SetModeInsert = 0;
	private static final int SaveSetRoot = 1;
	private static final int SaveSetUnmap = 1;
	
	private static final int X_SetInputFocus = 42;

	/** Window mode enum */
	private static final int FULLSCREEN_LEGACY = 1;
	private static final int WINDOWED = 3;

	/** Current window mode */
	private static int current_window_mode = WINDOWED;

	/** Display mode switching API */
	private static final int XRANDR = 10;
	private static final int XF86VIDMODE = 11;
	private static final int NONE = 12;

	/** Current X11 Display pointer */
	private static long display;
	private static long current_window;
	private static long saved_error_handler;

	private static int display_connection_usage_count;

	/** Event buffer */
	private final LinuxEvent event_buffer = new LinuxEvent();
	private final LinuxEvent tmp_event_buffer = new LinuxEvent();

	/** Current mode swithcing API */
	private int current_displaymode_extension = NONE;

	/** Atom used for the pointer warp messages */
	private long delete_atom;

	private PeerInfo peer_info;

	/** Saved mode to restore with */
	private DisplayMode saved_mode;
	private DisplayMode current_mode;


	private boolean keyboard_grabbed;
	private boolean pointer_grabbed;
	private boolean input_released;
	private boolean grab;
	private boolean focused;
	private boolean minimized;
	private boolean dirty;
	private boolean close_requested;
	private long current_cursor;
	private long blank_cursor;
	private boolean mouseInside = true;
	private boolean resizable;
	private boolean resized;
	
	private int window_x;
	private int window_y;
	private int window_width;
	private int window_height;
	
	private Canvas parent;
	private long parent_window;
	private static boolean xembedded;
	private long parent_proxy_focus_window;
	private boolean parent_focused;
	private boolean parent_focus_changed;
	private long last_window_focus = 0;

	private LinuxKeyboard keyboard;
	private LinuxMouse mouse;

	private final FocusListener focus_listener = new FocusListener() {
		public void focusGained(FocusEvent e) {
			synchronized (GlobalLock.lock) {
				parent_focused = true;
				parent_focus_changed = true;
			}
		}
		public void focusLost(FocusEvent e) {
			synchronized (GlobalLock.lock) {
				parent_focused = false;
				parent_focus_changed = true;
			}
		}
	};

	private static int getBestDisplayModeExtension() {
		int result;
		if (isXrandrSupported()) {
			LWJGLUtil.log("Using Xrandr for display mode switching");
			result = XRANDR;
		} else if (isXF86VidModeSupported()) {
			LWJGLUtil.log("Using XF86VidMode for display mode switching");
			result = XF86VIDMODE;
		} else {
			LWJGLUtil.log("No display mode extensions available");
			result = NONE;
		}
		return result;
	}

	private static boolean isXrandrSupported() {
		if (Display.getPrivilegedBoolean("LWJGL_DISABLE_XRANDR"))
			return false;
		try {
			incDisplay();
			try {
				return nIsXrandrSupported(getDisplay());
			} finally {
				decDisplay();
			}
		} catch (LWJGLException e) {
			LWJGLUtil.log("Got exception while querying Xrandr support: " + e);
			return false;
		}
	}
	private static native boolean nIsXrandrSupported(long display) throws LWJGLException;

	private static boolean isXF86VidModeSupported() {
		try {
			incDisplay();
			try {
				return nIsXF86VidModeSupported(getDisplay());
			} finally {
				decDisplay();
			}
		} catch (LWJGLException e) {
			LWJGLUtil.log("Got exception while querying XF86VM support: " + e);
			return false;
		}
	}
	private static native boolean nIsXF86VidModeSupported(long display) throws LWJGLException;

	/**
	 * increment and decrement display usage.
	 */
	static void incDisplay() throws LWJGLException {
		if (display_connection_usage_count == 0) {
			try {
				// TODO: Can we know if we're on desktop or ES?
				GLContext.loadOpenGLLibrary();
				org.lwjgl.opengles.GLContext.loadOpenGLLibrary();
			} catch (Throwable t) {
			}
			saved_error_handler = setErrorHandler();
			display = openDisplay();
//			synchronize(display, true);
		}
		display_connection_usage_count++;
	}
	private static native int callErrorHandler(long handler, long display, long error_ptr);
	private static native long setErrorHandler();
	private static native long resetErrorHandler(long handler);
	private static native void synchronize(long display, boolean synchronize);

	private static int globalErrorHandler(long display, long event_ptr, long error_display, long serial, long error_code, long request_code, long minor_code) throws LWJGLException {
		if (xembedded && request_code == X_SetInputFocus) return 0; // ignore X error in xembeded mode to fix a browser issue when dragging or switching tabs 
		
		if (display == getDisplay()) {
			String error_msg = getErrorText(display, error_code);
			throw new LWJGLException("X Error - disp: 0x" + Long.toHexString(error_display) + " serial: " + serial + " error: " + error_msg + " request_code: " + request_code + " minor_code: " + minor_code);
		} else if (saved_error_handler != 0)
			return callErrorHandler(saved_error_handler, display, event_ptr);
		return 0;
	}
	private static native String getErrorText(long display, long error_code);

	static void decDisplay() {
		/*
		 * Some drivers (at least some versions of the radeon dri driver)
		 * don't like it when the display is closed and later re-opened,
		 * so we'll just let the singleton display connection leak.
		 */
/*		display_connection_usage_count--;
		if (display_connection_usage_count < 0)
			throw new InternalError("display_connection_usage_count < 0: " + display_connection_usage_count);
		if (display_connection_usage_count == 0) {
			closeDisplay(display);
			resetErrorHandler(saved_error_handler);
			display = 0;
			GLContext.unloadOpenGLLibrary();
		}*/
	}

	static native long openDisplay() throws LWJGLException;
	static native void closeDisplay(long display);

	private int getWindowMode(boolean fullscreen) throws LWJGLException {
		if (fullscreen) {
			LWJGLUtil.log("Using legacy mode for fullscreen window");
			return FULLSCREEN_LEGACY;
		} else
			return WINDOWED;
	}

	static long getDisplay() {
		if (display_connection_usage_count <= 0)
			throw new InternalError("display_connection_usage_count = " + display_connection_usage_count);
		return display;
	}

	static int getDefaultScreen() {
		return nGetDefaultScreen(getDisplay());
	}
	static native int nGetDefaultScreen(long display);

	static long getWindow() {
		return current_window;
	}

	private void ungrabKeyboard() {
		if (keyboard_grabbed) {
			nUngrabKeyboard(getDisplay());
			keyboard_grabbed = false;
		}
	}
	static native int nUngrabKeyboard(long display);

	private void grabKeyboard() {
		if (!keyboard_grabbed) {
			int res = nGrabKeyboard(getDisplay(), getWindow());
			if (res == GrabSuccess)
				keyboard_grabbed = true;
		}
	}
	static native int nGrabKeyboard(long display, long window);

	private void grabPointer() {
		if (!pointer_grabbed) {
			int result = nGrabPointer(getDisplay(), getWindow(), None);
			if (result == GrabSuccess) {
				pointer_grabbed = true;
			}
		}
	}
	static native int nGrabPointer(long display, long window, long cursor);

	private void ungrabPointer() {
		if (pointer_grabbed) {
			pointer_grabbed = false;
			nUngrabPointer(getDisplay());
		}
	}
	static native int nUngrabPointer(long display);

	private static boolean isFullscreen() {
		return current_window_mode == FULLSCREEN_LEGACY;
	}

	private boolean shouldGrab() {
		return !input_released && grab && mouse != null;
	}

	private void updatePointerGrab() {
		if (isFullscreen() || shouldGrab()) {
			grabPointer();
		} else {
			ungrabPointer();
		}
		updateCursor();
	}

	private void updateCursor() {
		long cursor;
		if (shouldGrab()) {
			cursor = blank_cursor;
		} else {
			cursor = current_cursor;
		}
		nDefineCursor(getDisplay(), getWindow(), cursor);
	}
	private static native void nDefineCursor(long display, long window, long cursor_handle);

	private static boolean isLegacyFullscreen() {
		return current_window_mode == FULLSCREEN_LEGACY;
	}

	private void updateKeyboardGrab() {
		if (isLegacyFullscreen())
			grabKeyboard();
		else
			ungrabKeyboard();
	}

	public void createWindow(final DrawableLWJGL drawable, DisplayMode mode, Canvas parent, int x, int y) throws LWJGLException {
		try {
			incDisplay();
			try {
				if ( drawable instanceof DrawableGLES )
					peer_info = new BoatDisplayPeerInfo();

				ByteBuffer handle = peer_info.lockAndGetHandle();
				try {
					current_window_mode = getWindowMode(Display.isFullscreen());
					
					this.parent = parent;
					parent_window = parent != null ? getHandle(parent) : getRootWindow(getDisplay(), getDefaultScreen());
					resizable = Display.isResizable();
					resized = false;
					window_x = x;
					window_y = y;
					window_width = mode.getWidth();
					window_height = mode.getHeight();

                                        // overwrite arguments x and y - superclass always uses 0,0 for fullscreen windows
                                        // use the coordinates of XRandRs primary screen instead
                                        // this is required to let the fullscreen window appear on the primary screen
                                        if (mode.isFullscreenCapable()  && current_displaymode_extension == XRANDR) {
                                            Screen primaryScreen = XRandR.DisplayModetoScreen(Display.getDisplayMode());
                                            x = primaryScreen.xPos;
                                            y = primaryScreen.yPos;
                                        }

					current_window = nCreateWindow(getDisplay(), getDefaultScreen(), handle, mode, current_window_mode, x, y, parent_window, resizable);
					
					mapRaised(getDisplay(), current_window);
					xembedded = parent != null && isAncestorXEmbedded(parent_window);
					blank_cursor = createBlankCursor();
					current_cursor = None;
					focused = false;
					input_released = false;
					pointer_grabbed = false;
					keyboard_grabbed = false;
					close_requested = false;
					grab = false;
					minimized = false;
					dirty = true;

					if ( drawable instanceof DrawableGLES )
						((DrawableGLES)drawable).initialize(current_window, getDisplay(), EGL.EGL_WINDOW_BIT, (org.lwjgl.opengles.PixelFormat)drawable.getPixelFormat());

					if (parent != null) {
						parent.addFocusListener(focus_listener);
						parent_focused = parent.isFocusOwner();
						parent_focus_changed = true;
					}
				} finally {
					peer_info.unlock();
				}
			} catch (LWJGLException e) {
				decDisplay();
				throw e;
			}
		}
	}
	private static native long nCreateWindow(long display, int screen, ByteBuffer peer_info_handle, DisplayMode mode, int window_mode, int x, int y, boolean undecorated, long parent_handle, boolean resizable) throws LWJGLException;
	private static native long getRootWindow(long display, int screen);
	private static native boolean hasProperty(long display, long window, long property);
	private static native long getParentWindow(long display, long window) throws LWJGLException;
	private static native int getChildCount(long display, long window) throws LWJGLException;
	private static native void mapRaised(long display, long window);
	private static native void reparentWindow(long display, long window, long parent, int x, int y);
	private static native void nSetWindowSize(long display, long window, int width, int height, boolean resizable);
	private static native int nGetX(long display, long window);
	private static native int nGetY(long display, long window);
	private static native int nGetWidth(long display, long window);
	private static native int nGetHeight(long display, long window);

	private static boolean isAncestorXEmbedded(long window) throws LWJGLException {
		long xembed_atom = internAtom("_XEMBED_INFO", true);
		if (xembed_atom != None) {
			long w = window;
			while (w != None) {
				if (hasProperty(getDisplay(), w, xembed_atom))
					return true;
				w = getParentWindow(getDisplay(), w);
			}
		}
		return false;
	}

	private static long getHandle(Canvas parent) throws LWJGLException {
		AWTCanvasImplementation awt_impl = AWTGLCanvas.createImplementation();
		LinuxPeerInfo parent_peer_info = (LinuxPeerInfo)awt_impl.createPeerInfo(parent, null, null);
		ByteBuffer parent_peer_info_handle = parent_peer_info.lockAndGetHandle();
		try {
			return parent_peer_info.getDrawable();
		} finally {
			parent_peer_info.unlock();
		}
	}

	private void updateInputGrab() {
		updatePointerGrab();
		updateKeyboardGrab();
	}

	public void destroyWindow() {
		try {
			if (parent != null) {
				parent.removeFocusListener(focus_listener);
			}
			try {
				setNativeCursor(null);
			} catch (LWJGLException e) {
				LWJGLUtil.log("Failed to reset cursor: " + e.getMessage());
			}
			nDestroyCursor(getDisplay(), blank_cursor);
			blank_cursor = None;
			ungrabKeyboard();
			nDestroyWindow(getDisplay(), getWindow());
			decDisplay();
		}
	}
	static native void nDestroyWindow(long display, long window);

	public void switchDisplayMode(DisplayMode mode) throws LWJGLException {
		try {
			switchDisplayModeOnTmpDisplay(mode);
			current_mode = mode;
		}
	}

	private void switchDisplayModeOnTmpDisplay(DisplayMode mode) throws LWJGLException {
                if (current_displaymode_extension == XRANDR) {
                        // let Xrandr set the display mode
                        XRandR.setConfiguration(false, XRandR.DisplayModetoScreen(mode));
                } else {
                        incDisplay();
                        try {
                                nSwitchDisplayMode(getDisplay(), getDefaultScreen(), current_displaymode_extension, mode);
                        } finally {
                                decDisplay();
                        }
                }
	}
	private static native void nSwitchDisplayMode(long display, int screen, int extension, DisplayMode mode) throws LWJGLException;

	private static long internAtom(String atom_name, boolean only_if_exists) throws LWJGLException {
		incDisplay();
		try {
			return nInternAtom(getDisplay(), atom_name, only_if_exists);
		} finally {
			decDisplay();
		}
	}
	static native long nInternAtom(long display, String atom_name, boolean only_if_exists);

	public void resetDisplayMode() {
		try {
			if( current_displaymode_extension == XRANDR )
			{
				AccessController.doPrivileged(new PrivilegedAction<Object>() {
					public Object run() {
						XRandR.restoreConfiguration();
						return null;
					}
				});
			}
			else
			{
				switchDisplayMode(saved_mode);
			}
		} catch (LWJGLException e) {
			LWJGLUtil.log("Caught exception while resetting mode: " + e);
		}
	}

	public int getGammaRampLength() {
		return 0;
	}

	public void setGammaRamp(FloatBuffer gammaRamp) throws LWJGLException {
		throw new LWJGLException("No gamma ramp support on Boat");
	}

	public String getAdapter() {
		return null;
	}

	public String getVersion() {
		return null;
	}

	public DisplayMode init() throws LWJGLException {
		try {
			delete_atom = internAtom("WM_DELETE_WINDOW", false);
			current_displaymode_extension = getBestDisplayModeExtension();
			if (current_displaymode_extension == NONE)
				throw new LWJGLException("No display mode extension is available");
			DisplayMode[] modes = getAvailableDisplayModes();
			if (modes == null || modes.length == 0)
				throw new LWJGLException("No modes available");
			switch (current_displaymode_extension) {
				case XRANDR:
					saved_mode = AccessController.doPrivileged(new PrivilegedAction<DisplayMode>() {
						public DisplayMode run() {
							XRandR.saveConfiguration();
                                                        return XRandR.ScreentoDisplayMode(XRandR.getConfiguration());
						}
					});
					break;
				case XF86VIDMODE:
					saved_mode = modes[0];
					break;
				default:
					throw new LWJGLException("Unknown display mode extension: " + current_displaymode_extension);
			}
			current_mode = saved_mode;
			return saved_mode;
		}
	}

	private static DisplayMode getCurrentXRandrMode() throws LWJGLException {
		try {
			incDisplay();
			try {
				return nGetCurrentXRandrMode(getDisplay(), getDefaultScreen());
			} finally {
				decDisplay();
			}
		}
	}

	/** Assumes extension == XRANDR */
	private static native DisplayMode nGetCurrentXRandrMode(long display, int screen) throws LWJGLException;

	public void setTitle(String title) {
	}

	public boolean isCloseRequested() {
		boolean result = close_requested;
		close_requested = false;
		return result;
	}

	public boolean isVisible() {
		return !minimized;
	}

	public boolean isActive() {
		return focused || isLegacyFullscreen();
	}

	public boolean isDirty() {
		boolean result = dirty;
		dirty = false;
		return result;
	}

	public PeerInfo createPeerInfo(PixelFormat pixel_format, ContextAttribs attribs) throws LWJGLException {
		peer_info = new LinuxDisplayPeerInfo(pixel_format);
		return peer_info;
	}

	private void relayEventToParent(LinuxEvent event_buffer, int event_mask) {
		tmp_event_buffer.copyFrom(event_buffer);
		tmp_event_buffer.setWindow(parent_window);
		tmp_event_buffer.sendEvent(getDisplay(), parent_window, true, event_mask);
	}

	private void relayEventToParent(LinuxEvent event_buffer) {
		if (parent == null)
			return;
		switch (event_buffer.getType()) {
			case LinuxEvent.KeyPress:
				relayEventToParent(event_buffer, KeyPressMask);
				break;
			case LinuxEvent.KeyRelease:
				relayEventToParent(event_buffer, KeyPressMask);
				break;
			case LinuxEvent.ButtonPress:
				if (xembedded || !focused) relayEventToParent(event_buffer, KeyPressMask);
				break;
			case LinuxEvent.ButtonRelease:
				if (xembedded || !focused) relayEventToParent(event_buffer, KeyPressMask);
				break;
			default:
				break;
		}
	}

	private void processEvents() {
		while (LinuxEvent.getPending(getDisplay()) > 0) {
			event_buffer.nextEvent(getDisplay());
			long event_window = event_buffer.getWindow();
			relayEventToParent(event_buffer);
			if (event_window != getWindow() || event_buffer.filterEvent(event_window) ||
					(mouse != null && mouse.filterEvent(grab, shouldWarpPointer(), event_buffer)) ||
					 (keyboard != null && keyboard.filterEvent(event_buffer)))
				continue;
			switch (event_buffer.getType()) {
				case LinuxEvent.FocusIn:
					setFocused(true, event_buffer.getFocusDetail());
					break;
				case LinuxEvent.FocusOut:
					setFocused(false, event_buffer.getFocusDetail());
					break;
				case LinuxEvent.ClientMessage:
					if ((event_buffer.getClientFormat() == 32) && (event_buffer.getClientData(0) == delete_atom))
						close_requested = true;
					break;
				case LinuxEvent.MapNotify:
					dirty = true;
					minimized = false;
					break;
				case LinuxEvent.UnmapNotify:
					dirty = true;
					minimized = true;
					break;
				case LinuxEvent.Expose:
					dirty = true;
					break;
				case LinuxEvent.ConfigureNotify:
					int x = nGetX(getDisplay(), getWindow());
					int y = nGetY(getDisplay(), getWindow());
					
					int width = nGetWidth(getDisplay(), getWindow());
					int height = nGetHeight(getDisplay(), getWindow());
					
					window_x = x;
					window_y = y;
					
					if (window_width != width || window_height != height) {
						resized = true;
						window_width = width;
						window_height = height;
					}
					
					break;
				case LinuxEvent.EnterNotify:
					mouseInside = true;
					break;
				case LinuxEvent.LeaveNotify:
					mouseInside = false;
					break;
				default:
					break;
			}
		}
	}

	public void update() {
		try {
			processEvents();
		}
	}

	public void reshape(int x, int y, int width, int height) {
		try {
			nReshape(getDisplay(), getWindow(), x, y, width, height);
		}
	}
	private static native void nReshape(long display, long window, int x, int y, int width, int height);

	public DisplayMode[] getAvailableDisplayModes() throws LWJGLException {
		try {
                        incDisplay();
                        if (current_displaymode_extension == XRANDR) {
                                // nGetAvailableDisplayModes cannot be trusted. Use it only for bitsPerPixel
                                DisplayMode[] nDisplayModes = nGetAvailableDisplayModes(getDisplay(), getDefaultScreen(), current_displaymode_extension);
                                int bpp = 24;
                                if (nDisplayModes.length > 0) {
                                    bpp = nDisplayModes[0].getBitsPerPixel();
                                }
                                // get the resolutions and frequencys from XRandR
                                Screen[] resolutions = XRandR.getResolutions(XRandR.getScreenNames()[0]);
                                DisplayMode[] modes = new DisplayMode[resolutions.length];
                                for (int i = 0; i < modes.length; i++) {
                                    modes[i] = new DisplayMode(resolutions[i].width, resolutions[i].height, bpp, resolutions[i].freq);
                                }
                                return modes;
                        } else {
                                try {
                                        DisplayMode[] modes = nGetAvailableDisplayModes(getDisplay(), getDefaultScreen(), current_displaymode_extension);
                                        return modes;
                                } finally {
                                        decDisplay();
                                }
                        }
		}
	}
	private static native DisplayMode[] nGetAvailableDisplayModes(long display, int screen, int extension) throws LWJGLException;

	/* Mouse */
	public boolean hasWheel() {
		return true;
	}

	public int getButtonCount() {
		return mouse.getButtonCount();
	}

	public void createMouse() throws LWJGLException {
		try {
			mouse = new LinuxMouse(getDisplay(), getWindow(), getWindow());
		}
	}

	public void destroyMouse() {
		mouse = null;
		updateInputGrab();
	}

	public void pollMouse(IntBuffer coord_buffer, ByteBuffer buttons) {
		try {
			mouse.poll(grab, coord_buffer, buttons);
		}
	}

	public void readMouse(ByteBuffer buffer) {
		try {
			mouse.read(buffer);
		}
	}

	public void setCursorPosition(int x, int y) {
		try {
			mouse.setCursorPosition(x, y);
		}
	}

	/**
	 * This method will check if the parent window is active when running
	 * in xembed mode. Every xembed embedder window has a focus proxy
	 * window that recieves all the input. This method will test whether
	 * the provided window handle is the focus proxy, if so it will get its
	 * parent window and then test whether this is an ancestor to our
	 * current_window. If so then parent window is active.
	 *
	 * @param window - the window handle to test
	 */
	private boolean isParentWindowActive(long window) {
		try {
			// parent window already active as window is current_window
			if (window == current_window) return true;

			// xembed focus proxy will have no children
			if (getChildCount(getDisplay(), window) != 0) return false;

			// get parent, will be xembed embedder window and ancestor of current_window
			long parent_window = getParentWindow(getDisplay(), window);

			// parent must not be None
			if (parent_window == None) return false;

			// scroll current_window's ancestors to find parent_window
			long w = current_window;

			while (w != None) {
				w = getParentWindow(getDisplay(), w);
				if (w == parent_window) {
					parent_proxy_focus_window = window; // save focus proxy window
					return true;
				}
			}
		} catch (LWJGLException e) {
			LWJGLUtil.log("Failed to detect if parent window is active: " + e.getMessage());
			return true; // on failure assume still active
		}

		return false; // failed to find an active parent window
	}

	private void setFocused(boolean got_focus, int focus_detail) {
		if (focused == got_focus || focus_detail == NotifyDetailNone || focus_detail == NotifyPointer || focus_detail == NotifyPointerRoot || xembedded)
			return;
		focused = got_focus;

		if (focused) {
			acquireInput();
		}
		else {
			releaseInput();
		}
	}

	private void releaseInput() {
		if (isLegacyFullscreen() || input_released)
			return;
		if ( keyboard != null )
			keyboard.releaseAll();
		input_released = true;
		updateInputGrab();
	}

	private void acquireInput() {
		if (isLegacyFullscreen() || !input_released)
			return;
		input_released = false;
		updateInputGrab();
	}

	public void grabMouse(boolean new_grab) {
		try {
			if (new_grab != grab) {
				grab = new_grab;
				updateInputGrab();
				mouse.changeGrabbed(grab, shouldWarpPointer());
			}
		}
	}

	private boolean shouldWarpPointer() {
		return pointer_grabbed && shouldGrab();
	}

	public int getNativeCursorCapabilities() {
		try {
			incDisplay();
			try {
				return nGetNativeCursorCapabilities(getDisplay());
			} finally {
				decDisplay();
			}
		} catch (LWJGLException e) {
			throw new RuntimeException(e);
		}
	}
	private static native int nGetNativeCursorCapabilities(long display) throws LWJGLException;

	public void setNativeCursor(Object handle) throws LWJGLException {
		current_cursor = getCursorHandle(handle);
		try {
			updateCursor();
		}
	}

	public int getMinCursorSize() {
		try {
			incDisplay();
			try {
				return nGetMinCursorSize(getDisplay(), getWindow());
			} finally {
				decDisplay();
			}
		} catch (LWJGLException e) {
			LWJGLUtil.log("Exception occurred in getMinCursorSize: " + e);
			return 0;
		}
	}
	private static native int nGetMinCursorSize(long display, long window);

	public int getMaxCursorSize() {
		try {
			incDisplay();
			try {
				return nGetMaxCursorSize(getDisplay(), getWindow());
			} finally {
				decDisplay();
			}
		} catch (LWJGLException e) {
			LWJGLUtil.log("Exception occurred in getMaxCursorSize: " + e);
			return 0;
		}
	}
	private static native int nGetMaxCursorSize(long display, long window);

	/* Keyboard */
	public void createKeyboard() throws LWJGLException {
		try {
			keyboard = new LinuxKeyboard(getDisplay(), getWindow());
		}
	}

	public void destroyKeyboard() {
		try {
			keyboard.destroy(getDisplay());
			keyboard = null;
		}
	}

	public void pollKeyboard(ByteBuffer keyDownBuffer) {
		try {
			keyboard.poll(keyDownBuffer);
		}
	}

	public void readKeyboard(ByteBuffer buffer) {
		try {
			keyboard.read(buffer);
		}
	}

	private static native long nCreateCursor(long display, int width, int height, int xHotspot, int yHotspot, int numImages, IntBuffer images, int images_offset, IntBuffer delays, int delays_offset) throws LWJGLException;

	private static long createBlankCursor() {
		return nCreateBlankCursor(getDisplay(), getWindow());
	}
	static native long nCreateBlankCursor(long display, long window);

	public Object createCursor(int width, int height, int xHotspot, int yHotspot, int numImages, IntBuffer images, IntBuffer delays) throws LWJGLException {
		try {
			incDisplay();
			try {
				long cursor = nCreateCursor(getDisplay(), width, height, xHotspot, yHotspot, numImages, images, images.position(), delays, delays != null ? delays.position() : -1);
				return cursor;
			} catch (LWJGLException e) {
				decDisplay();
				throw e;
			}
		}
	}

	private static long getCursorHandle(Object cursor_handle) {
		return cursor_handle != null ? (Long)cursor_handle : None;
	}

	public void destroyCursor(Object cursorHandle) {
		try {
			nDestroyCursor(getDisplay(), getCursorHandle(cursorHandle));
			decDisplay();
		}
	}
	static native void nDestroyCursor(long display, long cursorHandle);

	public int getPbufferCapabilities() {
		try {
			incDisplay();
			try {
				return nGetPbufferCapabilities(getDisplay(), getDefaultScreen());
			} finally {
				decDisplay();
			}
		} catch (LWJGLException e) {
			LWJGLUtil.log("Exception occurred in getPbufferCapabilities: " + e);
			return 0;
		}
	}
	private static native int nGetPbufferCapabilities(long display, int screen);

	public boolean isBufferLost(PeerInfo handle) {
		return false;
	}

	public PeerInfo createPbuffer(int width, int height, PixelFormat pixel_format, ContextAttribs attribs,
			IntBuffer pixelFormatCaps,
			IntBuffer pBufferAttribs) throws LWJGLException {
		return new LinuxPbufferPeerInfo(width, height, pixel_format);
	}

	public void setPbufferAttrib(PeerInfo handle, int attrib, int value) {
		throw new UnsupportedOperationException();
	}

	public void bindTexImageToPbuffer(PeerInfo handle, int buffer) {
		throw new UnsupportedOperationException();
	}

	public void releaseTexImageFromPbuffer(PeerInfo handle, int buffer) {
		throw new UnsupportedOperationException();
	}

	/**
	 * Sets one or more icons for the Display.
	 * <ul>
	 * <li>On Windows you should supply at least one 16x16 icon and one 32x32.</li>
	 * <li>Linux (and similar platforms) expect one 32x32 icon.</li>
	 * <li>Mac OS X should be supplied one 128x128 icon</li>
	 * </ul>
	 * The implementation will use the supplied ByteBuffers with image data in RGBA and perform any conversions necessary for the specific platform.
	 *
	 * @param icons Array of icons in RGBA mode
	 * @return number of icons used.
	 */
	public int setIcon(ByteBuffer[] icons) {
		LWJGLUtil.log("Failed to set display icon: no support on Boat.");
		return 0;
	}

	public int getX() {
		return window_x;
	}

	public int getY() {
		return window_y;
	}
	
	public int getWidth() {
		return window_width;
	}

	public int getHeight() {
		return window_height;
	}

	public boolean isInsideWindow() {
		return mouseInside;
	}

	public void setResizable(boolean resizable) {
		if (this.resizable == resizable) {
			return;
		}
		
		this.resizable = resizable;
		nSetWindowSize(getDisplay(), getWindow(), window_width, window_height, resizable);
	}

	public boolean wasResized() {
		if (resized) {
			resized = false;
			return true;
		}
		
		return false;
	}

	public float getPixelScaleFactor() {
		return 1f;
	}
}
