# FLIPEngine3D
## Overview
![demo](docs/images/demo.gif)

FLIPEngine3D is a visualization of the FLIP Fluids<sup>[1](#footnote_1)</sup> using <A href="https://github.com/frostsim/DXViewer">DXViewer</A>.

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
* <a name="footnote_1">[1]</a> Ryan Guy. _Blender-FLIP-Fluids_. https://github.com/rlguy/Blender-FLIP-Fluids
