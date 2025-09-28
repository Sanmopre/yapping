# <img src="docs/images/logo.png" alt="logo" width="100" style="vertical-align: middle;"/> YAPPING

Open source cross-platform chatting application. For more information check the [yapping website](https://www.santiagomoliner.com/yapping/). 

## Build status

![Build](https://github.com/Sanmopre/yapping/actions/workflows/build.yml/badge.svg)

## Dependencies

The application uses the following libraries:
- [boost-asio](https://www.boost.org/doc/libs/1_89_0/doc/html/boost_asio.html)
- [CLI11](https://github.com/CLIUtils/CLI11)
- [imgui](https://github.com/ocornut/imgui)
- [nlohmann-json](https://github.com/nlohmann/json)
- [SDL2](https://www.libsdl.org/)
- [spdlog](https://github.com/gabime/spdlog)
- [zlib](https://github.com/madler/zlib)

## Build from source
First add all the necessary submodules with
```
git submodule update --init --recursive
```

After that, just run the following cmake commands
```
cmake -S . -B build -DBUILD_CLIENT:BOOL=ON -DBUILD_SERVER:BOOL=ON
cmake --build build
```