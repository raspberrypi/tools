Linaro Toolchain 2012.08
========================

This package contains pre-built versions of Linaro GCC and Linaro GDB
that target either the Linaro Evaluation Build or baremetal systems.

Installation
------------
To install:
 * On Linux, ensure the 32 bit LSB libraries installed
 * Extract the tarball to a convenient location such as `/opt`
 * Add the `bin` directory to your path

See below for more detailed instructions.

This package can be installed in any location.  Consider using the
included `arm-*-pkg-config` to make finding libraries and dependencies
easier.

What's included
---------------
 * Linaro GCC 4.7 2012.08
 * Linaro GDB 7.4 2012.06
 * A statically linked gdbserver in arm-linux-gnueabihf/debug-root
 * A system root
 * Manuals under share/doc/

The system root contains the basic header files and libraries to link
your programs against.

Support
-------
Need help?  Ask a question on https://ask.linaro.org/

Already on Launchpad?  Submit a bug at https://bugs.launchpad.net/linaro-toolchain-binaries

On IRC?  See us on #linaro on Freenode.

Other ways that you can contact us or get involved are listed at
https://wiki.linaro.org/GettingInvolved.

Target compatibility
--------------------
The arm-linux-gnueabihf version targets the Linaro Evaluation Build
12.05 flavour of Ubuntu 12.04 "Precise Pangolin".  The default
configuration is:

 * Runs on all Cortex-A profile devices
 * Tuned for the Cortex-A9
 * Thumb-2
 * 'hard float' calling convention
 * Uses the VFPv3-D16 FPU
 * Multiarch and multilib enabled
 * EGLIBC 2.15
 * A GCC 4.7 series libgcc and libstdc++

For more information on the LEB, see the Linaro Wiki at
https://wiki.linaro.org/

Precise comes with a 4.6 based runtime.  To use all of the features of
Linaro GCC 4.7, and especially if you get runtime errors about a
missing 'GCC_4.7.0 version', please download the runtime package from:

 https://launchpad.net/linaro-toolchain-binaries/+download

and extract on the target.  Future LEB images will include the 4.7
runtime by default.

In addition to the default configuration, a basic ARMv4T runtime is
included for use when building baremetal projects such as Linux and
u-boot for earlier architectures.  To use it, pass the following flags
to GCC:

 -marm -march=armv4t -mfloat-abi=soft

The arm-none-eabi version targets a baremetal system.  The default
configuration is:

 * Runs on all Cortex-M profile devices
 * Tuned for the Cortex-M3
 * Thumb-2
 * Software floating point only
 * Newlib 1.19.0
 * A GCC 4.7 series libgcc and libstdc++

The baremetal version is unsupported.

Host compatibility
------------------
The Linux version is supported on:
 * Ubuntu 10.04.3 and 11.10
 * Debian 6.0.2
 * Fedora 16
 * openSUSE 12.1
 * Red Hat Enterprise Linux Workstation 5.7 and later

and should run on any Linux Standard Base 3.0 compatible
distribution.  Installing the LSB depends on the distribution:

 * On Ubuntu or Debian, run `sudo apt-get install lsb`
 * On Fedora 16, run `sudo yum install redhat-lsb.i686`
 * On openSUSE 12, run `sudo zypper install lsb.i586`
 * Red Hat 5 is LSB compatible by default
 * For other distributions, please see the distribution documentation

This package contains 32 bit executables that also run on 64 bit
hosts.  In some cases you may need to install the 32 bit support
libraries.  On Ubuntu or Debian run `sudo apt-get install ia32-libs`.
For other distributions, please see the distribution documentation.

The Windows version is supported on:
 * Windows XP Pro SP3
 * Windows Vista Business SP2
 * Windows 7 Pro SP1

on both 32 bit and 64 bit hosts.  No other files are required.
For best results, make sure there are no spaces in the directory name.

Source
------
This package was built using an Ubuntu 10.04 i686 host, crosstool-NG,
and the source tarballs included in the `src` archive.

The following instructions are untested.  Patches are welcome!

To reproduce this build:
 * Install the build dependencies:
  * apt-get build-dep gcc binutils gdb
  * apt-get install lsb lsb-build-cc3 lsb-appchk3 ccache
  * apt-get install gcc-mingw32 gcc-4.1 g++-4.1 gcc-4.1-multilib g++-4.1-multilib
  * apt-get install texlive flip
 * Building requires /bin/sh to be bash. See http://crosstool-ng.org/ for more
 * Download the `src` archive
 * Download and extract the crosstool-NG tarball
 * Change to the crosstool-NG directory
 * Extract the `src` archive giving `tarballs/binutils-*` and similar
 * Run `make -f contrib/linaro/build.mk`

The builds will end up in `builds/$target-$host`.  See
`contrib/linaro/build.mk` for more.

Alternatively, you can build directly using the sample configurations.
See the crosstool-NG documentation and `samples/linaro-arm-linux-gnueabihf`.

A patched version of the LSB is used to work around configuration and
host GCC compatibility issues.  `build.mk` builds and uses this
automatically.  See `contrib/linaro/lsb/make-lsb.sh` for manual builds.

Links
-----
This project:
 https://launchpad.net/linaro-toolchain-binaries

Unsupported builds:
 https://launchpad.net/linaro-toolchain-unsupported

The Linaro version of crosstool-NG:
 https://code.launchpad.net/~linaro-toolchain-dev/crosstool-ng/linaro
