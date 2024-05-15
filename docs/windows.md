# Windows

## Build

```
mkdir .build && cd .build
cmake -B .  -DCMAKE_INSTALL_PREFIX:PATH=${HOME}/dev/git/Mini-Tech/.install -D3RDPARTY_ROOT=${HOME}/dev/git/Mini-Tech/.install -DGAFFER_ROOT=${HOME}/OneDrive/Desktop/gaffer-1.2.2.0-windows -DPYTHON_VERSION=310 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_BUILD_TYPE=Release -S ..
cmake --build .  --config Release
cmake --install .  --config Release
```

## Run

**Windows Updated Gaffer.cmd to install extension path**

todo: MINITECH_REF_NODES=1

```
call :prependToPath "${HOME}\dev\Mini-Tech\.install"  GAFFER_EXTENSION_PATHS
call :prependToPath "${HOME}\dev\Mini-Tech\.install\nodes"  GAFFER_REFERENCE_PATHS
```