<p align="center">
  <img src="https://raw.githubusercontent.com/Antonio-Brum/C-TANIST_MACHINE/bdcd00b156e7e0f84975cc284f1859e1dce37953/assets/pentacle.svg">
</p>

![GitHub License](https://img.shields.io/github/license/Antonio-Brum/C-TANIST_MACHINE)
![GitHub contributors](https://img.shields.io/github/contributors/Antonio-Brum/C-TANIST_MACHINE)
![GitHub top language](https://img.shields.io/github/languages/top/Antonio-Brum/C-TANIST_MACHINE)
# The C-Tanist Virtual Machine
Our virtual machine (and future complete assembly suite) is here! It's called `pentacle`.

## How to compile:

```bash
$ meson setup build
$ meson compile -C build
```

## Dependencies
You need to have installed and properly configured on your machine:

- `meson`
- `ninja`
- `python`
- `OpenGL`
- a compatible `c`/`c++` compiler

If you're on `Linux` or another *POSIX*-compliant OS, use your package manager of choice to install the previous dependencies and:

`glfw`:

```sh
$ [packager manager command to install] glfw
```

or it's build deps:
Via *apt* (Debian, Ubuntu and derivates): `sudo apt install xorg-dev libwayland-dev libxkbcommon-dev wayland-protocols extra-cmake-modules`

Via *dnf* (Fedora and derivates): `sudo dnf install libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel wayland-devel libxkbcommon-devel wayland-protocols-devel extra-cmake-modules`

Arch and derivates: find what is provided by the above packages.

Also, make sure you have an OpenGL implementation **with developer headers** installed (`mesa-devel`, `libgl-dev`).


If you're on `Windows`, you can try the [bundled MSI installer](https://github.com/mesonbuild/meson/releases/download/1.8.1/meson-1.8.1-64.msi).

`meson` should handle all other dependencies for you. 

In case of doubt, refer to the [meson installation manual](https://mesonbuild.com/Getting-meson.html).

## Authors
- [@Antonio-Brum](https://github.com/Antonio-Brum)
- [@Carpedim](https://github.com/Carpedim)
- [@duhdah](https://github.com/duhdah)
- [@gckneip](https://github.com/gckneip)
- [@mthsrnn](https://www.github.com/mthsrnn)
- [@RaffaellSM](https://github.com/RaffaellSM)
- [@Vitor-a-o](https://github.com/Vitor-a-o)
