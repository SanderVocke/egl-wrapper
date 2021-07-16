# egl-wrapper

egl-wrapper is a library which can be used to hook into EGL API calls.
callbacks can be registered for each of the supported calls.
The default behavior is to forward calls directly to the system's EGL.

## Building

This should build on Linux (tested) and possibly Android (untested)
using straightforward CMake.

## Usage

Create your own library which links to libegl-wrapper.so. In your 
library, you can define hooks into EGL API calls. For example:

```c
EGLBoolean my_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    printf("eglSwapBuffers says hello!\n");
    return bare_eglSwapBuffers(dpy, surface);
}
```

The name of your function doesn't matter, as it will be explicitly
registered below. However, it should have the same calling signature
as the EGL API function you are wrapping (in this case, `eglSwapBuffers`).

You can access the underlying system's EGL APIs by prepending `bare_`
(in this case: `bare_eglSwapBuffers`).

Next, you need to register your hook(s). This can be done by implementing
`void egl_wrapper_first_contact(const char* egl_fn_name)`, which will be
automatically called by `egl-wrapper` upon the first EGL call made by the
client. In this function you should initialize the wrapper to load the
underlying EGL implementation using `egl_wrapper_initialize`. Then, you
can register your hook(s) using the `register_hook_egl***` functions.
Example:

```c
void egl_wrapper_first_contact(const char* egl_fn_name) {
    static bool registered = false;
    if(!registered) {
        egl_wrapper_initialize(NULL);
        register_hook_eglSwapBuffers(my_eglSwapBuffers);
        printf("Registered eglSwapBuffers wrapper.\n");
        registered = true;
    }
}
```

If you compile this into a shared library and make a client load it
(e.g. via LD_PRELOAD), your injected hooks will be used.

See the `slow_render` example in the `examples` folder for a buildable
example.

## Status

This code has so far hardly been used or tested. Only the given `slow_render`
example was tested on Arch Linux on `es2gears`. However, that shows the 
working principle. It should be easy to use this for more complex tasks,
making minor changes to `egl-wrapper` if needed (e.g. for any missing API
call).
