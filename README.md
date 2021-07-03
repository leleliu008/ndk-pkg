# ndk-pkg
a package manager for [Android NDK](https://developer.android.google.cn/ndk) to build C/C++/Rust/Go project.

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


## Give a Star!
𝙄𝙛 𝙮𝙤𝙪 ❤𝙩𝙝𝙞𝙨 𝙥𝙧𝙤𝙟𝙚𝙘𝙩, 𝙥𝙡𝙚𝙖𝙨𝙚 𝙜𝙞𝙫𝙚 𝙞𝙩 𝙖 𝙨𝙩𝙖𝙧 <span style='color:red;font-size:1.2em'>★</span> . 𝙏𝙝𝙖𝙣𝙠𝙨!


## ndk-pkg command usage
*   print the help infomation of `ndk-pkg` command
        
        ndk-pkg -h
        ndk-pkg --help
        
*   print the version of `ndk-pkg`
        
        ndk-pkg -V
        ndk-pkg --version
        
*   show [Android NDK](https://developer.android.google.cn/ndk) toolchain info

        ndk-pkg toolchain

*   integrate `zsh-completion` script

        ndk-pkg integrate zsh
        
*   update the [ndk-pkg-formula repository](https://github.com/leleliu008/ndk-pkg-formula)

        ndk-pkg update
        
*   search packages can be installed
        
        ndk-pkg search curl
        ndk-pkg search lib
        
*   print the basic infomation of packages
        
        ndk-pkg info curl
        ndk-pkg info curl openssl
        
*   install packages
        
        ndk-pkg install curl
        ndk-pkg install curl bzip2 --min-sdk-api-level=21
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v -x
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v -x --dry-run
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v -x --dry-run --keep-working-dir
        
*   reinstall packages
        
        ndk-pkg reinstall curl
        ndk-pkg reinstall curl bzip2 --min-sdk-api-level=21 -v
        
*   uninstall packages

        ndk-pkg uninstall curl
        ndk-pkg uninstall curl bzip2
        
*   upgrade the outdated packages

        ndk-pkg upgrade curl
        ndk-pkg upgrade curl bzip2 --min-sdk-api-level=21 -v
        
*   list add avaliable formula repos

        ndk-pkg formula repo list

*   add a new formula repo

        ndk-pkg formula repo add my_repo https://github.com/leleliu008/ndk-pkg-formula.git

*   delete a existing formula repo

        ndk-pkg formula repo del my_repo

*   view the formula of a package
        
        ndk-pkg formula view curl
        
*   edit the formula of a package
        
        ndk-pkg formula edit curl
        
*   create a formula
        
        ndk-pkg formula create curl
        
*   delete a formula
        
        ndk-pkg formula delete curl
        
*   rename a formula to new name
        
        ndk-pkg formula rename curl curl7
        
*   list the supported abis
        
        ndk-pkg list abis
        
*   list the supported sdk api-levels
        
        ndk-pkg list sdk-api-levels
        
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
        
*   get the value of key of a package.

        ndk-pkg get curl version
        ndk-pkg get curl summary
        ndk-pkg get curl webpage
        ndk-pkg get curl src.git

    more keys please read [README.md](https://github.com/leleliu008/ndk-pkg-formula/blob/master/README.md)

*   list contents of a installed package directory in a tree-like format.
        
        ndk-pkg tree curl
        ndk-pkg tree curl -L 3
        
*   download formula resources of a package to the cache
        
        ndk-pkg fetch curl
        
*   print the logs of a installed package
        
        ndk-pkg logs curl armeabi-v7a
        ndk-pkg logs curl arm64-v8a
        ndk-pkg logs curl x86
        ndk-pkg logs curl x86_64
        
*   pack a installed package
        
        ndk-pkg pack curl
        
*   visit the homepage of a formula or the `ndk-pkg` project
        
        ndk-pkg homepage
        ndk-pkg homepage curl
        
*   show the installation direcotory of a formula or the ndk-pkg home
        
        ndk-pkg prefix
        ndk-pkg prefix curl
        
*   show the depended packages by a package
        
        ndk-pkg depends curl
        
*   cleanup the unused cache
        
        ndk-pkg cleanup
        
