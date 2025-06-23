## Prerequisites
1. install Visual Studio 2022 with Clang and CMake workloads
2. `winget install --id Git.Git -e --source winget`
3. ensure `c:\program files\git\bin` is in your path
4. ensure `C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin` is in your path
5. Launch `python3` and allow WinStore to install it

```bash
gh repo clone rive-app/rive-runtime
cd <reporoot>\skia\dependencies
sh make_glfw.sh
cd <reporoot>\renderer
..\build\build_rive.ps1
```

Out contains vsproject and binary.  path_fiddle.exe is dropped there.  TODO flags.  With no flags a single unflagged arg is treated as path to a .riv
