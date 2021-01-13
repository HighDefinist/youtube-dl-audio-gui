cmake_minimum_required (VERSION 3.1)

add_executable(youtube-dl-audio-gui 
  default/main.cpp
  ../ext/imgui/imgui.cpp 
  ../ext/imgui/imgui_demo.cpp
  ../ext/imgui/imgui_draw.cpp
  ../ext/imgui/imgui_impl_dx11.cpp
  ../ext/imgui/imgui_impl_win32.cpp
  ../ext/imgui/imgui_widgets.cpp
)
target_link_libraries(youtube-dl-audio-gui d3d11.lib d3dcompiler.lib dxgi.lib imm32.lib Shcore.lib)
target_compile_features(youtube-dl-audio-gui INTERFACE cxx_std_11)
