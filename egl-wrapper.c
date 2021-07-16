// This file concerns the dynamic loading of the underlying EGL implementation.

#include "egl-wrapper.h"

#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>

// Preprocessor utilities
#define CHECK_FUNC_PTR_AND_INIT(F) \
    if(*F == NULL) { \
        egl_wrapper_first_contact(#F); \
        if(*F == NULL) { \
            fprintf(stderr, "Function " #F " is not initialized\n"); \
            exit(-1); \
        } \
    }

// Types
typedef enum {
    NOT_INITIALIZED = 0,
    INITIALIZING,
    INITIALIZED
} init_state;

// Globals
volatile _Atomic init_state g_init_state = NOT_INITIALIZED;
void * _Atomic g_egl_handle = NULL;

// bare EGL API function pointers
EGLint (*g_eglGetError)(void) = NULL;
EGLDisplay (*g_eglGetDisplay)(EGLNativeDisplayType) = NULL;
EGLBoolean (*g_eglInitialize)(EGLDisplay, EGLint *, EGLint *) = NULL;
EGLBoolean (*g_eglTerminate)(EGLDisplay) = NULL;
const char * (*g_eglQueryString)(EGLDisplay, EGLint) = NULL;
EGLBoolean (*g_eglGetConfigs)(EGLDisplay, EGLConfig *, EGLint, EGLint *) = NULL;
EGLBoolean (*g_eglChooseConfig)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *) = NULL;
EGLBoolean (*g_eglGetConfigAttrib)(EGLDisplay, EGLConfig, EGLint, EGLint *) = NULL;
EGLSurface (*g_eglCreateWindowSurface)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint *) = NULL;
EGLSurface (*g_eglCreatePbufferSurface)(EGLDisplay, EGLConfig, const EGLint *) = NULL;
EGLSurface (*g_eglCreatePixmapSurface)(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint *) = NULL;
EGLBoolean (*g_eglDestroySurface)(EGLDisplay, EGLSurface);
EGLBoolean (*g_eglQuerySurface)(EGLDisplay, EGLSurface, EGLint, EGLint *) = NULL;
EGLBoolean (*g_eglBindAPI)(EGLenum ) = NULL;
EGLenum (*g_eglQueryAPI)(void) = NULL;
EGLBoolean (*g_eglWaitClient)(void) = NULL;
EGLBoolean (*g_eglReleaseThread)(void) = NULL;
EGLSurface (*g_eglCreatePbufferFromClientBuffer)(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint *) = NULL;
EGLBoolean (*g_eglSurfaceAttrib)(EGLDisplay, EGLSurface, EGLint, EGLint) = NULL;
EGLBoolean (*g_eglBindTexImage)(EGLDisplay, EGLSurface, EGLint) = NULL;
EGLBoolean (*g_eglReleaseTexImage)(EGLDisplay, EGLSurface, EGLint) = NULL;
EGLBoolean (*g_eglSwapInterval)(EGLDisplay, EGLint) = NULL;
EGLContext (*g_eglCreateContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint *) = NULL;
EGLBoolean (*g_eglDestroyContext)(EGLDisplay, EGLContext) = NULL;
EGLBoolean (*g_eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext) = NULL;
EGLContext (*g_eglGetCurrentContext)(void) = NULL;
EGLSurface (*g_eglGetCurrentSurface)(EGLint) = NULL;
EGLDisplay (*g_eglGetCurrentDisplay)(void) = NULL;
EGLBoolean (*g_eglQueryContext)(EGLDisplay, EGLContext, EGLint, EGLint *) = NULL;
EGLBoolean (*g_eglWaitGL)(void) = NULL;
EGLBoolean (*g_eglWaitNative)(EGLint) = NULL;
EGLBoolean (*g_eglSwapBuffers)(EGLDisplay, EGLSurface) = NULL;
EGLBoolean (*g_eglCopyBuffers)(EGLDisplay, EGLSurface, EGLNativePixmapType) = NULL;
__eglMustCastToProperFunctionPointerType (*g_eglGetProcAddress)(const char*) = NULL;

// callbacks registered around bare EGL functions
EGLint (*g_callback_eglGetError)(void) = NULL;
EGLDisplay (*g_callback_eglGetDisplay)(EGLNativeDisplayType) = NULL;
EGLBoolean (*g_callback_eglInitialize)(EGLDisplay, EGLint *, EGLint *) = NULL;
EGLBoolean (*g_callback_eglTerminate)(EGLDisplay) = NULL;
const char * (*g_callback_eglQueryString)(EGLDisplay, EGLint) = NULL;
EGLBoolean (*g_callback_eglGetConfigs)(EGLDisplay, EGLConfig *, EGLint, EGLint *) = NULL;
EGLBoolean (*g_callback_eglChooseConfig)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *) = NULL;
EGLBoolean (*g_callback_eglGetConfigAttrib)(EGLDisplay, EGLConfig, EGLint, EGLint *) = NULL;
EGLSurface (*g_callback_eglCreateWindowSurface)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint *) = NULL;
EGLSurface (*g_callback_eglCreatePbufferSurface)(EGLDisplay, EGLConfig, const EGLint *) = NULL;
EGLSurface (*g_callback_eglCreatePixmapSurface)(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint *) = NULL;
EGLBoolean (*g_callback_eglDestroySurface)(EGLDisplay, EGLSurface);
EGLBoolean (*g_callback_eglQuerySurface)(EGLDisplay, EGLSurface, EGLint, EGLint *) = NULL;
EGLBoolean (*g_callback_eglBindAPI)(EGLenum ) = NULL;
EGLenum (*g_callback_eglQueryAPI)(void) = NULL;
EGLBoolean (*g_callback_eglWaitClient)(void) = NULL;
EGLBoolean (*g_callback_eglReleaseThread)(void) = NULL;
EGLSurface (*g_callback_eglCreatePbufferFromClientBuffer)(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint *) = NULL;
EGLBoolean (*g_callback_eglSurfaceAttrib)(EGLDisplay, EGLSurface, EGLint, EGLint) = NULL;
EGLBoolean (*g_callback_eglBindTexImage)(EGLDisplay, EGLSurface, EGLint) = NULL;
EGLBoolean (*g_callback_eglReleaseTexImage)(EGLDisplay, EGLSurface, EGLint) = NULL;
EGLBoolean (*g_callback_eglSwapInterval)(EGLDisplay, EGLint) = NULL;
EGLContext (*g_callback_eglCreateContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint *) = NULL;
EGLBoolean (*g_callback_eglDestroyContext)(EGLDisplay, EGLContext) = NULL;
EGLBoolean (*g_callback_eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext) = NULL;
EGLContext (*g_callback_eglGetCurrentContext)(void) = NULL;
EGLSurface (*g_callback_eglGetCurrentSurface)(EGLint) = NULL;
EGLDisplay (*g_callback_eglGetCurrentDisplay)(void) = NULL;
EGLBoolean (*g_callback_eglQueryContext)(EGLDisplay, EGLContext, EGLint, EGLint *) = NULL;
EGLBoolean (*g_callback_eglWaitGL)(void) = NULL;
EGLBoolean (*g_callback_eglWaitNative)(EGLint) = NULL;
EGLBoolean (*g_callback_eglSwapBuffers)(EGLDisplay, EGLSurface) = NULL;
EGLBoolean (*g_callback_eglCopyBuffers)(EGLDisplay, EGLSurface, EGLNativePixmapType) = NULL;
__eglMustCastToProperFunctionPointerType (*g_callback_eglGetProcAddress)(const char*) = NULL;

void egl_wrapper_initialize(const char* library_path_optional) {
    if(g_init_state == INITIALIZED) {
        return;
    }

    // Set the initializing flag to ensure thread safety
    Bool got_lock = false;
    init_state expected = NOT_INITIALIZED;
    got_lock = atomic_compare_exchange_strong(
        &g_init_state, &expected, INITIALIZING);
    if(!got_lock) {
        // init state was either initializing or initialized already.
        // Either way, we will just return after waiting for at least
        // another potential initialization attempt to finish.
        while(g_init_state == INITIALIZING) {}
        return;
    }

    char* maybe_manual_path = getenv("EGL_TO_WRAP");

    const char* wrapped_egl_path = 
        library_path_optional != NULL ? library_path_optional :
        maybe_manual_path != NULL ? maybe_manual_path :
        "libEGL.so";
    
    printf("Wrapped EGL is: %s\n", wrapped_egl_path);

    g_egl_handle = dlopen(wrapped_egl_path, RTLD_LAZY);

    if(g_egl_handle == NULL) {
        fprintf(stderr, "Failed to load wrapped EGL: %s\n", wrapped_egl_path);
        exit(-1);
    }

    g_eglGetError = dlsym(g_egl_handle, "eglGetError"  );
    g_eglGetDisplay = dlsym(g_egl_handle, "eglGetDisplay"  );
    g_eglInitialize = dlsym(g_egl_handle, "eglInitialize" );
    g_eglTerminate = dlsym(g_egl_handle, "eglTerminate" );
    g_eglQueryString = dlsym(g_egl_handle, "eglQueryString" );
    g_eglGetConfigs = dlsym(g_egl_handle, "eglGetConfigs" );
    g_eglChooseConfig = dlsym(g_egl_handle, "eglChooseConfig" );
    g_eglGetConfigAttrib = dlsym(g_egl_handle, "eglGetConfigAttrib" );
    g_eglCreateWindowSurface = dlsym(g_egl_handle, "eglCreateWindowSurface" );
    g_eglCreatePbufferSurface = dlsym(g_egl_handle, "eglCreatePbufferSurface" );
    g_eglCreatePixmapSurface = dlsym(g_egl_handle, "eglCreatePixmapSurface" );
    g_eglDestroySurface = dlsym(g_egl_handle, "eglDestroySurface" );
    g_eglQuerySurface = dlsym(g_egl_handle, "eglQuerySurface" );
    g_eglBindAPI = dlsym(g_egl_handle, "eglBindAPI" );
    g_eglQueryAPI = dlsym(g_egl_handle, "eglQueryAPI" );
    g_eglWaitClient = dlsym(g_egl_handle, "eglWaitClient" );
    g_eglReleaseThread = dlsym(g_egl_handle, "eglReleaseThread" );
    g_eglCreatePbufferFromClientBuffer = dlsym(g_egl_handle, "eglCreatePbufferFromClientBuffer" );
    g_eglSurfaceAttrib = dlsym(g_egl_handle, "eglSurfaceAttrib" );
    g_eglBindTexImage = dlsym(g_egl_handle, "eglBindTexImage" );
    g_eglReleaseTexImage = dlsym(g_egl_handle, "eglReleaseTexImage" );
    g_eglSwapInterval = dlsym(g_egl_handle, "eglSwapInterval" );
    g_eglCreateContext = dlsym(g_egl_handle, "eglCreateContext" );
    g_eglDestroyContext = dlsym(g_egl_handle, "eglDestroyContext" );
    g_eglMakeCurrent = dlsym(g_egl_handle, "eglMakeCurrent" );
    g_eglGetCurrentContext = dlsym(g_egl_handle, "eglGetCurrentContext" );
    g_eglGetCurrentSurface = dlsym(g_egl_handle, "eglGetCurrentSurface" );
    g_eglGetCurrentDisplay = dlsym(g_egl_handle, "eglGetCurrentDisplay" );
    g_eglQueryContext = dlsym(g_egl_handle, "eglQueryContext" );
    g_eglWaitGL = dlsym(g_egl_handle, "eglWaitGL" );
    g_eglWaitNative = dlsym(g_egl_handle, "eglWaitNative" );
    g_eglSwapBuffers = dlsym(g_egl_handle, "eglSwapBuffers" );
    g_eglCopyBuffers = dlsym(g_egl_handle, "eglCopyBuffers" );
    g_eglGetProcAddress = dlsym(g_egl_handle, "eglGetProcAddress");

    if(g_eglGetError == NULL ||
        g_eglGetDisplay == NULL ||
        g_eglInitialize == NULL ||
        g_eglTerminate == NULL ||
        g_eglQueryString == NULL ||
        g_eglGetConfigs == NULL ||
        g_eglChooseConfig == NULL ||
        g_eglGetConfigAttrib == NULL ||
        g_eglCreateWindowSurface == NULL ||
        g_eglCreatePbufferSurface == NULL ||
        g_eglCreatePixmapSurface == NULL ||
        g_eglDestroySurface == NULL ||
        g_eglQuerySurface == NULL ||
        g_eglBindAPI == NULL ||
        g_eglQueryAPI == NULL ||
        g_eglWaitClient == NULL ||
        g_eglReleaseThread == NULL ||
        g_eglCreatePbufferFromClientBuffer == NULL ||
        g_eglSurfaceAttrib == NULL ||
        g_eglBindTexImage == NULL ||
        g_eglReleaseTexImage == NULL ||
        g_eglSwapInterval == NULL ||
        g_eglCreateContext == NULL ||
        g_eglDestroyContext == NULL ||
        g_eglMakeCurrent == NULL ||
        g_eglGetCurrentContext == NULL ||
        g_eglGetCurrentSurface == NULL ||
        g_eglGetCurrentDisplay == NULL ||
        g_eglQueryContext == NULL ||
        g_eglWaitGL == NULL ||
        g_eglWaitNative == NULL ||
        g_eglSwapBuffers == NULL ||
        g_eglCopyBuffers == NULL ||
        g_eglGetProcAddress == NULL
    ) {
        fprintf(stderr, "Failed to load all EGL API functions\n");
        exit(-1);
    }

    g_init_state = INITIALIZED;
}

EGLint bare_eglGetError(void) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetError);
    return g_eglGetError();
}

EGLDisplay bare_eglGetDisplay(EGLNativeDisplayType display_id) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetDisplay);
    return g_eglGetDisplay(display_id);
}

EGLBoolean bare_eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglInitialize);
    return g_eglInitialize(dpy, major, minor);
}

EGLBoolean bare_eglTerminate(EGLDisplay dpy) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglTerminate);
    return g_eglTerminate(dpy);
}

const char * bare_eglQueryString(EGLDisplay dpy, EGLint name) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglQueryString);
    return g_eglQueryString(dpy, name);
}

EGLBoolean bare_eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetConfigs);
    return g_eglGetConfigs(dpy, configs, config_size, num_config);
}

EGLBoolean bare_eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglChooseConfig);
    return g_eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

EGLBoolean bare_eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetConfigAttrib);
    return g_eglGetConfigAttrib(dpy, config, attribute, value);
}

EGLSurface bare_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreateWindowSurface);
    return g_eglCreateWindowSurface(dpy, config, win, attrib_list);
}

EGLSurface bare_eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreatePbufferSurface);
    return g_eglCreatePbufferSurface(dpy, config, attrib_list);
}

EGLSurface bare_eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreatePixmapSurface);
    return g_eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
}

EGLBoolean bare_eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglDestroySurface);
    return g_eglDestroySurface(dpy, surface);
}

EGLBoolean bare_eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglQuerySurface);
    return g_eglQuerySurface(dpy, surface, attribute, value);
}

EGLBoolean bare_eglBindAPI(EGLenum api) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglBindAPI);
    return g_eglBindAPI(api);
}

EGLenum bare_eglQueryAPI(void) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglQueryAPI);
    return g_eglQueryAPI();
}

EGLBoolean bare_eglWaitClient(void) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglWaitClient);
    return g_eglWaitClient();
}

EGLBoolean bare_eglReleaseThread(void) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglReleaseThread);
    return g_eglReleaseThread();
}

EGLSurface bare_eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreatePbufferFromClientBuffer);
    return g_eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
}

EGLBoolean bare_eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglSurfaceAttrib);
    return g_eglSurfaceAttrib(dpy, surface, attribute, value);
}

EGLBoolean bare_eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglBindTexImage);
    return g_eglBindTexImage(dpy, surface, buffer);
}

EGLBoolean bare_eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglReleaseTexImage);
    return g_eglReleaseTexImage(dpy, surface, buffer);
}

EGLBoolean bare_eglSwapInterval(EGLDisplay dpy, EGLint interval) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglSwapInterval);
    return g_eglSwapInterval(dpy, interval);
}

EGLContext bare_eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreateContext);
    return g_eglCreateContext(dpy, config, share_context, attrib_list);
}

EGLBoolean bare_eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglDestroyContext);
    return g_eglDestroyContext(dpy, ctx);
}

EGLBoolean bare_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglMakeCurrent);
    return g_eglMakeCurrent(dpy, draw, read, ctx);
}

EGLContext bare_eglGetCurrentContext(void) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetCurrentContext);
    return g_eglGetCurrentContext();
}

EGLSurface bare_eglGetCurrentSurface(EGLint readdraw) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetCurrentSurface);
    return g_eglGetCurrentSurface(readdraw);
}

EGLDisplay bare_eglGetCurrentDisplay(void) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetCurrentDisplay);
    return g_eglGetCurrentDisplay();
}

EGLBoolean bare_eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglQueryContext);
    return g_eglQueryContext(dpy, ctx, attribute, value);
}

EGLBoolean bare_eglWaitGL(void) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglWaitGL);
    return g_eglWaitGL();
}

EGLBoolean bare_eglWaitNative(EGLint engine) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglWaitNative);
    return g_eglWaitNative(engine);
}

EGLBoolean bare_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglSwapBuffers);
    return g_eglSwapBuffers(dpy, surface);
}

EGLBoolean bare_eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglCopyBuffers);
    return g_eglCopyBuffers(dpy, surface, target);
}

__eglMustCastToProperFunctionPointerType bare_eglGetProcAddress(const char* proc_name) {
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetProcAddress);
    return g_eglGetProcAddress(proc_name);
}

EGLint eglGetError(void) {
    if(g_callback_eglGetError != NULL) {
        return g_callback_eglGetError();
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetError);
    return g_eglGetError();
}

EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id) {
    if(g_callback_eglGetDisplay != NULL) {
        return g_callback_eglGetDisplay(display_id);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetDisplay);
    return g_eglGetDisplay(display_id);
}

EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) {
    if(g_callback_eglInitialize != NULL) {
        return g_callback_eglInitialize(dpy, major, minor);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglInitialize);
    return g_eglInitialize(dpy, major, minor);
}

EGLBoolean eglTerminate(EGLDisplay dpy) {
    if(g_callback_eglTerminate != NULL) {
        return g_callback_eglTerminate(dpy);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglTerminate);
    return g_eglTerminate(dpy);
}

const char * eglQueryString(EGLDisplay dpy, EGLint name) {
    if(g_callback_eglQueryString != NULL) {
        return g_callback_eglQueryString(dpy, name);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglQueryString);
    return g_eglQueryString(dpy, name);
}

EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    if(g_callback_eglGetConfigs != NULL) {
        return g_callback_eglGetConfigs(dpy, configs, config_size, num_config);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetConfigs);
    return g_eglGetConfigs(dpy, configs, config_size, num_config);
}

EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    if(g_callback_eglChooseConfig != NULL) {
        return g_callback_eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglChooseConfig);
    return g_eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) {
    if(g_callback_eglGetConfigAttrib != NULL) {
        return g_callback_eglGetConfigAttrib(dpy, config, attribute, value);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetConfigAttrib);
    return g_eglGetConfigAttrib(dpy, config, attribute, value);
}

EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) {
    if(g_callback_eglCreateWindowSurface != NULL) {
        return g_callback_eglCreateWindowSurface(dpy, config, win, attrib_list);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreateWindowSurface);
    return g_eglCreateWindowSurface(dpy, config, win, attrib_list);
}

EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
    if(g_callback_eglCreatePbufferSurface != NULL) {
        return g_callback_eglCreatePbufferSurface(dpy, config, attrib_list);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreatePbufferSurface);
    return g_eglCreatePbufferSurface(dpy, config, attrib_list);
}

EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) {
    if(g_callback_eglCreatePixmapSurface != NULL) {
        return g_callback_eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreatePixmapSurface);
    return g_eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
}

EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
    if(g_callback_eglDestroySurface != NULL) {
        return g_callback_eglDestroySurface(dpy, surface);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglDestroySurface);
    return g_eglDestroySurface(dpy, surface);
}

EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) {
    if(g_callback_eglQuerySurface != NULL) {
        return g_callback_eglQuerySurface(dpy, surface, attribute, value);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglQuerySurface);
    return g_eglQuerySurface(dpy, surface, attribute, value);
}

EGLBoolean eglBindAPI(EGLenum api) {
    if(g_callback_eglBindAPI != NULL) {
        return g_callback_eglBindAPI(api);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglBindAPI);
    return g_eglBindAPI(api);
}

EGLenum eglQueryAPI(void) {
    if(g_callback_eglQueryAPI != NULL) {
        return g_callback_eglQueryAPI();
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglQueryAPI);
    return g_eglQueryAPI();
}

EGLBoolean eglWaitClient(void) {
    if(g_callback_eglWaitClient != NULL) {
        return g_callback_eglWaitClient();
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglWaitClient);
    return g_eglWaitClient();
}

EGLBoolean eglReleaseThread(void) {
    if(g_callback_eglReleaseThread != NULL) {
        return g_callback_eglReleaseThread();
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglReleaseThread);
    return g_eglReleaseThread();
}

EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) {
    if(g_callback_eglCreatePbufferFromClientBuffer != NULL) {
        return g_callback_eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreatePbufferFromClientBuffer);
    return g_eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
}

EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
    if(g_callback_eglSurfaceAttrib != NULL) {
        return g_callback_eglSurfaceAttrib(dpy, surface, attribute, value);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglSurfaceAttrib);
    return g_eglSurfaceAttrib(dpy, surface, attribute, value);
}

EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    if(g_callback_eglBindTexImage != NULL) {
        return g_callback_eglBindTexImage(dpy, surface, buffer);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglBindTexImage);
    return g_eglBindTexImage(dpy, surface, buffer);
}

EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    if(g_callback_eglReleaseTexImage != NULL) {
        return g_callback_eglReleaseTexImage(dpy, surface, buffer);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglReleaseTexImage);
    return g_eglReleaseTexImage(dpy, surface, buffer);
}

EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval) {
    if(g_callback_eglSwapInterval != NULL) {
        return g_callback_eglSwapInterval(dpy, interval);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglSwapInterval);
    return g_eglSwapInterval(dpy, interval);
}

EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) {
    if(g_callback_eglCreateContext != NULL) {
        return g_callback_eglCreateContext(dpy, config, share_context, attrib_list);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglCreateContext);
    return g_eglCreateContext(dpy, config, share_context, attrib_list);
}

EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
    if(g_callback_eglDestroyContext != NULL) {
        return g_callback_eglDestroyContext(dpy, ctx);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglDestroyContext);
    return g_eglDestroyContext(dpy, ctx);
}

EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    if(g_callback_eglMakeCurrent != NULL) {
        return g_callback_eglMakeCurrent(dpy, draw, read, ctx);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglMakeCurrent);
    return g_eglMakeCurrent(dpy, draw, read, ctx);
}

EGLContext eglGetCurrentContext(void) {
    if(g_callback_eglGetCurrentContext != NULL) {
        return g_callback_eglGetCurrentContext();
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetCurrentContext);
    return g_eglGetCurrentContext();
}

EGLSurface eglGetCurrentSurface(EGLint readdraw) {
    if(g_callback_eglGetCurrentSurface != NULL) {
        return g_callback_eglGetCurrentSurface(readdraw);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetCurrentSurface);
    return g_eglGetCurrentSurface(readdraw);
}

EGLDisplay eglGetCurrentDisplay(void) {
    if(g_callback_eglGetCurrentDisplay != NULL) {
        return g_callback_eglGetCurrentDisplay();
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetCurrentDisplay);
    return g_eglGetCurrentDisplay();
}

EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) {
    if(g_callback_eglQueryContext != NULL) {
        return g_callback_eglQueryContext(dpy, ctx, attribute, value);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglQueryContext);
    return g_eglQueryContext(dpy, ctx, attribute, value);
}

EGLBoolean eglWaitGL(void) {
    if(g_callback_eglWaitGL != NULL) {
        return g_callback_eglWaitGL();
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglWaitGL);
    return g_eglWaitGL();
}

EGLBoolean eglWaitNative(EGLint engine) {
    if(g_callback_eglWaitNative != NULL) {
        return g_callback_eglWaitNative(engine);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglWaitNative);
    return g_eglWaitNative(engine);
}

EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if(g_callback_eglSwapBuffers != NULL) {
        return g_callback_eglSwapBuffers(dpy, surface);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglSwapBuffers);
    return g_eglSwapBuffers(dpy, surface);
}

EGLBoolean eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) {
    if(g_callback_eglCopyBuffers != NULL) {
        return g_callback_eglCopyBuffers(dpy, surface, target);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglCopyBuffers);
    return g_eglCopyBuffers(dpy, surface, target);
}

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* proc_name) {
    if(g_callback_eglGetProcAddress != NULL) {
        return g_callback_eglGetProcAddress(proc_name);
    }
    CHECK_FUNC_PTR_AND_INIT(&g_eglGetProcAddress);
    return g_eglGetProcAddress(proc_name);
}

void register_hook_eglGetError(EGLint (*hook)(void)) {
    g_callback_eglGetError = hook;
}

void register_hook_eglGetDisplay(EGLDisplay (*hook)(EGLNativeDisplayType display_id)) {
    g_callback_eglGetDisplay = hook;
}

void register_hook_eglInitialize(EGLBoolean (*hook)(EGLDisplay dpy, EGLint *major, EGLint *minor)) {
    g_callback_eglInitialize = hook;
}

void register_hook_eglTerminate(EGLBoolean (*hook)(EGLDisplay dpy)) {
    g_callback_eglTerminate = hook;
}

void register_hook_eglQueryString(const char * (*hook)(EGLDisplay dpy, EGLint name)) {
    g_callback_eglQueryString = hook;
}

void register_hook_eglGetConfigs(EGLBoolean (*hook)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)) {
    g_callback_eglGetConfigs = hook;
}

void register_hook_eglChooseConfig(EGLBoolean (*hook)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)) {
    g_callback_eglChooseConfig = hook;
}

void register_hook_eglGetConfigAttrib(EGLBoolean (*hook)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)) {
    g_callback_eglGetConfigAttrib = hook;
}

void register_hook_eglCreateWindowSurface(EGLSurface (*hook)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)) {
    g_callback_eglCreateWindowSurface = hook;
}

void register_hook_eglCreatePbufferSurface(EGLSurface (*hook)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)) {
    g_callback_eglCreatePbufferSurface = hook;
}

void register_hook_eglCreatePixmapSurface(EGLSurface (*hook)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)) {
    g_callback_eglCreatePixmapSurface = hook;
}

void register_hook_eglDestroySurface(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface)) {
    g_callback_eglDestroySurface = hook;
}

void register_hook_eglQuerySurface(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)) {
    g_callback_eglQuerySurface = hook;
}

void register_hook_eglBindAPI(EGLBoolean (*hook)(EGLenum api)) {
    g_callback_eglBindAPI = hook;
}

void register_hook_eglQueryAPI(EGLenum (*hook)(void)) {
    g_callback_eglQueryAPI = hook;
}

void register_hook_eglWaitClient(EGLBoolean (*hook)(void)) {
    g_callback_eglWaitClient = hook;
}

void register_hook_eglReleaseThread(EGLBoolean (*hook)(void)) {
    g_callback_eglReleaseThread = hook;
}

void register_hook_eglCreatePbufferFromClientBuffer(EGLSurface (*hook)( EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)) {
    g_callback_eglCreatePbufferFromClientBuffer = hook;
}

void register_hook_eglSurfaceAttrib(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)) {
    g_callback_eglSurfaceAttrib = hook;
}

void register_hook_eglBindTexImage(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLint buffer)) {
    g_callback_eglBindTexImage = hook;
}

void register_hook_eglReleaseTexImage(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLint buffer)) {
    g_callback_eglReleaseTexImage = hook;
}

void register_hook_eglSwapInterval(EGLBoolean (*hook)(EGLDisplay dpy, EGLint interval)) {
    g_callback_eglSwapInterval = hook;
}

void register_hook_eglCreateContext(EGLContext (*hook)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)) {
    g_callback_eglCreateContext = hook;
}

void register_hook_eglDestroyContext(EGLBoolean (*hook)(EGLDisplay dpy, EGLContext ctx)) {
    g_callback_eglDestroyContext = hook;
}

void register_hook_eglMakeCurrent(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)) {
    g_callback_eglMakeCurrent = hook;
}

void register_hook_eglGetCurrentContext(EGLContext (*hook)(void)) {
    g_callback_eglGetCurrentContext = hook;
}

void register_hook_eglGetCurrentSurface(EGLSurface (*hook)(EGLint readdraw)) {
    g_callback_eglGetCurrentSurface = hook;
}

void register_hook_eglGetCurrentDisplay(EGLDisplay (*hook)(void)) {
    g_callback_eglGetCurrentDisplay = hook;
}

void register_hook_eglQueryContext(EGLBoolean (*hook)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)) {
    g_callback_eglQueryContext = hook;
}

void register_hook_eglWaitGL(EGLBoolean (*hook)(void)) {
    g_callback_eglWaitGL = hook;
}

void register_hook_eglWaitNative(EGLBoolean (*hook)(EGLint engine)) {
    g_callback_eglWaitNative = hook;
}

void register_hook_eglSwapBuffers(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface)) {
    g_callback_eglSwapBuffers = hook;
}

void register_hook_eglCopyBuffers(EGLBoolean (*hook)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)) {
    g_callback_eglCopyBuffers = hook;
}

void register_hook_eglGetProcAddress(__eglMustCastToProperFunctionPointerType (*hook)(const char* proc_name)) {
    g_callback_eglGetProcAddress = hook;
}