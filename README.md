# ndk-pkg
a package manager for [Android NDK](https://developer.android.google.cn/ndk) to build C/C++/Rust/Go project.

## supported host operating systems and subsystems
|HostOS|HostArch|SubSystem|recommended|summary|
|------|--------|---------|-----------|-------|
[Android](https://www.android.com/)|`aarch64`|[Termux](https://termux.com/)|❌|not fully tested|
[macOS](https://www.apple.com.cn/mac/)|`x86_64` `arm64`||✔︎|tested with [Github Actions](https://github.com/features/actions)|
[Ubuntu](https://ubuntu.com/)|`x86_64`||✔︎|tested with [Github Actions](https://github.com/features/actions)|
[GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html)|`x86_64`|[glibc](http://www.gnu.org/software/libc/)|✔︎|tested with [Github Actions](https://github.com/features/actions)|
[Windows](https://www.microsoft.com/en-us/windows/)|`x86_64`|[WSL](https://docs.microsoft.com/en-us/windows/wsl/)|✔︎|tested with [Github Actions](https://github.com/features/actions)|

## unsupported host operating systems and subsystems
|HostOS|SubSystem|reason|
|------|---------|------|
[GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html)|[musl-libc](http://musl.libc.org/)|no available [Android NDK](https://developer.android.google.cn/ndk)|
[FreeBSD](https://www.freebsd.org/)||no available  [Android NDK](https://developer.android.google.cn/ndk)|
[OpenBSD](https://www.openbsd.org/)||no available [Android NDK](https://developer.android.google.cn/ndk)|
[NetBSD](https://www.netbsd.org/)||no available [Android NDK](https://developer.android.google.cn/ndk)|
[Windows](https://www.microsoft.com/en-us/windows/)|[Cygwin](http://cygwin.org/)|CMake: Builds hosted on 'CYGWIN' not supported. [Android-Determine.cmake](https://github.com/Kitware/CMake/blob/master/Modules/Platform/Android-Determine.cmake#L271-L291)|
[Windows](https://www.microsoft.com/en-us/windows/)|[MSYS2](https://www.msys2.org/)|CMake: Builds hosted on 'MSYS' not supported. [Android-Determine.cmake](https://github.com/Kitware/CMake/blob/master/Modules/Platform/Android-Determine.cmake#L271-L291)|

## Install ndk-pkg via Docker
**step1. create the ndk-pkg docker container**
```bash
mkdir -p ~/.ndk-pkg
mkdir -p ~/.m2

docker create -it --name ndk-pkg -v ~/.ndk-pkg:/root/.ndk-pkg -v ~/.m2:/root/.m2 fpliu/ndk-pkg
```

**step2. start the ndk-pkg docker container**
```bash
docker start ndk-pkg
```

**step3. run `ndk-pkg` command in the ndk-pkg docker container**
```bash
docker exec -it ndk-pkg ndk-pkg upgrade @self
docker exec -it ndk-pkg ndk-pkg update
docker exec -it ndk-pkg ndk-pkg install curl
```

**Note:**
- This is the recommended way to install `ndk-pkg`, beacause many softwares will be automatically installed when running `ndk-pkg` command, running `ndk-pkg` command in a docker container will keep your host environment clean.
- you can use `podman` instead of `docker`
- chinese user may want to use `fpliu/ndk-pkg:china` instead of `fpliu/ndk-pkg`


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

**Note:**
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


## Integrate with Android Gradle Plugin
**step1. enables prefab feature in build.gradle**
```gradle
android {
    buildFeatures {
        prefab true
    }
}
```

**step2. enables mavenCentral repository in build.gradle**
```gradle
repositories {
    mavenCentral()
}
```

**step3. add dependencies in build.gradle**
```gradle
dependencies {
    implementation 'com.fpliu.ndk.pkg.prefab.android.21:libpng:1.6.37'
}
```

**step4. invoke [find_package(PACKAGE-NAME [REQUIRED] CONFIG)](https://cmake.org/cmake/help/latest/command/find_package.html) command in your Android project's CMakeLists.txt**
```cmake
find_package(curl REQUIRED CONFIG)
target_link_libraries(xx curl::libpng.a)
```
or
```cmake
find_package(curl CONFIG)
if (curl_FOUND)
    target_link_libraries(xx curl::libpng.a)
endif()
```

**step5. configure C++ standard and STL in build.gradle**

If you link a shared library that depends on `libc++_shared.so`, then your Android app should use `libc++_shared.so` too.
```gradle
android {
    defaultConfig {
        externalNativeBuild {
            cmake {
                arguments '-DANDROID_STL=c++_shared'
                cppFlags "-std=c++17"
            }
        }
    }
}
```

**Note:**
- you can look for packages that have been published to `mavenCentral` via visiting follwoing websites:
    - https://repo1.maven.org/maven2/com/fpliu/ndk/pkg/prefab/android/21/
    - https://search.maven.org/search?q=com.fpliu.ndk.pkg.prefab
- If packages that have been published to `mavenCentral` doesn't meet your needs, you can install package then deploy it to `mavenLocal` via running following commands:
    ```bash
    ndk-pkg install libpng --min-sdk-api-level=21
    ndk-pkg deploy  libpng 21 mavenLocal
    ```
    enables `mavenLocal` repository in `build.gradle`
    ```gradle
    repositories {
        mavenLocal()
    }
    ```

- Every package provides several cmake imported targets and each target has form: `${PACKAGE_NAME}::${LIBRARY_FILENAME}`

**References:**

- https://github.com/google/prefab
- https://developer.android.com/studio/build/dependencies?agpversion=4.1#using-native-dependencies

**Examples:**

- https://github.com/leleliu008/android-calendar-for-the-aged


## Integrate with CMake directly
**Note:**
- `Integrate with Android Gradle Plugin` is the recommended way of using this software. If you do not use Android Gradle Plugin's prefab feature for some reasons, you can integrate this software with CMake directly.

**step1. fetch [ndk-pkg.cmake](https://github.com/leleliu008/ndk-pkg/blob/master/ndk-pkg.cmake) to the directory where your Android project's CMakeLists.txt is located in**
```bash
#method1
ndk-pkg integrate cmake --output-dir=/path/of/your/android/project

#method2
curl -L -o /path/of/your/android/project/ndk-pkg.cmake https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/ndk-pkg.cmake

#method3
Invoke-WebRequest -OutFile /path/of/your/android/project/ndk-pkg.cmake https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/ndk-pkg.cmake
```

**step2. add following code to your Android project's CMakeLists.txt**
```cmake
if (ANDROID)
    include(ndk-pkg.cmake)
endif()
```

**step3. invoke [find_package(PACKAGE-NAME [REQUIRED] CONFIG)](https://cmake.org/cmake/help/latest/command/find_package.html) command in your Android project's CMakeLists.txt**
```cmake
find_package(curl REQUIRED CONFIG)
target_link_libraries(xx curl::libcurl.so)
```
or
```cmake
find_package(curl CONFIG)
if (curl_FOUND)
    target_link_libraries(xx curl::libcurl.so)
endif()
```

**Note:**
- Every package provides several cmake imported targets and each target has form: `${PACKAGE_NAME}::${LIBRARY_FILE_NAME}`
- If you want to know what cmake imported targets are provided by `${PACKAGE_NAME}`, you can look at `~/.ndk-pkg/install.d/android/${ANDROID_PLATFORM_LEVEL}/${PACKAGE_NAME}/${ANDROID_ABI}/lib-no-versioning/cmake/${PACKAGE_NAME}/${PACKAGE_NAME}Config.cmake`


## ~/.ndk-pkg
all relevant dirs and files are located in `~/.ndk-pkg` directory.


## environment variables
if environment variable `ANDROID_NDK_HOME` is set and `is_a_valid_android_ndk_root_dir "$ANDROID_NDK_HOME"` is true, then it will be used.

if environment variable `ANDROID_NDK_ROOT` is set and `is_a_valid_android_ndk_root_dir "$ANDROID_NDK_ROOT"` is true, then it will be used.

if environment variable `ANDROID_HOME` is set and `is_a_valid_android_ndk_root_dir "$ANDROID_HOME/ndk-bundle"` is true, then it will be used.

```bash
is_a_valid_android_ndk_root_dir() {
    NATIVE_OS_KIND=$(uname -s | tr A-Z a-z)
    NATIVE_OS_ARCH=$(uname -m)

    if [ "$NATIVE_OS_KIND" = darwin ] ; then
        ANDROID_NDK_TOOLCHAIN_HOST_TAG='darwin-x86_64'
    else
        ANDROID_NDK_TOOLCHAIN_HOST_TAG="$NATIVE_OS_KIND-$NATIVE_OS_ARCH"
    fi

    [ -n "$1" ] &&
    [ -d "$1" ] &&
    [ -f "$1/source.properties" ] &&
    [ -f "$1/build/cmake/android.toolchain.cmake" ] &&
    [ -d "$1/toolchains/llvm/prebuilt/$ANDROID_NDK_TOOLCHAIN_HOST_TAG" ] &&
    [ -d "$1/toolchains/llvm/prebuilt/$ANDROID_NDK_TOOLCHAIN_HOST_TAG/bin" ] &&
    [ -d "$1/toolchains/llvm/prebuilt/$ANDROID_NDK_TOOLCHAIN_HOST_TAG/sysroot" ]
}
```

**Note**: `Android NDK` will be automatically installed if it is not found when running `ndk-pkg install <PACKAHE-NAME>...`


## ndk-pkg command usage
*   **show help of this command**
        
        ndk-pkg -h
        ndk-pkg --help
        
*   **show version of this command**

        ndk-pkg -V
        ndk-pkg --version
        

*   **show current machine os and [Android NDK](https://developer.android.google.cn/ndk) toolchain info**

        ndk-pkg env

*   **integrate `zsh-completion` script**

        ndk-pkg integrate zsh
        ndk-pkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
        ndk-pkg integrate zsh -x
        ndk-pkg integrate zsh --china
        ndk-pkg integrate zsh --china -x
        
    I have provide a zsh-completion script for `ndk-pkg`. when you've typed `ndk-pkg` then type `TAB` key, it will auto complete the rest for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`


*   **update formula repositories**

        ndk-pkg update
        
    **Note:** this software supports multi formula repositories. Offical formula repository is [ndk-pkg-formula-repository](https://github.com/leleliu008/ndk-pkg-formula-repository)

*   **install [Android NDK](https://developer.android.google.cn/ndk)**

        ndk-pkg ndkmanager install r23b
        
    **Note**: In general, you don't need to run this command in advance. `ndk-pkg install <PKG>` will run this command if [Android NDK](https://developer.android.google.cn/ndk) not found on your build machine.

*   **search packages**
        
        ndk-pkg search curl
        ndk-pkg search lib
        
*   **show infomation of the given package**
        
        ndk-pkg info curl
        ndk-pkg info curl version
        ndk-pkg info curl summary
        ndk-pkg info curl webpage
        ndk-pkg info curl git.url
        ndk-pkg info curl src.url

        ndk-pkg info curl installed-dir                    --min-sdk-api-level=21
        ndk-pkg info curl installed-metadata-filepath      --min-sdk-api-level=21
        ndk-pkg info curl installed-metadata-content-raw   --min-sdk-api-level=21
        ndk-pkg info curl installed-metadata-content-json  --min-sdk-api-level=21
        ndk-pkg info curl installed-metadata-content-yaml  --min-sdk-api-level=21
        ndk-pkg info curl installed-datetime-unix          --min-sdk-api-level=21
        ndk-pkg info curl installed-datetime-formatted     --min-sdk-api-level=21
        ndk-pkg info curl installed-pkg-version            --min-sdk-api-level=21
        ndk-pkg info curl installed-files                  --min-sdk-api-level=21
        ndk-pkg info curl installed-abis                   --min-sdk-api-level=21

        ndk-pkg info curl --json
        ndk-pkg info curl --json | jq .

        ndk-pkg info curl --yaml
        ndk-pkg info curl --yaml | yq .

        ndk-pkg info curl --shell

        ndk-pkg info @all

        ndk-pkg info @all --shell

        ndk-pkg info @all --json
        ndk-pkg info @all --json | jq .

        ndk-pkg info @all --yaml
        ndk-pkg info @all --yaml | yq .
        

    For more keys, please see [README.md](https://github.com/leleliu008/ndk-pkg-formula-repository/blob/master/README.md#the-function-must-be-invoked-on-top-of-the-formula)

*   **show packages that are depended by the given package**
        
        ndk-pkg depends curl
        ndk-pkg depends curl --format=list
        ndk-pkg depends curl --format=json
        ndk-pkg depends curl --format=dot
        ndk-pkg depends curl --format=txt
        ndk-pkg depends curl --format=png -o curl-dependencies.png
        
*   **download formula resources of the given package to the local cache**
        
        ndk-pkg fetch curl
        ndk-pkg fetch @all

*   **install packages**
        
        ndk-pkg install curl
        ndk-pkg install curl bzip2 --min-sdk-api-level=21
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v -x
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v -x --dry-run
        ndk-pkg install curl bzip2 --min-sdk-api-level=21 --jobs=4 -v -x --dry-run --keep-work-dir
        ndk-pkg install @all
        
*   **reinstall packages**
        
        ndk-pkg reinstall curl
        ndk-pkg reinstall curl bzip2 --min-sdk-api-level=21 -v
        
*   **uninstall packages**

        ndk-pkg uninstall curl
        ndk-pkg uninstall curl bzip2

        ndk-pkg uninstall curl       --min-sdk-api-level=21
        ndk-pkg uninstall curl bzip2 --min-sdk-api-level=21
        
*   **upgrade the outdated packages**

        ndk-pkg upgrade
        ndk-pkg upgrade curl
        ndk-pkg upgrade curl bzip2 --min-sdk-api-level=21 -v
        
*   **upgrade this software**

        ndk-pkg upgrade @self
        ndk-pkg upgrade @self -x
        ndk-pkg upgrade @self --china
        ndk-pkg upgrade @self --china -x
        

*   **list the avaliable formula repositories**

        ndk-pkg formula-repo list

*   **add a new formula repository**

        ndk-pkg formula-repo add my_repo https://github.com/leleliu008/ndk-pkg-formula-repository.git

*   **delete a existing formula repository**

        ndk-pkg formula-repo del my_repo

*   **view the formula of the given package**
        
        ndk-pkg formula view curl
        
*   **edit the formula of the given package**
        
        ndk-pkg formula edit curl
        
*   **create the formula of the given package**
        
        ndk-pkg formula create curl
        
*   **delete the formula of the given package**
        
        ndk-pkg formula delete curl
        
*   **rename the formula of the given package to new name**
        
        ndk-pkg formula rename curl curl7
        
*   **list the supported target abis**
        
        ndk-pkg ls-target-abis

*   **list the supported target archs**
        
        ndk-pkg ls-target-archs

*   **list the supported target triples**
        
        ndk-pkg ls-target-triples

*   **list the supported sdk api-levels**
        
        ndk-pkg ls-target-levels
        
*   **list the available packages**
        
        ndk-pkg ls-available
        
*   **list the installed packages**
        
        ndk-pkg ls-installed
        ndk-pkg ls-installed --min-sdk-api-level=21
        
*   **list the outdated packages**
        
        ndk-pkg ls-outdated
        ndk-pkg ls-outdated --min-sdk-api-level=21
        
*   **is the given package available ?**
        
        ndk-pkg is-available curl
        ndk-pkg is-available curl ge 7.50.0
        ndk-pkg is-available curl gt 7.50.0
        ndk-pkg is-available curl le 7.50.0
        ndk-pkg is-available curl lt 7.50.0
        ndk-pkg is-available curl eq 7.50.0
        ndk-pkg is-available curl ne 7.50.0
        
*   **is the given package installed ?**
        
        ndk-pkg is-installed curl 21
        
*   **is the given package outdated ?**
        
        ndk-pkg is-outdated  curl 21
        
*   **list files of the given installed package in a tree-like format**
        
        ndk-pkg tree curl 21
        ndk-pkg tree curl 21 --dirsfirst
        ndk-pkg tree curl 21 -L 3
        ndk-pkg tree curl 21 -L 3 --dirsfirst
        
*   **show logs of the given installed package**
        
        ndk-pkg logs curl 21 armeabi-v7a
        ndk-pkg logs curl 21 arm64-v8a
        ndk-pkg logs curl 21 x86
        ndk-pkg logs curl 21 x86_64
        
*   **pack the given installed package**
        
        ndk-pkg pack curl 21 tar.gz
        ndk-pkg pack curl 21 tar.xz
        ndk-pkg pack curl 21 tar.bz2
        ndk-pkg pack curl 21 7z
        ndk-pkg pack curl 21 zip
        ndk-pkg pack curl 21 aar
        
*   **pack the given installed package as prefab aar and then deploy the prefab aar to Maven local repository**
        
        ndk-pkg deploy curl 21 mavenLocal
        ndk-pkg deploy curl 21 mavenLocal -d
        ndk-pkg deploy curl 21 mavenLocal -x

*   **pack the given installed package as prefab aar and then deploy the prefab aar to Maven remote repository**
        
        ndk-pkg deploy curl 21 mavenRemote    < ~/OSSRH-config
        ndk-pkg deploy curl 21 mavenRemote -d < ~/OSSRH-config
        ndk-pkg deploy curl 21 mavenRemote -x < ~/OSSRH-config
        ndk-pkg deploy curl 21 mavenRemote -x <<EOF
        SERVER_ID=OSSRH
        SERVER_URL=https://s01.oss.sonatype.org/service/local/repositories/comfpliu-1025/content
        SERVER_USERNAME=leleliu008
        SERVER_PASSWORD=xx
        GPG_PASSPHRASE=yy
        EOF

*   **show or open the homepage of this project**
        
        ndk-pkg homepage
        ndk-pkg homepage --open

*   **show or open the homepage of the given package**
        
        ndk-pkg homepage curl
        ndk-pkg homepage curl --open

*   **cleanup the unused cached files**
        
        ndk-pkg cleanup
        
