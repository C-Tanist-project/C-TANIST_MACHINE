project('pentacle', 'cpp', 'c',
  default_options : ['cpp_std=c++23'])

srcs = ['src/main.cpp', 'src/ui.cpp', 'src/vm.cpp', 'src/windows/memory_editor.cpp', 'src/windows/controls.cpp', 'src/windows/console.cpp', 'src/external/ImGuiFileDialog.cpp', 'src/windows/vmstate.cpp', 'src/assembler.cpp', 'src/linker.cpp', 'src/preprocessor.cpp', 'src/windows/text_editor.cpp']

imgui_compiled_options = ['opengl=enabled',
                          'glfw=enabled',
                          'dx9=disabled',
                          'dx10=disabled',
                          'dx11=disabled',
                          'dx12=disabled',
                          'metal=disabled',
                          'vulkan=disabled',
                          'webgpu=disabled',
                          'sdl2_renderer=disabled',
                          'sdl2=disabled',
                          'sdl3=disabled',
                          'sdl3=disabled',
                          'sdl3_renderer=disabled',
                          'sdl3_renderer=disabled',
                          'allegro5=disabled']

glfw_lib = dependency('glfw3', required: true, static: true)
imgui_lib = dependency('imgui', required: true, static: true, default_options: imgui_compiled_options)

if not glfw_lib.found()
  glfw_proj = subproject('glfw3')
  glfw_lib = glfw_proj.get_variable('glfw3_dep')
endif

if not imgui_lib.found()
  imgui_proj = subproject('imgui')
  imgui_lib = imgui_proj.get_variable('imgui_dep')
endif

deps = [glfw_lib, imgui_lib]

executable('pentacle', sources: srcs, dependencies: deps, install: true)
