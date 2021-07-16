// This example compiles a dynamic library which can be renamed to libEGL.so and used
// as a local replacement (e.g. place in the same folder as an application or place
// somewhere else and use LD_LIBRARY_PATH to ensure it is loaded before the system libEGL).
// The EGL_TO_WRAP environment variable must be set, or this library will wrap
// itself and cause a segfault.
//
// It loads the system libEGL and wraps the eglSwapBuffers call, injecting a delay
// into it. This effectively caps the framerate of the application.
// Not very useful, but it shows the mechanics.
//
// An example of an application to try this with is the es2gears example from
// mesa-demos. Example of the command line on an X11 system:
//
// LD_PRELOAD=/path/to/my/libEGL.so.1 es2gears_x11
//
// Where /path/to/my/libEGL.so.1  points to the library compiled from this example.
//
// If this works correctly, you should see 2 frames per second and a repeated
// "hello world" on stdout.

#include <egl-wrapper.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

// Our hook for eglSwapBuffers.
EGLBoolean my_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    usleep(500000);
    printf("eglSwapBuffers says hello!\n");
    return bare_eglSwapBuffers(dpy, surface);
}

// Implement eglCreateContext to register our callback.
void egl_wrapper_first_contact(const char* egl_fn_name) {
    static bool registered = false;
    
    if(!registered) {
        egl_wrapper_initialize(NULL);
        register_hook_eglSwapBuffers(my_eglSwapBuffers);
        printf("Registered eglSwapBuffers wrapper.\n");
        registered = true;
    }
}