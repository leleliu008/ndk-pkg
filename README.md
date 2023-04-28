# ndk-pkg

a package manager for [Android NDK](https://developer.android.google.cn/ndk) to build projects that are written in C/C++/Rust/Golang.

**Note**: This project is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.

## supported platforms

|HostOS|HostArch|SubSystem|recommended|summary|
|------|--------|---------|-----------|-------|
[Android](https://www.android.com/)|`aarch64`|[Termux](https://termux.com/)|❌|not fully tested|
[macOS](https://www.apple.com.cn/mac/)|`x86_64` `arm64`||✔︎|tested with `Github Actions`|
[GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html)|`x86_64`|[glibc](http://www.gnu.org/software/libc/)|✔︎|tested with `Github Actions`|
[Windows](https://www.microsoft.com/en-us/windows/)|`x86_64`|[WSL](https://docs.microsoft.com/en-us/windows/wsl/)|✔︎|tested with `Github Actions`|

**Note**:

- This software do NOT work on [musl-libc](http://musl.libc.org/) based [GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html), [FreeBSD](https://www.freebsd.org/), [OpenBSD](https://www.openbsd.org/), [NetBSD](https://www.netbsd.org/), because there is no available [Android NDK](https://developer.android.google.cn/ndk) can run on these platforms.
- This software do NOT work on [Cygwin](http://cygwin.org/) and [MSYS2](https://www.msys2.org/) due to `CMake: Builds hosted on 'CYGWIN' not supported.` [Android-Determine.cmake](https://github.com/Kitware/CMake/blob/master/Modules/Platform/Android-Determine.cmake#L271-L291)

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
docker exec -it ndk-pkg ndk-pkg upgrade-self
docker exec -it ndk-pkg ndk-pkg update
docker exec -it ndk-pkg ndk-pkg install curl
```

**Note:** you can use `podman` instead of `docker`

## Install ndk-pkg via cURL on UNIX

```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/bin/ndk-pkg
chmod a+x ndk-pkg
./ndk-pkg setup
```

## Install ndk-pkg via cURL on Termux

```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/bin/ndk-pkg
chmod a+x ndk-pkg
./ndk-pkg setup
```

## Install ndk-pkg on WSL

**Note:**

- `WSL2` is recommended. [Comparing WSL 1 and WSL 2](https://docs.microsoft.com/en-us/windows/wsl/compare-versions)

- `Ubuntu-22.04` distrbution is recommended. [Available WSL distribution](https://docs.microsoft.com/en-us/windows/wsl/install-manual#downloading-distributions)

- I assume that `WSL` has been installed, if not, you can follow the step-by-step instructions to install it. [Install WSL](https://docs.microsoft.com/en-us/windows/wsl/install)

**/etc/wsl.conf**

```ini
[network]
hostname = ubuntu
generateHosts = false

[automount]
enabled = true
root = /mnt/
options = "metadata,umask=22,fmask=11"
mountFsTab = true
```

_instructions installing ndk-pkg on WSL is same as described in section [Install ndk-pkg via cURL on UNIX](https://github.com/leleliu008/ndk-pkg#install-ndk-pkg-via-curl-on-unix)_

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
find_package(libpng REQUIRED CONFIG)
target_link_libraries(xx libpng::libpng.a)
```

or

```cmake
find_package(libpng CONFIG)
if (libpng_FOUND)
    target_link_libraries(xx libpng::libpng.a)
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
  - <https://repo1.maven.org/maven2/com/fpliu/ndk/pkg/prefab/android/21/>
  - <https://search.maven.org/search?q=com.fpliu.ndk.pkg.prefab>
- If packages that have been published to `mavenCentral` doesn't meet your needs, you can install package then deploy it to `mavenLocal` via running following commands:

    ```bash
    ndk-pkg install libpng
    ndk-pkg deploy  libpng mavenLocal
    ```

    enables `mavenLocal` repository in `build.gradle`

    ```gradle
    repositories {
        mavenLocal()
    }
    ```

- Every package provides several cmake imported targets and each target has form: `${PACKAGE_NAME}::${LIBRARY_FILENAME}`

**References:**

- <https://github.com/google/prefab>
- <https://developer.android.com/studio/build/dependencies?agpversion=4.1#using-native-dependencies>

**Examples:**

- <https://github.com/leleliu008/android-calendar-for-the-aged>

## ~/.ndk-pkg

all relevant directories and files are located in `~/.ndk-pkg` directory.

**Caveats**:

- Please do NOT place your own files in `~/.ndk-pkg` directory, as `ndk-pkg` will change files in `~/.ndk-pkg` directory without notice.
- Please do NOT run `ndk-pkg` command in parallell to avoid destroying the data.

## ndk-pkg command usage

- **show help of this command**

    ```bash
    ndk-pkg -h
    ndk-pkg --help
    ```

- **show version of this command**

    ```bash
    ndk-pkg -V
    ndk-pkg --version
    ```

- **show basic information about this software**

    ```bash
    ndk-pkg env
    ```

- **show basic information about your current running operation system**

    ```bash
    ndk-pkg sysinfo
    ```

- **show basic information about [Android NDK](https://developer.android.com/ndk)**

    ```bash
    ndk-pkg ndkinfo
    ndk-pkg ndkinfo --ndk-home=/usr/local/share/android-ndk
    ```

    **Note**: If `--ndk-home=<ANDROID-NDK-HOME>` option is not given, then `~/.ndk-pkg/android-ndk-r[1-9][0-9][a-z]` and value of `ANDROID_NDK_ROOT` and `ANDROID_NDK_HOME` environment variable would be checked in order.

- **install essential tools used by this shell script**

    ```bash
    ndk-pkg setup
    ```

    **Note**: above command do two things:

  - install [uppm](https://github.com/leleliu008/uppm)
  - install all the essential tools (e.g. `bash` `coreutils` `findutils` `gawk` `gsed` `grep` `gtar` `gzip` `lzip` `unzip` `zip` `jq` `yq` `git` `curl` `tree`) that are used by this shell script via [uppm](https://github.com/leleliu008/uppm)

- **integrate `zsh-completion` script**

    ```bash
    ndk-pkg integrate zsh
    ndk-pkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
    ndk-pkg integrate zsh -v
    ```

    This project provides a zsh-completion script for `ndk-pkg`. when you've typed `ndk-pkg` then type `TAB` key, the rest of the arguments will be automatically complete for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit` in your terminal (your current running shell must be zsh).

- **update all available formula repositories**

    ```bash
    ndk-pkg update
    ```

- **search all available packages whose name matches the given regular expression pattern**

    ```bash
    ndk-pkg search curl
    ndk-pkg search lib
    ```

- **show information of the given package**

    ```bash
    ndk-pkg info curl
    ndk-pkg info curl --yaml
    ndk-pkg info curl --json
    ndk-pkg info curl version
    ndk-pkg info curl license
    ndk-pkg info curl summary
    ndk-pkg info curl web-url
    ndk-pkg info curl git-url
    ndk-pkg info curl git-sha
    ndk-pkg info curl git-ref
    ndk-pkg info curl src-url
    ndk-pkg info curl src-sha

    ndk-pkg info curl installed-dir
    ndk-pkg info curl installed-files
    ndk-pkg info curl installed-version
    ndk-pkg info curl installed-timestamp-unix
    ndk-pkg info curl installed-timestamp-iso-8601
    ndk-pkg info curl installed-timestamp-rfc-3339
    ndk-pkg info curl installed-timestamp-iso-8601-utc
    ndk-pkg info curl installed-timestamp-rfc-3339-utc

    ndk-pkg info @all
    ```

- **show formula of the given package**

    ```bash
    ndk-pkg formula curl
    ndk-pkg formula curl --yaml
    ndk-pkg formula curl --json
    ndk-pkg formula curl --path
    ndk-pkg formula curl version
    ndk-pkg formula curl license
    ndk-pkg formula curl summary
    ndk-pkg formula curl web-url
    ndk-pkg formula curl git-url
    ndk-pkg formula curl git-sha
    ndk-pkg formula curl git-ref
    ndk-pkg formula curl src-url
    ndk-pkg formula curl src-sha
    ```

- **show receipt of the given installed package**

    ```bash
    ndk-pkg receipt curl
    ndk-pkg receipt android-21/arm64-v8a/curl
    ndk-pkg receipt android-21/arm64-v8a/curl --yaml
    ndk-pkg receipt android-21/arm64-v8a/curl --json
    ndk-pkg receipt android-21/arm64-v8a/curl --path
    ndk-pkg receipt android-21/arm64-v8a/curl version
    ndk-pkg receipt android-21/arm64-v8a/curl license
    ndk-pkg receipt android-21/arm64-v8a/curl summary
    ndk-pkg receipt android-21/arm64-v8a/curl web-url
    ndk-pkg receipt android-21/arm64-v8a/curl git-url
    ndk-pkg receipt android-21/arm64-v8a/curl git-sha
    ndk-pkg receipt android-21/arm64-v8a/curl git-ref
    ndk-pkg receipt android-21/arm64-v8a/curl src-url
    ndk-pkg receipt android-21/arm64-v8a/curl src-sha
    ```

- **show packages that are depended by the given package**

    ```bash
    ndk-pkg depends curl

    ndk-pkg depends curl -t dot
    ndk-pkg depends curl -t box
    ndk-pkg depends curl -t png
    ndk-pkg depends curl -t svg

    ndk-pkg depends curl -t dot -o dependencies/
    ndk-pkg depends curl -t box -o dependencies/
    ndk-pkg depends curl -t png -o dependencies/
    ndk-pkg depends curl -t svg -o dependencies/

    ndk-pkg depends curl -o curl-dependencies.dot
    ndk-pkg depends curl -o curl-dependencies.box
    ndk-pkg depends curl -o curl-dependencies.png
    ndk-pkg depends curl -o curl-dependencies.svg
    ```

- **download resources of the given package to the local cache**

    ```bash
    ndk-pkg fetch curl
    ndk-pkg fetch @all

    ndk-pkg fetch curl -v
    ndk-pkg fetch @all -v
    ```

- **install packages**

    ```bash
    ndk-pkg install curl
    ndk-pkg install curl bzip2 -v
    ```

- **reinstall packages**

    ```bash
    ndk-pkg reinstall curl
    ndk-pkg reinstall curl bzip2 -v
    ```

- **uninstall packages**

    ```bash
    ndk-pkg uninstall curl
    ndk-pkg uninstall curl bzip2 -v
    ```

- **upgrade the outdated packages**

    ```bash
    ndk-pkg upgrade
    ndk-pkg upgrade curl
    ndk-pkg upgrade curl bzip2 -v
    ```

- **upgrade this software**

    ```bash
    ndk-pkg upgrade-self
    ndk-pkg upgrade-self -v
    ```

- **edit the formula of the given package**

    ```bash
    ndk-pkg formula-edit curl
    ndk-pkg formula-edit curl --editor=/usr/local/bin/vim
    ```

    **Note**: `ndk-pkg` do NOT save your changes, which means that your changes may be lost after the formula repository is updated!

- **list all avaliable formula repositories**

    ```bash
    ndk-pkg formula-repo-list
    ```

- **add a new formula repository**

    ```bash
    ndk-pkg formula-repo-add my_repo https://github.com/leleliu008/ndk-pkg-formula-repository-my_repo
    ndk-pkg formula-repo-add my_repo https://github.com/leleliu008/ndk-pkg-formula-repository-my_repo master
    ndk-pkg formula-repo-add my_repo https://github.com/leleliu008/ndk-pkg-formula-repository-my_repo main
    ```

- **delete a existing formula repository**

    ```bash
    ndk-pkg formula-repo-del my_repo
    ```

- **list all available packages**

    ```bash
    ndk-pkg ls-available
    ```

- **list all installed packages**

    ```bash
    ndk-pkg ls-installed
    ```

- **list all outdated packages**

    ```bash
    ndk-pkg ls-outdated
    ```

- **check if the given package is available**

    ```bash
    ndk-pkg is-available curl
    ```

- **check if the given package is installed**

    ```bash
    ndk-pkg is-installed curl
    ndk-pkg is-installed android-21/arm64-v8a/curl
    ```

- **check if the given package is outdated**

    ```bash
    ndk-pkg is-outdated  curl
    ndk-pkg is-outdated  android-21/arm64-v8a/curl
    ```

- **list installed files of the given installed package in a tree-like format**

    ```bash
    ndk-pkg tree curl
    ndk-pkg tree android-21/arm64-v8a/curl -L 3
    ```

- **show logs of the given installed package**

    ```bash
    ndk-pkg logs curl
    ndk-pkg logs curl android-21/arm64-v8a/curl
    ```

- **pack the given installed package**

    ```bash
    ndk-pkg pack curl
    ndk-pkg pack android-21/arm64-v8a/curl
    ndk-pkg pack android-21/arm64-v8a/curl -t tar.xz
    ndk-pkg pack android-21/arm64-v8a/curl -t tar.gz
    ndk-pkg pack android-21/arm64-v8a/curl -t tar.lz
    ndk-pkg pack android-21/arm64-v8a/curl -t tar.bz2
    ndk-pkg pack android-21/arm64-v8a/curl -t zip
    ndk-pkg pack android-21/arm64-v8a/curl -t zip -o a/
    ndk-pkg pack android-21/arm64-v8a/curl -o a/x.zip
    ```

- **generate url-transform sample**

    ```bash
    ndk-pkg gen-url-transform-sample
    ```

- **delete the unused cached files**

    ```bash
    ndk-pkg cleanup
    ```

- **pack the given installed package as prefab aar and then deploy it to Maven local repository**

    ```bash
    ndk-pkg deploy curl mavenLocal
    ndk-pkg deploy curl mavenLocal -d
    ndk-pkg deploy curl mavenLocal -x
    ```

- **pack the given installed package as prefab aar and then deploy it to Maven remote repository**

    ```bash
    ndk-pkg deploy curl mavenRemote    < ~/OSSRH-config
    ndk-pkg deploy curl mavenRemote -d < ~/OSSRH-config
    ndk-pkg deploy curl mavenRemote -x < ~/OSSRH-config
    ndk-pkg deploy curl mavenRemote -x <<EOF
    SERVER_ID=OSSRH
    SERVER_URL=https://s01.oss.sonatype.org/service/local/repositories/comfpliu-1025/content
    SERVER_USERNAME=leleliu008
    SERVER_PASSWORD=xx
    GPG_PASSPHRASE=yy
    EOF
    ```

## environment variables

- **HOME**

    This environment variable already have been set on the most operating systems, if not set or set a empty string, you will receive an error message.

- **PATH**

    This environment variable already have been set on the most operating systems, if not set or set a empty string, you will receive an error message.

- **ANDROID_NDK_ROOT or ANDROID_NDK_HOME**

    If `--ndk-home=<ANDROID-NDK-HOME>` option is not given when installing a package, then `~/.ndk-pkg/android-ndk-r[1-9][0-9][a-z]` and value of `ANDROID_NDK_ROOT` and `ANDROID_NDK_HOME` environment variable would be checked in order. If Not Found, a specific version of `Android NDK` will be automatically installed to `~/.ndk-pkg/android-ndk-r[1-9][0-9][a-z]`.

- **SSL_CERT_FILE**

    ```bash
    curl -LO https://curl.se/ca/cacert.pem
    export SSL_CERT_FILE="$PWD/cacert.pem"
    ```

    In general, you don't need to set this environment variable, but, if you encounter the reporting `the SSL certificate is invalid`, trying to run above commands in your terminal will do the trick.

- **NDKPKG_URL_TRANSFORM**

    ```bash
    export NDKPKG_URL_TRANSFORM=/path/of/url-transform
    ```

    `/path/of/url-transform` command would be invoked as `/path/of/url-transform <URL>`

    `/path/of/url-transform` command must output a `<URL>`

    you can generate a url-transform sample via `ndk-pkg gen-url-transform-sample`

    If you want to change the request url, you can set this environment variable. It is very useful for chinese users.

- **NDKPKG_XTRACE**

    for debugging purposes.

    this environment variable only affects POSIX-Shell-based implementation.

    enables `set -x`:

    ```bash
    export NDKPKG_XTRACE=1
    ```

- **NDKPKG_DEFAULT_TARGET_OS_SPEC**

    some commands need `<PACKAGE-SPEC>` to be specified. `<PACKAGE-SPEC>` has the form `<TARGET-OS-SPEC>/<PACKAGE-NAME>`, you can omit `<TARGET-OS-SPEC>/`. If `<TARGET-OS-SPEC>/` is omitted, this environment variable will be used, if this environment variable is not set, then `android-21/arm64-v8a` will be used as the default.

    example:

    ```bash
    export NDKPKG_DEFAULT_TARGET_OS_SPEC='android-21/arm64-v8a'
    ```

- **other relevant environment variables**

    |utility|reference|
    |-|-|
    |[cmake](https://cmake.org/)|[reference](https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html)|
    |[cargo](https://doc.rust-lang.org/cargo/)|[reference](https://doc.rust-lang.org/cargo/reference/environment-variables.html)|
    |[go](https://golang.org/)|[reference](https://golang.org/doc/install/source#environment)|
    |[pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)|[reference](https://www.linuxhowtos.org/manpages/1/pkg-config.htm#lbAF)|
    |[aclocal](https://www.gnu.org/software/automake/manual/html_node/configure.html)|[reference](https://www.gnu.org/software/automake/manual/html_node/Macro-Search-Path.html)|

    example:

    ```bash
    export GOPROXY='https://goproxy.cn'
    ```

## ndk-pkg formula

a ndk-pkg formula is a [YAML](https://yaml.org/spec/1.2.2/) format file which is used to config a ndk-pkg package's meta-information including one sentence description, package version, installation instructions, etc.

a ndk-pkg formula's filename suffix must be `.yml`

a ndk-pkg formula'a filename prefix would be treated as the package name.

a ndk-pkg formula'a filename prefix must match regular expression pattern `^[A-Za-z0-9+-._]{1,50}$`

a ndk-pkg formula's file content must follow [the ndk-pkg formula scheme](https://github.com/leleliu008/ndk-pkg-formula-repository-offical-core)

## ndk-pkg formula repository

a ndk-pkg formula repository is a git repository.

a ndk-pkg formula repository's root dir should have a `formula` named sub dir, this repository's formulas all should be located in this dir.

a ndk-pkg formula repository's local path is `~/.ndk-pkg/repos.d/${ndk-pkgFormulaRepoName}`

ndk-pkg supports multiple formula repositories.

## ndk-pkg formula repository's config

After a ndk-pkg formula repository is successfully fetched from server to local, a config file for this repository would be created at `~/.ndk-pkg/repos.d/${ndk-pkgFormulaRepoName}/.ndk-pkg-formula-repo.yml`

a typical ndk-pkg formula repository's config as following:

```yaml
url: https://github.com/leleliu008/ndk-pkg-formula-repository-offical-core
branch: master
pinned: 0
enabled: 1
timestamp-created: 1673684639
timestamp-updated: 1673684767
```

If a ndk-pkg formula repository is `pinned`, which means it would not be updated.

If a ndk-pkg formula repository is `disabled`, which means ndk-pkg would not search formulas in this formula repository.

## ndk-pkg offical formula repository

ndk-pkg offical formula repository's url: <https://github.com/leleliu008/ndk-pkg-formula-repository-offical-core>

ndk-pkg offical formula repository would be automatically fetched to local cache as name `offical-core` when you run `ndk-pkg update` command.

**Note:** If you find that a package is not in ndk-pkg offical formula repository yet, PR is welcomed.
