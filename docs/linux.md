# Linux

## Build

Build Bullet
```
cd 3rdparty/bullet
mkdir .build
cmake .. -DNO_OPENGL=1 -DCMAKE_INSTALL_PREFIX:PATH=${HOME}/dev/git/Mini-Tech/.install -DBUILD_SHARED_LIBS=1
```

Build Mini-Tech
```
mkdir .build && cd .build
cmake -D3RDPARTY_ROOT=${HOME}/dev/git/Mini-Tech/.install -DGAFFER_ROOT=~/dev/tools/gaffer/gaffer-1.4.2.0-linux-gcc9 -DPYTHON_VERSION=3.10 -DCMAKE_INSTALL_PREFIX:PATH=~/dev/git/Mini-Tech/.install ..
```

## Run

```
env GAFFER_EXTENSION_PATHS=~/dev/git/Mini-Tech/.install GAFFER_REFERENCE_PATHS=~/dev/git/Mini-Tech/.install/nodes MINITECH_REF_NODES=1 ./gaffer
```

Linux laptop with Optimus requires using the nvidia GPU

```
env __NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia GAFFER_EXTENSION_PATHS=~/dev/git/Mini-Tech/.install GAFFER_REFERENCE_PATHS=~/dev/git/Mini-Tech/.install/nodes MINITECH_REF_NODES=1 ./gaffer
```

Running Tests

```
env __NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia GAFFER_EXTENSION_PATHS=~/dev/git/Mini-Tech/.install GAFFER_REFERENCE_PATHS=~/dev/git/Mini-Tech/.install/nodes MINITECH_REF_NODES=1 ./gaffer test MiniGafferTest MiniGafferBulletTest MiniGafferVDBTest
```



