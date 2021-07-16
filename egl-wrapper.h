#ifndef WRAPPED_EGL_H
#define WRAPPED_EGL_H

// Include the EGL API. calls made to this API will call the
// wrapped versions.
// The bare underlying EGL API is also accessible, through the
// modified symbols in this header file.
#include <EGL/egl.h>

// Load the undelying (wrapped) EGL dynamically. This should be called
// by the user's first contact callback.
// Searching procedure:
// - If library_path_optional is set, attempts to load from here.
// - Otherwise, if EGL_TO_WRAP env variable is set, attempts to use this.
// - Otherwise, attempts to simply load "libEGL.so" to rely
//   on the standard linker search paths.
// If any of these attempts fails, an error is printed and the program exited.
// If any calls are made to the wrapped or bare EGL functions
// before this load is finished, the program is also exited.
// TODO: proper error handling with a return code
void egl_wrapper_initialize(const char* library_path_optional);

// This function should be implemented by the user of this library.
// It will be called when an EGL call is made and the wrapper is not
// yet initialized. The user can use this to register any callbacks
// necessary.
void egl_wrapper_first_contact(const char* egl_fn_name);

// Access to the bare API.

EGLAPI EGLint EGLAPIENTRY bare_eglGetError(void);
EGLAPI EGLDisplay EGLAPIENTRY bare_eglGetDisplay(EGLNativeDisplayType display_id);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglTerminate(EGLDisplay dpy);
EGLAPI const char * EGLAPIENTRY bare_eglQueryString(EGLDisplay dpy, EGLint name);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglGetConfigs(EGLDisplay dpy, EGLConfig *configs,
			 EGLint config_size, EGLint *num_config);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list,
			   EGLConfig *configs, EGLint config_size,
			   EGLint *num_config);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config,
			      EGLint attribute, EGLint *value);
EGLAPI EGLSurface EGLAPIENTRY bare_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
				  EGLNativeWindowType win,
				  const EGLint *attrib_list);
EGLAPI EGLSurface EGLAPIENTRY bare_eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config,
				   const EGLint *attrib_list);
EGLAPI EGLSurface EGLAPIENTRY bare_eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config,
				  EGLNativePixmapType pixmap,
				  const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
			   EGLint attribute, EGLint *value);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglBindAPI(EGLenum api);
EGLAPI EGLenum EGLAPIENTRY bare_eglQueryAPI(void);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglWaitClient(void);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglReleaseThread(void);
EGLAPI EGLSurface EGLAPIENTRY bare_eglCreatePbufferFromClientBuffer(
	      EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
	      EGLConfig config, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface,
			    EGLint attribute, EGLint value);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglSwapInterval(EGLDisplay dpy, EGLint interval);
EGLAPI EGLContext EGLAPIENTRY bare_eglCreateContext(EGLDisplay dpy, EGLConfig config,
			    EGLContext share_context,
			    const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
			  EGLSurface read, EGLContext ctx);
EGLAPI EGLContext EGLAPIENTRY bare_eglGetCurrentContext(void);
EGLAPI EGLSurface EGLAPIENTRY bare_eglGetCurrentSurface(EGLint readdraw);
EGLAPI EGLDisplay EGLAPIENTRY bare_eglGetCurrentDisplay(void);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglQueryContext(EGLDisplay dpy, EGLContext ctx,
			   EGLint attribute, EGLint *value);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglWaitGL(void);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglWaitNative(EGLint engine);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
EGLAPI EGLBoolean EGLAPIENTRY bare_eglCopyBuffers(EGLDisplay dpy, EGLSurface surface,
			  EGLNativePixmapType target);
EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY bare_eglGetProcAddress
			(const char *procname);

// API to register hooks to replace calls to EGL functions.
// If a hook is registered, a call to eglXXXX will instead result in a
// call to this hook.
// Note that hooks still have access to calling the bare EGL functions
// in bare_eglXXXX.
// Only one hook may be registered at a time.
// Registering a NULL hook goes back to the original behavior.
// The only API call for which there is no registration function is
// eglCreateContext. That is because this library leaves that function
// unimplemented. The user of this library can implement it as an entry
// point to register all the needed callbacks.
void register_hook_eglGetError(EGLint (*hook)(void));
void register_hook_eglGetDisplay(EGLDisplay (*hook)(EGLNativeDisplayType display_id));
void register_hook_eglInitialize(EGLBoolean (*hook)(EGLDisplay dpy, EGLint *major, EGLint *minor));
void register_hook_eglTerminate(EGLBoolean (*hook)(EGLDisplay dpy));
void register_hook_eglQueryString(const char * (*hook)(EGLDisplay dpy, EGLint name));
void register_hook_eglGetConfigs(EGLBoolean (*hook)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config));
void register_hook_eglChooseConfig(EGLBoolean (*hook)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config));
void register_hook_eglGetConfigAttrib(EGLBoolean (*hook)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value));
void register_hook_eglCreateWindowSurface(EGLSurface (*hook)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list));
void register_hook_eglCreatePbufferSurface(EGLSurface (*hook)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list));
void register_hook_eglCreatePixmapSurface(EGLSurface (*hook)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list));
void register_hook_eglDestroySurface(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface));
void register_hook_eglQuerySurface(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value));
void register_hook_eglBindAPI(EGLBoolean (*hook)(EGLenum api));
void register_hook_eglQueryAPI(EGLenum (*hook)(void));
void register_hook_eglWaitClient(EGLBoolean (*hook)(void));
void register_hook_eglReleaseThread(EGLBoolean (*hook)(void));
void register_hook_eglCreatePbufferFromClientBuffer(EGLSurface (*hook)( EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list));
void register_hook_eglSurfaceAttrib(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value));
void register_hook_eglBindTexImage(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLint buffer));
void register_hook_eglReleaseTexImage(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLint buffer));
void register_hook_eglSwapInterval(EGLBoolean (*hook)(EGLDisplay dpy, EGLint interval));
void register_hook_eglDestroyContext(EGLBoolean (*hook)(EGLDisplay dpy, EGLContext ctx));
void register_hook_eglMakeCurrent(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx));
void register_hook_eglGetCurrentContext(EGLContext (*hook)(void));
void register_hook_eglGetCurrentSurface(EGLSurface (*hook)(EGLint readdraw));
void register_hook_eglGetCurrentDisplay(EGLDisplay (*hook)(void));
void register_hook_eglQueryContext(EGLBoolean (*hook)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value));
void register_hook_eglWaitGL(EGLBoolean (*hook)(void));
void register_hook_eglWaitNative(EGLBoolean (*hook)(EGLint engine));
void register_hook_eglSwapBuffers(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface));
void register_hook_eglCopyBuffers(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target));
void register_hook_eglGetProcAddress(__eglMustCastToProperFunctionPointerType (*hook)(const char* procname));

#endif