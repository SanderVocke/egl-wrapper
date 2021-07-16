# egl-wrapper

egl-wrapper is a library which can be used to hook into EGL API calls.
callbacks can be registered for each of the supported calls.
The default behavior is to forward calls directly to the system's EGL.

This should build on Linux (tested) and possibly Android (untested)
using straightforward CMake.

See the examples folder for usage.
