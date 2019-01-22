cmake_minimum_required (VERSION 3.1)

add_executable(default 
  default/main.cpp
  ../ext/imgui/imgui.cpp 
  ../ext/imgui/imgui_demo.cpp
  ../ext/imgui/imgui_draw.cpp
  ../ext/imgui/imgui_impl_dx11.cpp
  ../ext/imgui/imgui_impl_win32.cpp
  ../ext/imgui/imgui_widgets.cpp
)
target_link_libraries(default d3d11.lib d3dcompiler.lib dxgi.lib imm32.lib)
target_compile_features(default INTERFACE cxx_std_11)
