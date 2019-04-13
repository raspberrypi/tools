Installation instructions from [Stack Overflow](https://stackoverflow.com/questions/19162072/installing-raspberry-pi-cross-compiler/19269715#19269715)

Pre-requirements
----------------
Before you start you need to make sure the following is installed:

    apt-get install git rsync cmake ia32-libs

Let's cross compile a Pie!
--------------------------
Start with making a folder in your home directory called `raspberrypi`.

Go in to this folder and pull down the ENTIRE tools folder you mentioned above:

    git clone git://github.com/raspberrypi/tools.git

You wanted to use the following of the 3 ones, `gcc-linaro-arm-linux-gnueabihf-raspbian`, if I did not read wrong.

Go into your home directory and add:

    export PATH=$PATH:$HOME/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin

to the end of the file named `~/.bashrc`

Now you can either log out and log back in (i.e. restart your terminal session), or run `. ~/.bashrc` in your terminal to pick up the `PATH` addition in your current terminal session.

Now, verify that you can access the compiler `arm-linux-gnueabihf-gcc -v`. You should get something like this:

    Using built-in specs.
    COLLECT_GCC=arm-linux-gnueabihf-gcc
    COLLECT_LTO_WRAPPER=/home/tudhalyas/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/../libexec/gcc/arm-linux-gnueabihf/4.7.2/lto-wrapper
    Target: arm-linux-gnueabihf
    Configured with: /cbuild/slaves/oort61/crosstool-ng/builds/arm-linux-gnueabihf-raspbian-linux/.b
     uild/src/gcc-linaro-4.7-2012.08/configure --build=i686-build_pc-linux-gnu --host=i686-build_pc-
     linux-gnu --target=arm-linux-gnueabihf --prefix=/cbuild/slaves/oort61/crosstool-ng/builds/arm-l
     inux-gnueabihf-raspbian-linux/install --with-sysroot=/cbuild/slaves/oort61/crosstool-ng/builds/
     arm-linux-gnueabihf-raspbian-linux/install/arm-linux-gnueabihf/libc --enable-languages=c,c++,fo
     rtran --disable-multilib --with-arch=armv6 --with-tune=arm1176jz-s --with-fpu=vfp --with-float=
     hard --with-pkgversion='crosstool-NG linaro-1.13.1+bzr2458 - Linaro GCC 2012.08' --with-bugurl=
     https://bugs.launchpad.net/gcc-linaro --enable-__cxa_atexit --enable-libmudflap --enable-libgom
     p --enable-libssp --with-gmp=/cbuild/slaves/oort61/crosstool-ng/builds/arm-linux-gnueabihf-rasp
     bian-linux/.build/arm-linux-gnueabihf/build/static --with-mpfr=/cbuild/slaves/oort61/crosstool-
     ng/builds/arm-linux-gnueabihf-raspbian-linux/.build/arm-linux-gnueabihf/build/static --with-mpc
     =/cbuild/slaves/oort61/crosstool-ng/builds/arm-linux-gnueabihf-raspbian-linux/.build/arm-linux-
     gnueabihf/build/static --with-ppl=/cbuild/slaves/oort61/crosstool-ng/builds/arm-linux-gnueabihf
     -raspbian-linux/.build/arm-linux-gnueabihf/build/static --with-cloog=/cbuild/slaves/oort61/cros
     stool-ng/builds/arm-linux-gnueabihf-raspbian-linux/.build/arm-linux-gnueabihf/build/static --wi
     th-libelf=/cbuild/slaves/oort61/crosstool-ng/builds/arm-linux-gnueabihf-raspbian-linux/.build/a
     rm-linux-gnueabihf/build/static --with-host-libstdcxx='-L/cbuild/slaves/oort61/crosstool-ng/bui
     lds/arm-linux-gnueabihf-raspbian-linux/.build/arm-linux-gnueabihf/build/static/lib -lpwl' --ena
     ble-threads=posix --disable-libstdcxx-pch --enable-linker-build-id --enable-plugin --enable-gol
     d --with-local-prefix=/cbuild/slaves/oort61/crosstool-ng/builds/arm-linux-gnueabihf-raspbian-li
     nux/install/arm-linux-gnueabihf/libc --enable-c99 --enable-long-long
    Thread model: posix
    gcc version 4.7.2 20120731 (prerelease) (crosstool-NG linaro-1.13.1+bzr2458 - Linaro GCC 2012.08
     )

But hey! I did that and the libs still don't work!
-------

We're not done yet! So far, we've only done the basics.

In your `raspberrypi` folder, make a folder called `rootfs`.

Now you need to copy the entire `/lib`and `/usr` directory to this newly created folder. I usually bring the rpi image up and copy it via rsync:

    rsync -rl --delete-after --safe-links pi@192.168.1.PI:/{lib,usr} $HOME/raspberrypi/rootfs

where `192.168.1.PI` is replaced by the IP of your Raspberry Pi.

Now, we need to write a `cmake` config file. Open `~/home/raspberrypi/pi.cmake` in your favorite editor and insert the following:

    SET(CMAKE_SYSTEM_NAME Linux)
    SET(CMAKE_SYSTEM_VERSION 1)
    SET(CMAKE_C_COMPILER $ENV{HOME}/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc)
    SET(CMAKE_CXX_COMPILER $ENV{HOME}/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-g++)
    SET(CMAKE_FIND_ROOT_PATH $ENV{HOME}/raspberrypi/rootfs)
    SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

Now you should be able to compile your `cmake` programs simply by adding this extra flag: `-D CMAKE_TOOLCHAIN_FILE=$HOME/raspberrypi/pi.cmake`. 

Using a <a href="https://github.com/jameskbride/cmake-hello-world">cmake hello world</a> example:

    git clone https://github.com/jameskbride/cmake-hello-world.git 
    cd cmake-hello-world
    mkdir build
    cd build
    cmake -D CMAKE_TOOLCHAIN_FILE=$HOME/raspberrypi/pi.cmake ../
    make
    scp CMakeHelloWorld pi@192.168.1.PI:/home/pi/
    ssh pi@192.168.1.PI ./CMakeHelloWorld
