LWJGL - Lightweight Java Game Library
======

The Lightweight Java Game Library (LWJGL) is a solution aimed directly at professional and amateur Java programmers alike to enable commercial quality games to be written in Java. 
LWJGL provides developers access to high performance crossplatform libraries such as OpenGL (Open Graphics Library), OpenCL (Open Computing Language) and OpenAL (Open Audio Library) allowing for state of the art 3D games and 3D sound.
Additionally LWJGL provides access to controllers such as Gamepads, Steering wheel and Joysticks.
All in a simple and straight forward API.

Website: [http://lwjgl.org](http://lwjgl.org)
Forum: [http://lwjgl.org/forum](http://lwjgl.org/forum)
Bugs/Suggestions: [https://github.com/LWJGL/lwjgl/issues](https://github.com/LWJGL/lwjgl/issues)

Compilation
-----------

LWJGL requires a JDK and Ant installed to compile, as well as your platforms native compiler to compile the JNI.

* ant generate-all
* ant compile
* ant compile_native


There are some extra Ant variables needed when building for Boat, use `-D` flag of Ant to specific.
* `-Dinclude=`
  Specific compiler flags to include JDK headers.

* `-Dcross.compile.target=`
  Prefix of target cross-compile platform, such as `arm-linux-androideabi` and `aarch64-linux-android`.
  
* `-Dlwjgl.platform.boat=`
  Set this variable to `true` to enable building for Boat.

* `-Dbuild.32bit.only=`
  Set this variable to `true` to enable building 32bit only.
  
* `-Dbuild.64bit.only=`
  Set this variable to `true` to enable building 64bit only.

* `-Dboat.include=`
  Specific the directory of Boat headers (`boat.h`).
  
* `-Dboat.lib=`
  Specific the directory of Boat libraries (`libboat.so`).

