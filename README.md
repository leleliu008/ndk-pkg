# ndk-pkg
a package manager for [Android NDK](https://developer.android.google.cn/ndk)

## Install via package manager

|OS|PackageManager|Installation Instructions|
|-|-|-|
|`macOS`|[HomeBrew](http://blog.fpliu.com/it/os/macOS/software/HomeBrew)|`brew tap leleliu008/fpliu`<br>`brew install ndk-pkg`|
|`GNU/Linux`|[LinuxBrew](http://blog.fpliu.com/it/software/LinuxBrew)|`brew tap leleliu008/fpliu`<br>`brew install ndk-pkg`|
|`ArchLinux`<br>`ArcoLinux`<br>`Manjaro Linux`<br>`Windows/msys2`|[pacman](http://blog.fpliu.com/it/software/pacman)|`curl -LO https://github.com/leleliu008/ndk-pkg/releases/download/v0.1.0/ndk-pkg-0.1.0-1-any.pkg.tar.gz`<br>`pacman -Syyu --noconfirm`<br>`pacman -U ndk-pkg-0.1.0-1-any.pkg.tar.gz`|
|`Windows/WSL`|[LinuxBrew](http://blog.fpliu.com/it/software/LinuxBrew)|`brew tap leleliu008/fpliu`<br>`brew install ndk-pkg`|

## Install using shell script
```
sh -c "$(curl -fsSL https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/install.sh)"
```

## zsh-completion for ndk-pkg
I have provide a zsh-completion script for `ndk-pkg`. when you've typed `ndk-pkg` then type `TAB` key, it will auto complete the rest for you.

**Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`

## ndk-pkg command usage
*   print the help infomation of `ndk-pkg` command
        
        ndk-pkg -h
        ndk-pkg --help
        ndk-pkg help
        
*   print the version of `ndk-pkg` and `NDK`
        
        ndk-pkg -V
        ndk-pkg --version
        ndk-pkg version
        
*   search packages can be installed
        
        ndk-pkg search curl
        ndk-pkg search lib
        
*   print the basic infomation of packages
        
        ndk-pkg info curl
        ndk-pkg info curl openssl
        
*   install packages
        
        ndk-pkg install curl
        ndk-pkg install curl bzip2 --target-api=21 -v
        
*   reinstall packages
        
        ndk-pkg reinstall curl
        ndk-pkg reinstall curl bzip2 --target-api=21 -v
        
*   uninstall packages
        
        ndk-pkg uninstall curl
        ndk-pkg uninstall curl bzip2
        
*   upgrade the outdated packages
        
        ndk-pkg upgrade curl
        ndk-pkg upgrade curl bzip2 --target-api=21 -v

*   view the formula source code of a package
        
        ndk-pkg cat curl
        
*   edit the formula source code of a package
        
        ndk-pkg edit curl
        
*   list the supported target abis
        
        ndk-pkg list target-abis
        
*   list the supported target api-levels
        
        ndk-pkg list target-apis
        
*   list the available packages
        
        ndk-pkg list available
        ndk-pkg list available -q
        
*   list the installed packages
        
        ndk-pkg list installed
        
*   list the outdated packages
        
        ndk-pkg list outdated
        
*   is the specified package available ?
        
        ndk-pkg is available curl
        
*   is the specified package installed ?
        
        ndk-pkg is installed curl
        
*   is the specified package outdated ?
        
        ndk-pkg is outdated curl
        
*   list contents of a installed package directory in a tree-like format.
        
        ndk-pkg tree curl
        
*   print the environment variable settings
        
        ndk-pkg env curl --target-api=21 --target-abi=armeabi-v7a - -x
        ndk-pkg env curl --target-api=21 --target-abi=arm64-v8a -x
        ndk-pkg env curl --target-api=21 --target-abi=x86 -x
        ndk-pkg env curl --target-api=21 --target-abi=x86_64 -x

*   update the [formula repository](https://github.com/leleliu008/ndk-pkg-formula)
        
        ndk-pkg update
        
*   download formula resources of a package to the cache
        
        ndk-pkg fetch curl
        
*   print the logs of a installed package
        
        ndk-pkg logs curl
        
*   pack a installed package
        
        ndk-pkg pack curl
        
*   visit the homepage of a formula or the `ndk-pkg` project
        
        ndk-pkg homepage
        ndk-pkg homepage curl
        
*   show the installation direcotory of a formula or the ndk-pkg home
        
        ndk-pkg prefix
        ndk-pkg prefix curl
        
*   cleanup the unused cache
        
        ndk-pkg cleanup
        
