add_library(slow_render_example SHARED slow_render.c)
target_link_libraries(slow_render_example PRIVATE egl-wrapper)

# Ensure we create a library called libEGL.so.1
set_target_properties(slow_render_example PROPERTIES PREFIX "lib")
set_target_properties(slow_render_example PROPERTIES OUTPUT_NAME "EGL")
set_target_properties(slow_render_example PROPERTIES SUFFIX ".so.1")
