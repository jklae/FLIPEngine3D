# FLIPEngine3D
## Overview
![demo](docs/images/demo.gif)

FLIPEngine3D is a visualization of the <A href="https://github.com/rlguy/Blender-FLIP-Fluids">FLIP Fluids</A> using <A href="https://github.com/frostsim/DXViewer">DXViewer</A>.

## Build
This repo was developed in the following environment:
* Windows 10 64-bit
* Microsoft Visual Studio 2019 on x64 platform (C++14)
* CMake 3.19.0

You should update submodules before creating a project with cmake.

```bash
git submodule update --progress --init -- "ext/DXViewer"
```

## Reference
* Ryan Guy. _Blender-FLIP-Fluids_. https://github.com/rlguy/Blender-FLIP-Fluids
