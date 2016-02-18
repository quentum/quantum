# quentum
## Build Instructions (Windows)

Follow the guidelines below for building quentum on Windows.

## Prerequisites

* Windows 7 / Server 2008 R2 or higher
* Visual Studio 2013 with Update 4 or Visual Studio 2015 with Update 1
* Windows 10 SDK
* [Python 2.7](http://www.python.org/download/releases/2.7/)
* [Git](http://git-scm.com)

## Getting the Code

```powershell
$ git clone https://github.com/baiduclient/quentum.git
```

## Bootstrapping

The bootstrap script will download all necessary build dependencies. Notice that we're using `ninja` to build vendor so
there is no Visual Studio project generated.

```powershell
$ cd quentum
$ python script\bootstrap.py
```

## Building

Build x86 Debug targets by default:

```powershell
$ python script\build.py
```

Build Debug and Release targets:

```powershell
$ python script\build.py --configuration Debug Release
```

After building is done, you can find `dll lib` under `vendor\out\Debug` (debug
target) or under `out\Release` (release target).

## 64bit Build

To build for the 64bit target, you need to pass `--target-arch=x64` when running
the build script:

```powershell
$ python script\build.py --configuration Debug Release --target-arch=x64
```
After building is done, you can find `dll lib` under `vendor\out\Debug_x64` (debug
target) or under `out\Release_x64` (release target).
