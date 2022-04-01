# ndk-pkg
a package manager for [Android NDK](https://developer.android.google.cn/ndk) to build C/C++/Rust/Go project.

## ndk-pkg supports system
|HostOS|SubSystem|supported|recommended|summary|
|------|---------|---------|-----------|-------|
[Android](https://www.android.com/)|[Termux](https://termux.com/)|✔︎|❌|not fully tested|
[macOS](https://www.apple.com.cn/mac/)||✔︎|✔︎|tested with [Github Actions](https://github.com/features/actions)|
[Ubuntu](https://ubuntu.com/)||✔︎|✔︎|tested with [Github Actions](https://github.com/features/actions)|
[GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html)|[glibc](http://www.gnu.org/software/libc/)|✔︎|✔︎|tested with [Github Actions](https://github.com/features/actions)|
[GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html)|[musl-libc](http://musl.libc.org/)|❌|❌|no available [Android NDK](https://developer.android.google.cn/ndk)|
[FreeBSD](https://www.freebsd.org/)||❌|❌|no available  [Android NDK](https://developer.android.google.cn/ndk)|
[OpenBSD](https://www.openbsd.org/)||❌|❌|no available [Android NDK](https://developer.android.google.cn/ndk)|
[NetBSD](https://www.netbsd.org/)||❌|❌|no available [Android NDK](https://developer.android.google.cn/ndk)|
[Windows](https://www.microsoft.com/en-us/windows/)|[Cygwin](http://cygwin.org/)|❌|❌||
[Windows](https://www.microsoft.com/en-us/windows/)|[MSYS2](https://www.msys2.org/)|❌|❌||
[Windows](https://www.microsoft.com/en-us/windows/)|[WSL](https://docs.microsoft.com/en-us/windows/wsl/)|✔︎|✔︎|tested with [Github Actions](https://github.com/features/actions)|

## Install ndk-pkg via HomeBrew

```bash
brew tap leleliu008/fpliu
brew install ndk-pkg
```

## Install ndk-pkg via cURL on UNIX
```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/bin/ndk-pkg
chmod a+x ndk-pkg
mv ndk-pkg /usr/local/bin/
```

## Install ndk-pkg via cURL on Termux
```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/bin/ndk-pkg
chmod a+x ndk-pkg
mv ndk-pkg /data/data/com.termux/files/usr/bin/
```

## Install ndk-pkg on WSL

**Note**:
- `WSL2` is recommended. [Comparing WSL 1 and WSL 2](https://docs.microsoft.com/en-us/windows/wsl/compare-versions)

- `Ubuntu-20.04` distrbution is recommended. [Available WSL distribution](https://docs.microsoft.com/en-us/windows/wsl/install-manual#downloading-distributions)

- `HomeBrew` package manager is recommended. [HomeBrew homepage](https://brew.sh/)

- DO NOT RUN AS `root`. Running as root is extremely dangerous. Homebrew does not run as root.

- I assume that `WSL` has been installed, if not, you can follow the step-by-step instructions to install it. [Install WSL](https://docs.microsoft.com/en-us/windows/wsl/install)

**/etc/wsl.conf**
```
[network]
hostname = ubuntu
generateHosts = false

[automount]
enabled = true
root = /mnt/
options = "metadata,umask=22,fmask=11"
mountFsTab = true
```

_instructions installing ndk-pkg on WSL is same as described in section [Install ndk-pkg via HomeBrew](https://github.com/leleliu008/ndk-pkg#install-ndk-pkg-via-homebrew) and [Install ndk-pkg via cURL on UNIX](https://github.com/leleliu008/ndk-pkg#install-ndk-pkg-via-curl-on-unix)_

**Things You Should Do Immediately After Installing ndk-pkg on WSL** :
```bash
sudo install -o $(whoami) -d /mnt/d/ndk-pkg
ln -sf /mnt/d/ndk-pkg ~/.ndk-pkg
```


## Running ndk-pkg in Docker
```bash
mkdir -p ~/.ndk-pkg

docker create -it --name ndk-pkg -v ~/.ndk-pkg:/root/.ndk-pkg fpliu/ndk-pkg

# TO CHINESE USER, if VPN is not being used, please use fowlling instruction
docker create -it --name ndk-pkg -v ~/.ndk-pkg:/root/.ndk-pkg fpliu/ndk-pkg:china

docker start ndk-pkg

docker exec -it ndk-pkg ndk-pkg update
docker exec -it ndk-pkg ndk-pkg install curl
```

**It is strongly recommended to run `ndk-pkg` in docker container to keep your host environment clean.**



## Integrate with CMake
**step1** : fetch [ndk-pkg.cmake](https://github.com/leleliu008/ndk-pkg/blob/master/ndk-pkg.cmake) to the directory where your Android project's CMakeLists.txt is located in
```bash
#method1
ndk-pkg integrate cmake --output-dir=/path/of/your/android/project

#method2
curl -L -o /path/of/your/android/project/ndk-pkg.cmake https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/ndk-pkg.cmake

#method3
Invoke-WebRequest -OutFile /path/of/your/android/project/ndk-pkg.cmake https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/ndk-pkg.cmake
```

**step2** : add following code to your Android project's CMakeLists.txt
```cmake
include(ndk-pkg.cmake)
```

**step3** : invoke [find_package(PKG)](https://cmake.org/cmake/help/latest/command/find_package.html) command in your Android project's CMakeLists.txt
```cmake
find_package(curl)
if (curl_FOUND)
    target_include_directories(xx PRIVATE ${CURL_INCLUDE_DIR})
    target_link_libraries     (xx PRIVATE ${CURL_LIBRARY})
endif()
```


## ndk-pkg command usage
*   **show help of this command**
        
        ndk-pkg -h
        ndk-pkg --help
        
*   **show version of this command**

        ndk-pkg -V
        ndk-pkg --version
        
*   show home directory of this software

        ndk-pkg --homedir

*   show home webpage of this software

        ndk-pkg --homepage

*   show current machine os and [Android NDK](https://developer.android.google.cn/ndk) toolchain info

        ndk-pkg env

*   integrate `zsh-completion` script

        ndk-pkg integrate zsh
        ndk-pkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
        ndk-pkg integrate zsh -x
        ndk-pkg integrate zsh --china
        ndk-pkg integrate zsh --china -x
        
    I have provide a zsh-completion script for `ndk-pkg`. when you've typed `ndk-pkg` then type `TAB` key, it will auto complete the rest for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`


*   update the formula repositories

        ndk-pkg update
        
    **Note:** this software supports multi formula repositories. Offical formula repository is [ndk-pkg-formula-repository](https://github.com/leleliu008/ndk-pkg-formula-repository)

*   install [Android NDK](https://developer.android.google.cn/ndk)

        ndk-pkg ndkmanager install r23b
        
    **Note**: In general, you don't need to run this command in advance. `ndk-pkg install <PKG>` will run this command if [Android NDK](https://developer.android.google.cn/ndk) not found on your build machine.

*   search packages can be installed
        
        ndk-pkg search curl
        ndk-pkg search lib
        
*   show infomation of the given package or all available packages
        
        ndk-pkg info curl
        ndk-pkg info curl version
        ndk-pkg info curl summary
        ndk-pkg info curl webpage
        ndk-pkg info curl git.url
        ndk-pkg info curl src.url
        ndk-pkg info curl installed-dir
        ndk-pkg info curl installed-metadata
        ndk-pkg info curl installed-datetime-unix
        ndk-pkg info curl installed-datetime-formatted
        ndk-pkg info curl installed-pkg-version
        ndk-pkg info curl installed-files
        ndk-pkg info curl installed-abis
        ndk-pkg info curl --json
        ndk-pkg info curl --json | jq .
        ndk-pkg info @all
        ndk-pkg info @all --json
        ndk-pkg info @all --json | jq .
        

    For more keys, please see [README.md](https://github.com/leleliu008/ndk-pkg-formula-repository/blob/master/README.md#the-function-must-be-invoked-on-top-of-the-formula)

*   install packages
        
        ndk-pkg install curl
        ndk-pkg install curl bzip2 --min-sdk-api-level=21
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v -x
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v -x --dry-run
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v -x --dry-run --keep-work-dir
        ndk-pkg install @all
        
*   reinstall packages
        
        ndk-pkg reinstall curl
        ndk-pkg reinstall curl bzip2 --min-sdk-api-level=21 -v
        
*   uninstall packages

        ndk-pkg uninstall curl
        ndk-pkg uninstall curl bzip2
        
*   upgrade the outdated packages

        ndk-pkg upgrade
        ndk-pkg upgrade curl
        ndk-pkg upgrade curl bzip2 --min-sdk-api-level=21 -v
        
*   upgrade this software

        ndk-pkg upgrade @self
        ndk-pkg upgrade @self -x
        ndk-pkg upgrade @self --china
        ndk-pkg upgrade @self --china -x
        

*   list the avaliable formula repos

        ndk-pkg formula-repo list

*   add a new formula repo

        ndk-pkg formula-repo add my_repo https://github.com/leleliu008/ndk-pkg-formula-repository.git

*   delete a existing formula repo

        ndk-pkg formula-repo del my_repo

*   view the formula of the given package
        
        ndk-pkg formula view curl
        
*   edit the formula of the given package
        
        ndk-pkg formula edit curl
        
*   create the formula of the given package
        
        ndk-pkg formula create curl
        
*   delete the formula of the given package
        
        ndk-pkg formula delete curl
        
*   rename the formula of the given package to new name
        
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
        
        ndk-pkg ls-available
        
*   list the installed packages
        
        ndk-pkg ls-installed
        
*   list the outdated packages
        
        ndk-pkg ls-outdated
        
*   is the given package available ?
        
        ndk-pkg is-available curl
        ndk-pkg is-available curl ge 7.50.0
        ndk-pkg is-available curl gt 7.50.0
        ndk-pkg is-available curl le 7.50.0
        ndk-pkg is-available curl lt 7.50.0
        ndk-pkg is-available curl eq 7.50.0
        ndk-pkg is-available curl ne 7.50.0
        
*   is the given package installed ?
        
        ndk-pkg is-installed curl
        
*   is the given package outdated ?
        
        ndk-pkg is-outdated curl
        
*   list files of the given installed package in a tree-like format.
        
        ndk-pkg tree curl
        ndk-pkg tree curl --dirsfirst
        ndk-pkg tree curl -L 3
        ndk-pkg tree curl -L 3 --dirsfirst
        
*   download formula resources of the given package to the cache
        
        ndk-pkg fetch curl
        
*   download formula resources of all available packages to the cache

        ndk-pkg fetch @all

*   show logs of the given installed package
        
        ndk-pkg logs curl armeabi-v7a
        ndk-pkg logs curl arm64-v8a
        ndk-pkg logs curl x86
        ndk-pkg logs curl x86_64
        
*   pack the given installed package
        
        ndk-pkg pack curl
        ndk-pkg pack curl --type=tar.gz
        ndk-pkg pack curl --type=tar.xz
        ndk-pkg pack curl --type=tar.bz2
        ndk-pkg pack curl --type=zip
        ndk-pkg pack curl --type=7z
        
*   show or open the homepage of the given package or this project
        
        ndk-pkg homepage
        ndk-pkg homepage --open
        ndk-pkg homepage --open curl
        ndk-pkg homepage curl --open
        
*   show the depended packages of the given package
        
        ndk-pkg depends curl
        
*   cleanup the unused cache
        
        ndk-pkg cleanup
        
