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
 * @author elias_naur
 */

import java.awt.Canvas;

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

import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.List;

final class BoatDisplay implements DisplayImplementation {
	/* Boat constants */
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
	private static final int FULLSCREEN_NETWM = 2;
	private static final int WINDOWED = 3;

	/** Current window mode */
	private static int current_window_mode = WINDOWED;

	/** Display mode switching API */
	private static final int XRANDR = 10;
	private static final int XF86VIDMODE = 11;
	private static final int NONE = 12;
	private static final int BOAT = 13;

	/** Current Boat Display pointer */
	private static long display;
	private static long current_window;
	private static long saved_error_handler;

	private static int display_connection_usage_count;

	/** Event buffer */
	private final BoatEvent event_buffer = new BoatEvent();
	private final BoatEvent tmp_event_buffer = new BoatEvent();

	/** Current mode swithcing API */
	private int current_displaymode_extension = NONE;

	/** Atom used for the pointer warp messages */
	private long delete_atom;

	private PeerInfo peer_info;

	/** Saved gamma used to restore display settings */
	private ByteBuffer saved_gamma;
	private ByteBuffer current_gamma;

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

	private BoatKeyboard keyboard;
	private BoatMouse mouse;
	
	private String wm_class;

	private static int getBestDisplayModeExtension() {
	
		int result;
		
		LWJGLUtil.log("Using Boat for display mode switching");
		result = BOAT;
		return result;
	}

	/**
	 * increment and decrement display usage.
	 */
	static void incDisplay() throws LWJGLException {
		if (display_connection_usage_count == 0) {
			try {
				// TODO: Can we know if we're on desktop or ES?
				GLContext.loadOpenGLLibrary();
				//org.lwjgl.opengles.GLContext.loadOpenGLLibrary();
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
		//if (xembedded && request_code == X_SetInputFocus) return 0; // ignore X error in xembeded mode to fix a browser issue when dragging or switching tabs 
		
		if (display == getDisplay()) {
			String error_msg = getErrorText(display, error_code);
			throw new LWJGLException("Boat Error - disp: 0x" + Long.toHexString(error_display) + " serial: " + serial + " error: " + error_msg + " request_code: " + request_code + " minor_code: " + minor_code);
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
		
		LWJGLUtil.log("Using legacy mode for fullscreen window");
		return FULLSCREEN_LEGACY;
	}

	static long getDisplay() {
		if (display_connection_usage_count <= 0)
			throw new InternalError("display_connection_usage_count = " + display_connection_usage_count);
		return display;
	}

	static int getDefaultScreen() {
		return 0;
	}

	static long getWindow() {
		return current_window;
	}
        
	private void ungrabKeyboard() {
		if (keyboard_grabbed) {
			keyboard_grabbed = false;
		}
	}

	private void grabKeyboard() {
		if (!keyboard_grabbed) {
			keyboard_grabbed = true;
		}
	}
	
	private void grabPointer() {
		if (!pointer_grabbed) {
			int result = nGrabPointer(getDisplay(), getWindow(), None);
			if (result == GrabSuccess) {
				pointer_grabbed = true;
				// make sure we have a centered window
				if (isLegacyFullscreen()) {
					// do nothing
				}
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
		return true;
	}

	private boolean shouldGrab() {
		return !input_released && grab && mouse != null;
	}

	private void updatePointerGrab() {
		if (shouldGrab()) {
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
	}
	
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
		current_boat_display = this;
		
		
		try {
			incDisplay();
			try {
				if ( drawable instanceof DrawableGLES )
					peer_info = new BoatDisplayPeerInfo();

				ByteBuffer handle = peer_info.lockAndGetHandle();
				try {
					current_window_mode = getWindowMode(Display.isFullscreen());
					
					// Try to enable Lecagy FullScreen Support in Compiz, else
					// we may have trouble with stuff overlapping our fullscreen window.
					/*
					if ( current_window_mode != WINDOWED )
						Compiz.setLegacyFullscreenSupport(true);
					*/
					
					// Setting _MOTIF_WM_HINTS in fullscreen mode is problematic for certain window
					// managers. We do not set MWM_HINTS_DECORATIONS in fullscreen mode anymore,
					// unless org.lwjgl.opengl.Window.undecorated_fs has been specified.
					// See native/linux/org_lwjgl_opengl_Display.c, createWindow function.
					boolean undecorated = Display.getPrivilegedBoolean("org.lwjgl.opengl.Window.undecorated") || (current_window_mode != WINDOWED && Display.getPrivilegedBoolean("org.lwjgl.opengl.Window.undecorated_fs"));
					
					this.parent = null; //parent;
					parent_window = 0;   //parent != null ? getHandle(parent) : getRootWindow(getDisplay(), getDefaultScreen());
					resizable = false;//Display.isResizable();
					resized = false;
				

                                        // overwrite arguments x and y - superclass always uses 0,0 for fullscreen windows
                                        // use the coordinates of XRandRs primary screen instead
                                        // this is required to let the fullscreen window appear on the primary screen
                                        
					current_window = nCreateWindow(getDisplay(), getDefaultScreen(), handle, mode, current_window_mode, x, y, undecorated, parent_window, resizable);
					
					window_x = 0;
					window_y = 0;
					window_width = nGetWidth(getDisplay(), getWindow());
					window_height = nGetHeight(getDisplay(), getWindow());
					
					current_cursor = None;
					focused = false;
					input_released = false;
					pointer_grabbed = false;
					keyboard_grabbed = false;
					close_requested = false;
					grab = false;
					minimized = false;
					dirty = true;
                                        
				} finally {
					peer_info.unlock();
				}
			} catch (LWJGLException e) {
				decDisplay();
				throw e;
			}
		} finally {
			
		}
	}
	private static native long nCreateWindow(long display, int screen, ByteBuffer peer_info_handle, DisplayMode mode, int window_mode, int x, int y, boolean undecorated, long parent_handle, boolean resizable) throws LWJGLException;
	
	private static native int nGetX(long display, long window);
	private static native int nGetY(long display, long window);
	private static native int nGetWidth(long display, long window);
	private static native int nGetHeight(long display, long window);

	private void updateInputGrab() {
		updatePointerGrab();
		updateKeyboardGrab();
	}

	public void destroyWindow() {
	        current_boat_display = null;
		try {
			ungrabKeyboard();
			nDestroyWindow(getDisplay(), getWindow());
			decDisplay();
		} finally {
			
		}
	}
	static native void nDestroyWindow(long display, long window);

	public void switchDisplayMode(DisplayMode mode) throws LWJGLException {
	        
		try {
			switchDisplayModeOnTmpDisplay(mode);
			current_mode = mode;
		} finally {
			
		}
		
	}
        
        
	private void switchDisplayModeOnTmpDisplay(DisplayMode mode) throws LWJGLException {
                
                incDisplay();
                try {
                                
                        nSwitchDisplayMode(getDisplay(), getDefaultScreen(), current_displaymode_extension, mode);
                } finally {
                        decDisplay();
                }
                
	}
	
	private static native void nSwitchDisplayMode(long display, int screen, int extension, DisplayMode mode) throws LWJGLException;
        
	public void resetDisplayMode() {
	        
		try {
			switchDisplayMode(saved_mode);
		} catch (LWJGLException e) {
			LWJGLUtil.log("Caught exception while resetting mode: " + e);
		} finally {
			
		}
		
	}
        
	public int getGammaRampLength() {
		return 0;
	}
	
	public void setGammaRamp(FloatBuffer gammaRamp) throws LWJGLException {
		
	}

	public String getAdapter() {
		return null;
	}

	public String getVersion() {
		return null;
	}

	public DisplayMode init() throws LWJGLException {
	        
		try {
			current_displaymode_extension = getBestDisplayModeExtension();
			if (current_displaymode_extension == NONE)
				throw new LWJGLException("No display mode extension is available");
			DisplayMode[] modes = getAvailableDisplayModes();
			if (modes == null || modes.length == 0)
				throw new LWJGLException("No modes available");
			switch (current_displaymode_extension) {
				case BOAT:
					saved_mode = modes[0];
					break;
				default:
					throw new LWJGLException("Unknown display mode extension: " + current_displaymode_extension);
			}
			current_mode = saved_mode;
			saved_gamma = null;
			current_gamma = saved_gamma;
			return saved_mode;
		} finally {
			
		}
		
	}

	public void setTitle(String title) {
	        
	}
	
	public boolean isCloseRequested() {
		boolean result = close_requested;
		close_requested = false;
		return result;
	}

	public boolean isVisible() {
		return true;//!minimized;
	}

	public boolean isActive() {
		return true; //focused;
	}

	public boolean isDirty() {
		boolean result = dirty;
		dirty = false;
		return result;
	}

	public PeerInfo createPeerInfo(PixelFormat pixel_format, ContextAttribs attribs) throws LWJGLException {
		peer_info = new BoatDisplayPeerInfo(pixel_format);
		return peer_info;
	}
	
	private static BoatDisplay current_boat_display = null;
	// Boat input event processor. To be invoked by Boat.
	public static int processEvent() {
	        if (current_boat_display != null) {
	                current_boat_display.processEvents();
	        }
	        return 0;
	}
	// Boat input event processor.
	
	public void processEvents() {
	        
		event_buffer.nextEvent();
		if (mouse != null && mouse.filterEvent(grab, shouldWarpPointer(), event_buffer)) {
		        return;
		}
		else if (keyboard != null && keyboard.filterEvent(event_buffer)) {
		        return;
		}
		
	}

	public void update() {
		try {
			//processEvents();
			//checkInput();
		} finally {
			
		}
	}

	public void reshape(int x, int y, int width, int height) {
	        
	}
	
	public DisplayMode[] getAvailableDisplayModes() throws LWJGLException {
	        
		try {
                        incDisplay();
                        
                        try {
                                DisplayMode[] modes = nGetAvailableDisplayModes(getDisplay(), getDefaultScreen(), current_displaymode_extension);
                                return modes;
                        } finally {
                                decDisplay();
                        }
                        
		} finally {
			
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
			mouse = new BoatMouse(getDisplay(), getWindow(), getWindow());
		} finally {
			
		}
	}

	public void destroyMouse() {
		mouse = null;
		updateInputGrab();
	}

	public void pollMouse(IntBuffer coord_buffer, ByteBuffer buttons) {
		try {
			mouse.poll(grab, coord_buffer, buttons);
		} finally {
			
		}
	}

	public void readMouse(ByteBuffer buffer) {
		try {
			mouse.read(buffer);
		} finally {
			
		}
	}

	public void setCursorPosition(int x, int y) {
		
		try {
			mouse.setCursorPosition(x, y);
		} finally {
			
		}
	}

	public void grabMouse(boolean new_grab) {
		try {
			if (new_grab != grab) {
				grab = new_grab;
				updateInputGrab();
				mouse.changeGrabbed(grab, shouldWarpPointer());
			}
		} finally {
			
		}
	}

	private boolean shouldWarpPointer() {
		return pointer_grabbed && shouldGrab();
	}

	public int getNativeCursorCapabilities() {
	        
		return 0xffffffff;
	}

	public void setNativeCursor(Object handle) throws LWJGLException {
	        
	}

	public int getMinCursorSize() {
	        
		return 16;
	}
	
	public int getMaxCursorSize() {
	        
		return 16;
	}
	
	/* Keyboard */
	public void createKeyboard() throws LWJGLException {
		
		try {
			keyboard = new BoatKeyboard(getDisplay(), getWindow());
		} finally {
			
		}
	}

	public void destroyKeyboard() {
		
		try {
			keyboard.destroy(getDisplay());
			keyboard = null;
		} finally {
			
		}
	}

	public void pollKeyboard(ByteBuffer keyDownBuffer) {
		
		try {
			keyboard.poll(keyDownBuffer);
		} finally {
			
		}
	}

	public void readKeyboard(ByteBuffer buffer) {
		
		try {
			keyboard.read(buffer);
		} finally {
			
		}
	}

	public Object createCursor(int width, int height, int xHotspot, int yHotspot, int numImages, IntBuffer images, IntBuffer delays) throws LWJGLException {
	        
		return null;
	}

	public void destroyCursor(Object cursorHandle) {
	       
	}
	
	public int getPbufferCapabilities() {
	        
		return 0;
	}
	
	public boolean isBufferLost(PeerInfo handle) {
		return false;
	}

	public PeerInfo createPbuffer(int width, int height, PixelFormat pixel_format, ContextAttribs attribs,
			IntBuffer pixelFormatCaps,
			IntBuffer pBufferAttribs) throws LWJGLException {
		/*
		return new BoatPbufferPeerInfo(width, height, pixel_format);
		*/
		throw new UnsupportedOperationException();
		//return null;
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
	 * <li>Boat should not be set icon</li>
	 * </ul>
	 * The implementation will use the supplied ByteBuffers with image data in RGBA and perform any conversions necessary for the specific platform.
	 *
	 * @param icons Array of icons in RGBA mode
	 * @return number of icons used.
	 */
	public int setIcon(ByteBuffer[] icons) {
	        
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
	        
	}

	public boolean wasResized() {
	        
		return false;
	}

	public float getPixelScaleFactor() {
		return 1f;
	}
}
