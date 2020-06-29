# ndk-pkg
a package manager for [Android NDK](http://blog.fpliu.com/it/software/GoogleAndroidNDK)

## Installation
* on macOS, using [HomeBrew](http://blog.fpliu.com/it/os/macOS/software/HomeBrew)
```
brew install ndk-pkg
```
* on GNU/Linux, using [LinuxBrew](http://blog.fpliu.com/it/software/LinuxBrew)
```
brew install ndk-pkg
```

## zsh-completion for ndk-pkg
I have provide a zsh-completion script for ndk-pkg. when you've typed `ndk-pkg` then type `TAB` key, it will auto complete the rest for you.

## ndk-pkg command usage
* print the help infomation of ndk-pkg command
```
ndk-pkg -h
ndk-pkg --help
ndk-pkg help
```

* print the version of ndk-pkg and NDK
```
ndk-pkg -V
ndk-pkg --version
ndk-pkg version
```

* search packages can be installed
```
ndk-pkg search curl
```

* print the basic infomation of packages
```
ndk-pkg info curl
ndk-pkg info curl openssl
```

* install packages
```
ndk-pkg install curl
ndk-pkg install curl --target-api=21
```

* uninstall packages
```
ndk-pkg uninstall curl
ndk-pkg uninstall openssl
```

* view the formula source code of a package
```
ndk-pkg cat curl
```

* edit the formula source code of a package
```
ndk-pkg edit curl
```

* list the supported target abis
```
ndk-pkg list target-abis
```

* list the supported target api-levels
```
ndk-pkg list target-apis
```

* list the available packages
```
ndk-pkg list available
```

* list the installed packages
```
ndk-pkg list installed
```

* list contents of a installed package directory in a tree-like format.
```
ndk-pkg tree curl
```

* print the environment variable settings
```
ndk-pkg env curl --target-api=21 --target-abi=armeabi-v7a
ndk-pkg env curl --target-api=21 --target-abi=arm64-v8a
ndk-pkg env curl --target-api=21 --target-abi=x86
ndk-pkg env curl --target-api=21 --target-abi=x86_64
```

* update the [formula repository](https://github.com/leleliu008/ndk-pkg-formula)
```
ndk-pkg update
```

* cleanup the unused cache
```
ndk-pkg cleanup
```

