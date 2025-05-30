## Prerequisites
1. install Visual Studio 2022
2. winget install --id Git.Git -e --source winget
3. ensure c:\program files\git\bin is in your path
4. winget install ezwinports.make
5. winget install Python.Python.3.9 TODO: steps for winget and store
6. Visual Studio Installer -> Clang

TODO: D3D11 debuglayer

```bash
gh repo clone rive-app/rive-runtime
cd <reporoot>\skia\dependencies
sh make_glfw.sh
cd <reporoot>\renderer
..\build\build_rive.ps1
```

Out contains vsproject and binary.  path_fiddle.exe is dropped there.  TODO flags.  With no flags a single unflagged arg is treated as path to a .riv
