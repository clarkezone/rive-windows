## Prerequisites
install Visual Studio 2022
winget install --id Git.Git -e --source winget
ensure c:\program files\git\bin is in your path
winget install ezwinports.make
winget install Python.Python.3.9 TODO: steps for winget and store
Visual Studio Installer -> Clang
TODO: D3D11 debuglayer


gh repo clone rive-app/rive-runtime
cd <reporoot>\skia\dependencies
sh make_glfw.sh
cd <reporoot>\renderer
..\build\build_rive.ps1
