# ndk-pkg
a package manager for [Android NDK](https://developer.android.google.cn/ndk) to build C/C++/Rust/Go project.

## Install ndk-pkg via package manager

|OS|PackageManager|Installation Instructions|
|-|-|-|
|`macOS`|[HomeBrew](http://blog.fpliu.com/it/os/macOS/software/HomeBrew)|`brew tap leleliu008/fpliu`<br>`brew install ndk-pkg`|
|`GNU/Linux`|[LinuxBrew](http://blog.fpliu.com/it/software/LinuxBrew)|`brew tap leleliu008/fpliu`<br>`brew install ndk-pkg`|
|`ArchLinux`<br>`ArcoLinux`<br>`Manjaro Linux`<br>`Windows/msys2`|[pacman](http://blog.fpliu.com/it/software/pacman)|`curl -LO https://github.com/leleliu008/ndk-pkg/releases/download/v0.1.0/ndk-pkg-0.1.0-1-any.pkg.tar.gz`<br>`pacman -Syyu --noconfirm`<br>`pacman -U ndk-pkg-0.1.0-1-any.pkg.tar.gz`|
|`Windows/WSL`|[LinuxBrew](http://blog.fpliu.com/it/software/LinuxBrew)|`brew tap leleliu008/fpliu`<br>`brew install ndk-pkg`|

## Install ndk-pkg via running shell script
```
sh -c "$(curl -fsSL https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/install.sh)"
```

## Install ndk-pkg via cURL
```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/bin/ndk-pkg
chmod a+x ndk-pkg
mv ndk-pkg /usr/local/bin/

# following instrutions is optional, these instructions only worked in zsh
ndk-pkg integrate zsh
autoload -U compinit && compinit
```

## ndk-pkg command usage
*   print the help infomation of `ndk-pkg` command
        
        ndk-pkg -h
        ndk-pkg --help
        
*   print the version of `ndk-pkg`
        
        ndk-pkg -V
        ndk-pkg --version
        
*   show current machine os and [Android NDK](https://developer.android.google.cn/ndk) toolchain info

        ndk-pkg env

*   integrate `zsh-completion` script

        ndk-pkg integrate zsh
        ndk-pkg integrate zsh -x
        ndk-pkg integrate zsh --china
        ndk-pkg integrate zsh --china -x
        
    I have provide a zsh-completion script for `ndk-pkg`. when you've typed `ndk-pkg` then type `TAB` key, it will auto complete the rest for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`


*   update the formula repositories

        ndk-pkg update
        
    **Note:** this software supports multi formula repositories. Offical formula repository is [ndk-pkg-formula-repository](https://github.com/leleliu008/ndk-pkg-formula-repository)

*   search packages can be installed
        
        ndk-pkg search curl
        ndk-pkg search lib
        
*   print the infomation of a package or all available packages
        
        ndk-pkg info curl
        ndk-pkg info curl version
        ndk-pkg info curl summary
        ndk-pkg info curl webpage
        ndk-pkg info curl src.git
        ndk-pkg info @all
        ndk-pkg info @all --json
        ndk-pkg info @all --json | jq .
        

    more keys please read [README.md](https://github.com/leleliu008/ndk-pkg-formula-repository/blob/master/README.md)

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
        
*   upgrade this software

        ndk-pkg upgrade @self
        ndk-pkg upgrade @self -x
        ndk-pkg upgrade @self --china
        ndk-pkg upgrade @self --china -x
        

*   list the avaliable formula repos

        ndk-pkg formula repo list

*   add a new formula repo

        ndk-pkg formula repo add my_repo https://github.com/leleliu008/ndk-pkg-formula-repository.git

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
        
*   list the supported target abis
        
        ndk-pkg target abis

*   list the supported target archs
        
        ndk-pkg target archs

*   list the supported target triples
        
        ndk-pkg target triples

*   list the supported sdk api-levels
        
        ndk-pkg target levels
        
*   list the available packages
        
        ndk-pkg ls available
        
*   list the installed packages
        
        ndk-pkg ls installed
        
*   list the outdated packages
        
        ndk-pkg ls outdated
        
*   is the specified package available ?
        
        ndk-pkg is available curl
        ndk-pkg is available curl ge 7.50.0
        ndk-pkg is available curl gt 7.50.0
        ndk-pkg is available curl le 7.50.0
        ndk-pkg is available curl lt 7.50.0
        ndk-pkg is available curl eq 7.50.0
        ndk-pkg is available curl ne 7.50.0
        
*   is the specified package installed ?
        
        ndk-pkg is installed curl
        
*   is the specified package outdated ?
        
        ndk-pkg is outdated curl
        
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
        
*   show the homepage of the given formula or this project
        
        ndk-pkg homepage show
        ndk-pkg homepage show curl
        
*   open the homepage of the given formula or this project
        
        ndk-pkg homepage open
        ndk-pkg homepage open curl
        
*   show the installation direcotory of a formula or the ndk-pkg home
        
        ndk-pkg prefix
        ndk-pkg prefix curl
        
*   show the depended packages by a package
        
        ndk-pkg depends curl
        
*   cleanup the unused cache
        
        ndk-pkg cleanup
        
