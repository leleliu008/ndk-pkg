# ndk-pkg

A package manager for [Android NDK](https://developer.android.google.cn/ndk) to build projects that are written in C/C++/Rust/Golang.

## Caveats

Please read these caveats carefully before starting to use this software.

- This software is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.

- This software can NOT run on [Android](https://www.android.com/), [musl-libc](http://musl.libc.org/) based [GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html), [FreeBSD](https://www.freebsd.org/), [OpenBSD](https://www.openbsd.org/), [NetBSD](https://www.netbsd.org/) due to lack of [Android NDK](https://developer.android.google.cn/ndk) for these platforms.

- This software can NOT run on [Cygwin](http://cygwin.org/) and [MSYS2](https://www.msys2.org/) due to `CMake: Builds hosted on 'CYGWIN' not supported.` [Android-Determine.cmake](https://github.com/Kitware/CMake/blob/master/Modules/Platform/Android-Determine.cmake#L271-L291)

- Please do NOT place your own files under `~/.ndk-pkg` directory, as `ndk-pkg` will change files under `~/.ndk-pkg` directory without notice.

- Please do NOT run `ndk-pkg` command in parallel to avoid generating dirty data.

## Supported platforms (platforms that this software can be run on)

|HostOS|HostArch|SubSystem|recommended|summary|
|------|--------|---------|-----------|-------|
✶|`x86_64`|[Docker](https://www.docker.com/)|✔︎|tested with `Github Actions`|
[GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html)|`x86_64`|[glibc](http://www.gnu.org/software/libc/)|✔︎|tested with `Github Actions`|
[Windows](https://www.microsoft.com/en-us/windows/)|`x86_64`|[WSL](https://docs.microsoft.com/en-us/windows/wsl/)||tested with `Github Actions`|
[macOS](https://www.apple.com.cn/mac/)|`x86_64` `arm64`|||tested with `Github Actions`|

## Using ndk-pkg via GitHub Actions

This is the recommended way of using this software.

In this way, you don't need to care about other things than:

- what package would you like to build?
- which minimum Android SDK API level would you like to support?
- which Android ABI would you like to build for?
- which build type would you like to use?
- which link type would you like to use?

||
|-|
|<img src="1.png" width="311px" >|

For details please refer to <https://github.com/leleliu008/ndk-pkg-package-manually-build>

## Using ndk-pkg via Docker

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

**step3. install essential tools**

```bash
docker exec -it ndk-pkg ndk-pkg setup
```

**step4. update formula repositories**

```bash
docker exec -it ndk-pkg ndk-pkg update
```

If all goes well, then next you can start to install packages whatever you want, for example, let's install `curl` package:

```bash
docker exec -it ndk-pkg ndk-pkg install android-21-aarch64/curl
```

**Note:** you can use `podman` instead of `docker`

## Install ndk-pkg via cURL

```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/ndk-pkg
chmod a+x ndk-pkg
./ndk-pkg setup
```

## Install ndk-pkg via wget

```bash
wget https://cdn.jsdelivr.net/gh/leleliu008/ndk-pkg/ndk-pkg
chmod a+x ndk-pkg
./ndk-pkg setup
```

## Install ndk-pkg via git

```bash
git clone --depth 1 https://github.com/leleliu008/ndk-pkg
ndk-pkg/ndk-pkg setup
```

## ~/.ndk-pkg

all relevant directories and files are located under `~/.ndk-pkg` directory.

a typical hierarchical structure under `~/.ndk-pkg` directory is as follows:

```text
~/.ndk-pkg
├── core
│   ├── cacert.pem
│   ├── profile
│   ├── sed-in-place
│   ├── sed-in-place.c
│   ├── uppm
│   ├── uppm-0.15.0-linux-x86_64.tar.xz
│   ├── uppm-latest-release
│   ├── wrapper-native-c++
│   ├── wrapper-native-c++.c
│   ├── wrapper-native-cc
│   ├── wrapper-native-cc.c
│   ├── wrapper-native-objc
│   ├── wrapper-native-objc.c
│   ├── wrapper-target-c++
│   ├── wrapper-target-cc
│   └── wrapper-target-cc.c
├── downloads
│   ├── 8f74213b56238c85a50a5329f77e06198771e70dd9a739779f4c02f65d971313.tgz
│   ├── b3a24de97a8fdbc835b9833169501030b8977031bcb54b3b3ac13740f846ab30.tgz
│   └── c642ae9b75fee120b2d96c712538bd2cf283228d2337df2cf2988e3c02678ef4.tgz
├── installed
│   ├── android-21-aarch64
│   │   ├── f39a5f7836ac7ca1e04de14c8103e663d0b375a524a40e537258747e2deb3c0b
│   │   │   ├── include
│   │   │   ├── lib
│   │   │   ├── lib-for-apk
│   │   │   └── share
│   │   └── zlib -> f39a5f7836ac7ca1e04de14c8103e663d0b375a524a40e537258747e2deb3c0b
│   ├── android-21-armv7a
│   │   ├── cc9b367d5068ef6b8aaaee38ec2a25691da35e02757c7e0d83aff3775aef3323
│   │   │   ├── include
│   │   │   ├── lib
│   │   │   ├── lib-for-apk
│   │   │   └── share
│   │   └── zlib -> cc9b367d5068ef6b8aaaee38ec2a25691da35e02757c7e0d83aff3775aef3323
│   ├── android-21-i686
│   │   ├── a0718632fe829426c1d946e6658cc7586da0039e99d5a140d1e402a6b4a4e2f3
│   │   │   ├── include
│   │   │   ├── lib
│   │   │   ├── lib-for-apk
│   │   │   └── share
│   │   └── zlib -> a0718632fe829426c1d946e6658cc7586da0039e99d5a140d1e402a6b4a4e2f3
│   └── android-21-x86_64
│       ├── c099047714d4ce1402d66346da88d14f25c313b0d4c879520198426ebc2f36fe
│       │   ├── include
│       │   ├── lib
│       │   ├── lib-for-apk
│       │   └── share
│       └── zlib -> c099047714d4ce1402d66346da88d14f25c313b0d4c879520198426ebc2f36fe
├── native
│   └── linux-x86_64
│       ├── 30b5043e2c5513343152506e5b1e14436ddbb654f7edf69167df05f117fcdb16
│       │   ├── bin
│       │   │   ├── aclocal
│       │   │   ├── aclocal-1.16
│       │   │   ├── automake
│       │   │   └── automake-1.16
│       │   ├── share
│       │   └── receipt.txt
│       └── automake -> 30b5043e2c5513343152506e5b1e14436ddbb654f7edf69167df05f117fcdb16
├── repos.d
│   └── official-core
│       ├── formula
│       │   ├── zlib.yml
│       │   ├── libbz2.yml
│       │   ├── liblzma.yml
│       │   └── libzstd.yml
│       └── README.md
└── run
    ├── 3409784
    ├── 3447656
    └── 3457395
```

**Caveats**:

- Please do NOT place your own files under `~/.ndk-pkg` directory, as `ndk-pkg` will change files under `~/.ndk-pkg` directory without notice.

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

    **Note**: If `--ndk-home=<ANDROID-NDK-HOME>` option is not given, then value of `ANDROID_NDK_ROOT` and `ANDROID_NDK_HOME` environment variable would be checked in order.

- **generate url-transform sample**

    ```bash
    ndk-pkg gen-url-transform-sample
    ```

- **install essential tools used by this shell script**

    ```bash
    ndk-pkg setup
    ```

    This command is actually to do two things:

  - install [uppm](https://github.com/leleliu008/uppm) to `~/.ndk-pkg/core`
  - install other essential tools (listed below) that are used by this shell script via [uppm](https://github.com/leleliu008/uppm)

    - [GNU Bash](https://www.gnu.org/software/bash/manual/bash.html)
    - [GNU CoreUtils](https://www.gnu.org/software/coreutils/manual/coreutils.html)
    - [GNU FindUtils](https://www.gnu.org/software/findutils/manual/html_mono/find.html)
    - [GNU awk](https://www.gnu.org/software/gawk/manual/gawk.html)
    - [GNU sed](https://www.gnu.org/software/sed/manual/sed.html)
    - [GNU grep](https://www.gnu.org/software/grep/manual/grep.html)
    - [BSD tar](https://man.archlinux.org/man/core/libarchive/bsdtar.1.en)
    - [tree](https://linux.die.net/man/1/tree)
    - [curl](https://curl.se/docs/manpage.html)
    - [git](https://git-scm.com/docs/git)
    - [yq](https://mikefarah.gitbook.io/yq/)
    - [jq](https://stedolan.github.io/jq/manual/)

- **integrate `zsh-completion` script**

    ```bash
    ndk-pkg integrate zsh
    ndk-pkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
    ndk-pkg integrate zsh -v
    ```

    This software provides a zsh-completion script for `ndk-pkg`. when you've typed `ndk-pkg` then type `TAB` key, the rest of the arguments will be automatically complete for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit` in your terminal (your current running shell must be zsh).

    **Caveat**: to use this feature, you should put `ndk-pkg` command in `PATH`

- **update all available formula repositories**

    ```bash
    ndk-pkg update
    ```

- **search all available packages whose name matches the given regular expression pattern**

    ```bash
    ndk-pkg search curl
    ndk-pkg search lib
    ```

- **show information of the given available package**

    ```bash
    ndk-pkg info-available curl
    ndk-pkg info-available curl --yaml
    ndk-pkg info-available curl --json
    ndk-pkg info-available curl version
    ndk-pkg info-available curl license
    ndk-pkg info-available curl summary
    ndk-pkg info-available curl web-url
    ndk-pkg info-available curl git-url
    ndk-pkg info-available curl git-sha
    ndk-pkg info-available curl git-ref
    ndk-pkg info-available curl src-url
    ndk-pkg info-available curl src-sha
    ndk-pkg info-available curl src-ft
    ndk-pkg info-available curl src-fp
    ```

- **show information of the given installed package**

    ```bash
    ndk-pkg info-installed android-21-aarch64/curl --prefix
    ndk-pkg info-installed android-21-aarch64/curl --files
    ndk-pkg info-installed android-21-aarch64/curl builtat
    ndk-pkg info-installed android-21-aarch64/curl builtat-iso-8601
    ndk-pkg info-installed android-21-aarch64/curl builtat-rfc-3339
    ndk-pkg info-installed android-21-aarch64/curl builtat-iso-8601-utc
    ndk-pkg info-installed android-21-aarch64/curl builtat-rfc-3339-utc
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
    ndk-pkg fetch curl -v
    ```

- **install packages**

    ```bash
    ndk-pkg install curl
    ndk-pkg install android-33-aarch64/curl
    ndk-pkg install android-33-aarch64/curl --link-type=static-full
    ```

- **reinstall packages**

    ```bash
    ndk-pkg reinstall curl
    ndk-pkg reinstall android-33-aarch64/curl --link-type=static-full
    ```

- **uninstall packages**

    ```bash
    ndk-pkg uninstall curl
    ndk-pkg uninstall android-33-aarch64/curl
    ```

- **upgrade the outdated packages**

    ```bash
    ndk-pkg upgrade curl
    ndk-pkg upgrade android-33-aarch64/curl --link-type=static-full
    ```

- **upgrade this software**

    ```bash
    ndk-pkg upgrade-self
    ndk-pkg upgrade-self -v
    ```

- **list all available formula repositories**

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
    ndk-pkg is-installed android-21-aarch64/curl
    ```

- **check if the given package is outdated**

    ```bash
    ndk-pkg is-outdated  curl
    ndk-pkg is-outdated  android-21-aarch64/curl
    ```

- **list installed files of the given installed package in a tree-like format**

    ```bash
    ndk-pkg tree curl
    ndk-pkg tree android-21-aarch64/curl -L 3
    ```

- **show logs of the given installed package**

    ```bash
    ndk-pkg logs curl
    ndk-pkg logs android-21-aarch64/curl
    ```

- **pack the given installed package**

    ```bash
    ndk-pkg pack curl
    ndk-pkg pack android-21-aarch64/curl
    ndk-pkg pack android-21-aarch64/curl -t tar.xz
    ndk-pkg pack android-21-aarch64/curl -t tar.gz
    ndk-pkg pack android-21-aarch64/curl -t tar.lz
    ndk-pkg pack android-21-aarch64/curl -t tar.bz2
    ndk-pkg pack android-21-aarch64/curl -t zip
    ndk-pkg pack android-21-aarch64/curl -t zip -o a/
    ndk-pkg pack android-21-aarch64/curl -o a/x.zip
    ```

- **export the given installed package as the google prefab aar**

    ```bash
    ndk-pkg export android-21-aarch64,x86_64/curl -o .
    ndk-pkg export android-21-aarch64,x86_64/curl -o curl-8.1.2.aar
    ```

- **export the given installed package as the google prefab aar then deploy it to Maven Local Repository**

    ```bash
    ndk-pkg depoly android-21-aarch64,x86_64/curl
    ndk-pkg depoly android-21-aarch64,x86_64/curl --debug
    ndk-pkg depoly android-21-aarch64,x86_64/curl --local=/somewhere
    ```

- **export the given installed package as the google prefab aar then deploy it to Sonatype OSSRH**

    ```bash
    ndk-pkg depoly android-21-aarch64,x86_64/curl --remote < ~/OSSRH-config
    ndk-pkg depoly android-21-aarch64,x86_64/curl --remote <<EOF
    SERVER_ID=OSSRH
    SERVER_URL=https://s01.oss.sonatype.org/service/local/staging/deploy/maven2/
    SERVER_USERNAME=your-sonatype-account-username
    SERVER_PASSWORD=your-sonatype-account-password
    GPG_PASSPHRASE=your-gpg-store-passphase
    EOF
    ```

- **delete the unused cached files**

    ```bash
    ndk-pkg cleanup
    ```

## environment variables

- **HOME**

    This environment variable already have been set on the most operating systems, if not set or set a empty string, you will receive an error message.

- **PATH**

    This environment variable already have been set on the most operating systems, if not set or set a empty string, you will receive an error message.

- **SSL_CERT_FILE**

    ```bash
    curl -LO https://curl.se/ca/cacert.pem
    export SSL_CERT_FILE="$PWD/cacert.pem"
    ```

    In general, you don't need to set this environment variable, but, if you encounter the reporting `the SSL certificate is invalid`, trying to run above commands in your terminal will do the trick.

- **GOPROXY**

    ```bash
    export GOPROXY='https://goproxy.cn'
    ```

- **ANDROID_NDK_ROOT and ANDROID_NDK_HOME**

    If `--ndk-home=<ANDROID-NDK-HOME>` option is not given when installing a package, then value of `ANDROID_NDK_ROOT` and `ANDROID_NDK_HOME` environment variable would be checked in order. If they both are not set or set a empty string or are invalid android ndk home directory, a specific version of `Android NDK` will be automatically installed via [uppm](https://github.com/leleliu008/uppm).

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

    enable `set -x`:

    ```bash
    export NDKPKG_XTRACE=1
    ```

- **NDKPKG_DEFAULT_TARGET**

    some sub-commands of `ndk-pkg` need `<PACKAGE-SPEC>` to be specified. `<PACKAGE-SPEC>` has the form `<TARGET>/<PACKAGE-NAME>`, To simplify the usage, you are allowed to omit `<TARGET>/`. If `<TARGET>/` is omitted, this environment variable will be used, if this environment variable is not set, then `android-21-aarch64` will be used as the default.

    `<TARGET>` has the form `android-<MIN-SDK-API-LEVEL>-<TARGET-ARCH>`

    **Example**:

    ```bash
    export NDKPKG_DEFAULT_TARGET='android-21-aarch64'
    ```

    **References**:

  - <https://developer.android.com/tools/releases/platforms>
  - <https://developer.android.com/ndk/guides/abis>

**Note:** some commonly used environment variables are override by this software, these are `CC`, `CXX`, `CPP`, `AS`, `AR`, `LD`, `CFLAGS`, `CPPFLAGS`, `LDFLAGS`, `PKG_CONFIG_LIBDIR`, `PKG_CONFIG_PATH`, `ACLOCAL_PATH`

## ndk-pkg formula scheme

a ndk-pkg formula is a [YAML](https://yaml.org/spec/1.2.2/) format file which is used to config a ndk-pkg package's meta-information including one sentence description, package version, installation instructions, etc.

a ndk-pkg formula's filename suffix must be `.yml`

a ndk-pkg formula'a filename prefix would be treated as the package name.

a ndk-pkg formula'a filename prefix must match regular expression pattern `^[A-Za-z0-9+-._@]{1,50}$`

a uppm formula's file content only has one level mapping and shall has following KEY:

|KEY|required?|overview|
|-|-|-|
|`summary`|required|describe this package in one sentence.|
|`license`|optional|a space-separated list of [SPDX license short identifiers](https://spdx.github.io/spdx-spec/v2.3/SPDX-license-list/#a1-licenses-with-short-identifiers)|
|`version`|optional|the version of this package.<br>If this mapping is not present, it will be calculated from `src-url`, if `src-url` is also not present, it will be calculated from running time as format `date +%Y.%m.%d`|
||||
|`web-url`|optional|the home webpage of this package.<br>If this mapping is not present, `git-url` must be present.|
||||
|`git-url`|optional|the source code git repository.<br>If `src-url` is not present, this mapping must be present.|
|`git-ref`|optional|reference: <https://git-scm.com/book/en/v2/Git-Internals-Git-References> <br>example values: `HEAD` `refs/heads/master` `refs/heads/main` `refs/tags/v1`, default value is `HEAD`|
|`git-sha`|optional|the full git commit id, 40-byte hexadecimal string, if `git-ref` and `git-sha` both are present, `git-sha` takes precedence over `git-ref`|
|`git-nth`|optional|tell `ndk-pkg` that how many depth commits would you like to be fetched. default is `1`, this would save your time and storage. If you want to fetch all commits, set this to `0`|
||||
|`src-url`|optional|the source code download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2`, it will be uncompressed to `$PACKAGE_WORKING_DIR/src` when this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/src`<br>also support format like `dir://DIR`|
|`src-uri`|optional|the mirror of `src-url`.|
|`src-sha`|optional|the `sha256sum` of source code.<br>`src-sha` and `src-url` must appear together.|
||||
|`fix-url`|optional|the patch file download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2`, it will be uncompressed to `$PACKAGE_WORKING_DIR/fix` when this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/fix`.|
|`fix-sha`|optional|the `sha256sum` of patch file.<br>`fix-sha` and `fix-url` must appear together.|
||||
|`res-url`|optional|other resource download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2`, it will be uncompressed to `$PACKAGE_WORKING_DIR/res` when this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/res`.|
|`res-sha`|optional|the `sha256sum` of resource file.<br>`res-sha` and `res-url` must appear together.|
||||
|`dep-pkg`|optional|a space-separated list of   `ndk-pkg packages` that are depended by this package when installing and/or runtime, which will be installed via [ndk-pkg](https://github.com/leleliu008/ndk-pkg).|
|`dep-upp`|optional|a space-separated list of   `uppm packages` that are depended by this package when installing and/or runtime, which will be installed via [uppm](https://github.com/leleliu008/uppm).|
|`dep-pym`|optional|a space-separated list of `python packages` that are depended by this package when installing and/or runtime, which will be installed via [pip3](https://github.com/pypa/pip).|
|`dep-plm`|optional|a space-separated list of    `perl modules` that are depended by this package when installing and/or runtime, which will be installed via [cpan](https://metacpan.org/dist/CPAN/view/scripts/cpan).|
||||
|`ccflags`|optional|append to `CFLAGS`|
|`xxflags`|optional|append to `CXXFLAGS`|
|`ppflags`|optional|append to `CPPFLAGS`|
|`ldflags`|optional|append to `LDFLAGS`|
||||
|`bsystem`|optional|build system name.<br>values can be some of `autogen` `autotools` `configure` `cmake` `cmake-gmake` `cmake-ninja` `meson` `xmake` `gmake` `ninja` `cargo` `go`|
|`bscript`|optional|the directory where the build script is located in, relative to `PACKAGE_WORKING_DIR`. build script such as `configure`, `Makefile`, `CMakeLists.txt`, `meson.build`, `Cargo.toml`, etc.|
|`binbstd`|optional|whether to build in the directory where the build script is located in, otherwise build in other directory. value shall be `0` or `1`. default value is `0`.|
||||
|`symlink`|optional|whether to symlink installed files to `$NDKPKG_HOME/symlinked/*`. value shall be `0` or `1`. default value is `1`. It is only meaningful when requesting for native building.|
||||
|`do12345`|optional|POSIX shell code to be run for native build. It is only meaningful when requesting for cross building.|
|`dopatch`|optional|POSIX shell code to be run to apply patches for target build. current working directory is `$PACKAGE_BSCRIPT_DIR`|
|`install`|optional|POSIX shell code to be run for target build. If this mapping is not present, `ndk-pkg` will run default install code according to `bsystem`|
||||
|`api-min`|optional|specify which minimum Android SDK API level is supported for this package.|

**commands that can be used out of the box:**

|command|usage-example|
|-|-|
|`bash`|[Reference](https://www.gnu.org/software/bash/manual/bash.html)|
|`CoreUtils`|[Reference](https://www.gnu.org/software/coreutils/manual/coreutils.html)|
|`xargs`|[Reference](https://www.gnu.org/software/findutils/manual/html_node/find_html/Invoking-xargs.html)|
|`find`|[Reference](https://www.gnu.org/software/findutils/manual/html_mono/find.html)|
|`gawk`|[Reference](https://www.gnu.org/software/gawk/manual/gawk.html)|
|`gsed`|[Reference](https://www.gnu.org/software/sed/manual/sed.html)|
|`grep`|[Reference](https://www.gnu.org/software/grep/manual/grep.html)|
|`yq`|[Reference](https://mikefarah.gitbook.io/yq/)|
|`jq`|[Reference](https://stedolan.github.io/jq/manual/)|
|`git`|[Reference](https://git-scm.com/docs/git)|
|`curl`|[Reference](https://curl.se/docs/manpage.html)|
|`bsdtar`|[Reference](https://man.archlinux.org/man/core/libarchive/bsdtar.1.en)|
|`tree`|[Reference](https://linux.die.net/man/1/tree)|
|`pkg-config`|[Reference](https://people.freedesktop.org/~dbn/pkg-config-guide.html)|
|||
|`echo`|`echo 'your message.'`|
|`info`|`info 'your information.'`|
|`warn`|`warn "no package manager found."`|
|`error`|`error 'error message.'`|
|`abort`|`abort 1 "please specify a package name."`|
|`success`|`success "build success."`|
|`sed_in_place`|`sed_in_place 's/-mandroid//g' Configure`|
|`wfetch`|`wfetch <URL> [--uri=<URL-MIRROR>] [--sha256=<SHA256>] [-o <PATH> [-q]`|
|||
|`configure`|`configure --enable-pic`|
|`mesonw`|`mesonw -Dneon=disabled -Darm-simd=disabled`|
|`cmakew`|`cmakew -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON`|
|`gmakew`|`gmakew`|
|`xmakew`|`xmakew`|
|`cargow`|`cargow`|
|`gow`|`gow`|

**shell variables can be used directly:**

|variable|overview|
|-|-|
|`NDKPKG_VERSION`|the version of `ndk-pkg`.|
|`NDKPKG_HOME`|the home directory of `ndk-pkg`.|
|`NDKPKG`|the executable filepath of `ndk-pkg`.|
|||
|`UPPM`|the executable filepath of [uppm](https://github.com/leleliu008/uppm)|
|||
|`TIMESTAMP_UNIX`|the unix timestamp of this action.|
|||
|`NATIVE_OS_KIND`|current running os kind. value might be `linux` or `darwin`|
|`NATIVE_OS_TYPE`|current running os type. value might be `linux` or `macos`|
|`NATIVE_OS_NAME`|current running os name. value might be `Ubuntu`, `macOS`, etc|
|`NATIVE_OS_VERS`|current running os version.|
|`NATIVE_OS_ARCH`|current running os arch. value might be any one of `x86_64`, `arm64`, etc|
|`NATIVE_OS_NCPU`|current running os's cpu core count.|
|`NATIVE_OS_EUID`|current running os's effective user ID.|
|`NATIVE_OS_EGID`|current running os's effective group ID.|
|||
|`TARGET_PLATFORM_VERS`|[android sdk api-level table](https://developer.android.google.cn/guide/topics/manifest/uses-sdk-element#api-level-table)|
|`TARGET_PLATFORM_ARCH`|value shall be any one of `armv7a` `aarch64` `i686` `x86_64`|
|`TARGET_PLATFORM_NBIT`|value shall be any one of `32` `64`|
|`TARGET_PLATFORM_ABI`|value shall be any one of `armeabi-v7a` `arm64-v8a` `x86` `x86_64`|
|`TARGET_TRIPLE`|value shall be any one of `armv7a-linux-androideabi` `aarch64-linux-android` `i686-linux-android` `x86_64-linux-android`|
|||
|`ANDROID_NDK_HOME`|the home directory of `Android NDK`.|
|`ANDROID_NDK_ROOT`|the home directory of `Android NDK`.|
|`ANDROID_NDK_VERSION`|the version of `Android NDK`.|
|`ANDROID_NDK_VERSION_MAJOR`|the major part of version of `Android NDK`.|
|`ANDROID_NDK_TOOLCHAIN_BIND`|the `bin` directory of `Android NDK`.|
|`ANDROID_NDK_SYSROOT`|the `sysroot` directory of `Android NDK`.|
|||
|`CC_FOR_BUILD`|the C Compiler for native build.|
|`CFLAGS_FOR_BUILD`|the flags of `CC_FOR_BUILD`.|
|`CXX_FOR_BUILD`|the C++ Compiler for native build.|
|`CXXFLAGS_FOR_BUILD`|the flags of `CXX_FOR_BUILD`.|
|`CPP_FOR_BUILD`|the C/C++ PreProcessor for native build.|
|`CPPFLAGS_FOR_BUILD`|the flags of `CPP_FOR_BUILD`.|
|`AS_FOR_BUILD`|the assembler for native build.|
|`AR_FOR_BUILD`|the archiver for native build.|
|`RANLIB_FOR_BUILD`|the archiver extra tool for native build.|
|`LD_FOR_BUILD`|the linker for native build.|
|`LDFLAGS_FOR_BUILD`|the flags of `LD_FOR_BUILD`.|
|`NM_FOR_BUILD`|a command line tool to list symbols from object files for native build.|
|`STRIP_FOR_BUILD`|a command line tool to discard symbols and other data from object files for native build.|
|||
|`CC`|the C Compiler.|
|`CFLAGS`|the flags of `CC`.|
|`CXX`|the C++ Compiler.|
|`CXXFLAGS`|the flags of `CXX`.|
|`CPP`|the C/C++ PreProcessor.|
|`CPPFLAGS`|the flags of `CPP`.|
|`AS`|the assembler.|
|`AR`|the archiver.|
|`RANLIB`|the archiver extra tool.|
|`LD`|the linker.|
|`LDFLAGS`|the flags of `LD`.|
|`NM`|a command line tool to list symbols from object files.|
|`STRIP`|a command line tool to discard symbols and other data from object files.|
|||
|`PACKAGE_WORKING_DIR`|the working directory when installing.|
|`PACKAGE_BSCRIPT_DIR`|the directory where the build script (e.g. `Makefile`, `configure`, `CMakeLists.txt`, `meson.build`, `Cargo.toml`, etc) is located in.|
|`PACKAGE_BCACHED_DIR`|the directory where the temporary files are stored in when building.|
|`PACKAGE_INSTALL_DIR`|the directory where the final files will be installed to.|
|||
|`x_INSTALL_DIR`|the installation directory of x package.|
|`x_INCLUDE_DIR`|`$x_INSTALL_DIR/include`|
|`x_LIBRARY_DIR`|`$x_INSTALL_DIR/lib`|


## ndk-pkg formula repository

a ndk-pkg formula repository is a git repository.

a ndk-pkg formula repository's root directory should have a `formula` named sub directory, this repository's formulas all should be located in this directory.

a ndk-pkg formula repository's local path is `~/.ndk-pkg/repos.d/${ndk-pkgFormulaRepoName}`

ndk-pkg supports multiple formula repositories.

## ndk-pkg formula repository's config

After a ndk-pkg formula repository is successfully fetched from server to local, a config file for this repository would be created at `~/.ndk-pkg/repos.d/${ndk-pkgFormulaRepoName}/.ndk-pkg-formula-repo.yml`

a typical ndk-pkg formula repository's config is as follows:

```yaml
url: https://github.com/leleliu008/ndk-pkg-formula-repository-official-core
branch: master
pinned: 0
enabled: 1
created: 1673684639
updated: 1673684767
```

If a ndk-pkg formula repository is `pinned`, which means it would not be updated.

If a ndk-pkg formula repository is `disabled`, which means ndk-pkg would not search formulas in this formula repository.

## ndk-pkg official formula repository

ndk-pkg official formula repository is hosted at <https://github.com/leleliu008/ndk-pkg-formula-repository-official-core>

ndk-pkg official formula repository would be automatically fetched to local cache as name `official-core` when you run `ndk-pkg update` command.

**Note:** If you find that a package is not in ndk-pkg official formula repository yet, PR is welcomed.

## Using my prefab aars that have been published to GitHub-Hosted Maven Repository alongside with Android Gradle Plugin

I have published some commonly used packages as google prefab aar to <https://github.com/leleliu008/ndk-pkg-prefab-aar-maven-repo>

## Using my prefab aars that have been published to Maven Central Repository alongside with Android Gradle Plugin

I have published some commonly used packages as google prefab aar to `Maven Central Repository`.

To get the full list of my published packages, please visit the following websites:

- <https://repo1.maven.org/maven2/com/fpliu/ndk/pkg/prefab/android/21/>
- <https://search.maven.org/search?q=com.fpliu.ndk.pkg.prefab>

In the next two sections, I will show you how to configure with `Android Gradle Plugin` in `Kotlin DSL` and `Groovy DSL` respectively.

## configure with Android Gradle Plugin Kotlin DSL

**step1. enable prefab feature for Android Gradle Plugin**

```gradle
android {
    buildFeatures {
        prefab = true
    }
}
```

**step2. enable `Maven Central Repository` for Gradle**

```gradle
allprojects {
    repositories {
        maven {
            mavenCentral()
        }
    }
}
```

中国大陆的用户可使用如下配置：

```gradle
allprojects {
    repositories {
        maven {
            maven { url = uri("https://maven.aliyun.com/repository/public") }
        }
    }
}
```

**step3. add dependencies in build.gradle.kts**

Every package's coordinate for Gradle is `com.fpliu.ndk.pkg.prefab.android.21:<PACKAGE-NAME>:<PACKAGE-VERSION>`, for example, `libpng` package has a version `1.6.37`, we could use it as follows:

```gradle
dependencies {
    implementation ("com.fpliu.ndk.pkg.prefab.android.21:libpng:1.6.37")
}
```

**step4. invoke [find_package(PACKAGE-NAME [REQUIRED] CONFIG)](https://cmake.org/cmake/help/latest/command/find_package.html) command in your Android project's CMakeLists.txt**

Every package provides several cmake imported targets:

|TARGET-NAME|example|summary|
|-|-|-|
|`<PACKAGE-NAME>::headers`|`libpng::headers`|C/C++ header files only|
|`<PACKAGE-NAME>::lib*.a`|`libpng::libpng16.a`|static library|
|`<PACKAGE-NAME>::lib*.so`|`libpng::libpng16.so`|shared library|
|`<PACKAGE-NAME>::*`|`libpng::libpng`|base on .pc files|

Following is a piece of codes show you how to link `libpng.a` which is provided by `libpng` package:

```cmake
find_package(libpng REQUIRED CONFIG)
target_link_libraries(app libpng::libpng.a)
```

or

```cmake
find_package(libpng CONFIG)
if (libpng_FOUND)
    target_link_libraries(app libpng::libpng.a)
endif()
```

**step5. configure C++ standard and STL in build.gradle.kts**

```gradle
android {
    defaultConfig {
        externalNativeBuild {
            cmake {
                arguments += "-DANDROID_STL=c++_shared"
                cppFlags  += "-std=c++17"
            }
        }
    }
}
```

**Note:**

- This step is only required for packages that use `libc++`.
- If you link a shared library that depends on `libc++_shared.so`, then your Android app should use `libc++_shared.so` too.

## configure with Android Gradle Plugin Groovy DSL

**step1. enable prefab feature for Android Gradle Plugin**

```gradle
android {
    buildFeatures {
        prefab true
    }
}
```

**step2. enable `Maven Central Repository` for Gradle**

```gradle
allprojects {
    repositories {
        maven {
            mavenCentral()
        }
    }
}
```

中国大陆的用户可使用如下配置：

```gradle
allprojects {
    repositories {
        maven {
            url 'https://maven.aliyun.com/repository/public'
        }
    }
}
```

**step3. add dependencies in build.gradle**

Every package's coordinate for Gradle is `com.fpliu.ndk.pkg.prefab.android.21:<PACKAGE-NAME>:<PACKAGE-VERSION>`, for example, `libpng` package has a version `1.6.37`, we could use it as follows:

```gradle
dependencies {
    implementation 'com.fpliu.ndk.pkg.prefab.android.21:libpng:1.6.37'
}
```

**step4. invoke [find_package(PACKAGE-NAME [REQUIRED] CONFIG)](https://cmake.org/cmake/help/latest/command/find_package.html) command in your Android project's CMakeLists.txt**

Every package provides several cmake imported targets:

|TARGET-NAME|example|summary|
|-|-|-|
|`<PACKAGE-NAME>::headers`|`libpng::headers`|C/C++ header files only|
|`<PACKAGE-NAME>::lib*.a`|`libpng::libpng16.a`|static library|
|`<PACKAGE-NAME>::lib*.so`|`libpng::libpng16.so`|shared library|
|`<PACKAGE-NAME>::*`|`libpng::libpng`|base on .pc files|

Following is a piece of codes show you how to link `libpng.a` which is provided by `libpng` package:

```cmake
find_package(libpng REQUIRED CONFIG)
target_link_libraries(app libpng::libpng.a)
```

or

```cmake
find_package(libpng CONFIG)
if (libpng_FOUND)
    target_link_libraries(app libpng::libpng.a)
endif()
```

**step5. configure C++ standard and STL in build.gradle**

```gradle
android {
    defaultConfig {
        externalNativeBuild {
            cmake {
                arguments '-DANDROID_STL=c++_shared'
                cppFlags  '-std=c++17'
            }
        }
    }
}
```

**Note:**

- This step is only required for packages that use `libc++`.
- If you link a shared library that depends on `libc++_shared.so`, then your Android app should use `libc++_shared.so` too.

**References:**

- <https://google.github.io/prefab/>
- <https://developer.android.com/studio/projects/android-library#aar-contents>
- <https://developer.android.com/studio/build/dependencies?agpversion=4.1#using-native-dependencies>

**Examples:**

- <https://github.com/leleliu008/android-calendar-for-the-aged>

## Create the google prefab aar then deploy it to Maven Local Repository and use it alongside with Android Gradle Plugin

If my published packages don't meet your needs, you can use this software to install packages then deploy them to `Maven Local Repository`.

In the next two sections, I will show you how to configure with `Android Gradle Plugin` in `Kotlin DSL` and `Groovy DSL` respectively.

## configure with Android Gradle Plugin Kotlin DSL

**step1. build and install libpng**

Suppose you want to build and install `libpng`, the following command will build `libpng` with `android-21` API and build for `aarch64` and `armv7a` arch respectively.

```bash
ndk-pkg install android-21-aarch64,armv7a/libpng
```

**step2. export the installed libpng package as the google prefab aar and deploy it to your Maven Local Repository**

```bash
ndk-pkg deploy  android-21-aarch64,armv7a/libpng
```

**step3. enable prefab feature for Android Gradle Plugin**

```gradle
android {
    buildFeatures {
        prefab = true
    }
}
```

**step4. enable `Maven Local Repository` for Gradle**

```gradle
allprojects {
    repositories {
        maven {
            mavenLocal()
        }
    }
}
```

**step5. add dependencies in build.gradle.kts**

Every package's coordinate for Gradle is `com.fpliu.ndk.pkg.prefab.android.21:<PACKAGE-NAME>:<PACKAGE-VERSION>`, for example, `libpng` package has a version `1.6.37`, we could use it as follows:

```gradle
dependencies {
    implementation ("com.fpliu.ndk.pkg.prefab.android.21:libpng:1.6.37")
}
```

**step6. invoke [find_package(PACKAGE-NAME [REQUIRED] CONFIG)](https://cmake.org/cmake/help/latest/command/find_package.html) command in your Android project's CMakeLists.txt**

Every package provides several cmake imported targets:

|TARGET-NAME|example|summary|
|-|-|-|
|`<PACKAGE-NAME>::headers`|`libpng::headers`|C/C++ header files only|
|`<PACKAGE-NAME>::lib*.a`|`libpng::libpng16.a`|static library|
|`<PACKAGE-NAME>::lib*.so`|`libpng::libpng16.so`|shared library|
|`<PACKAGE-NAME>::*`|`libpng::libpng`|base on .pc files|

Following is a piece of codes show you how to link `libpng.a` which is provided by `libpng` package:

```cmake
find_package(libpng REQUIRED CONFIG)
target_link_libraries(app libpng::libpng.a)
```

or

```cmake
find_package(libpng CONFIG)
if (libpng_FOUND)
    target_link_libraries(app libpng::libpng.a)
endif()
```

**step7. configure C++ standard and STL in build.gradle.kts**

```gradle
android {
    defaultConfig {
        externalNativeBuild {
            cmake {
                arguments += "-DANDROID_STL=c++_shared"
                cppFlags  += "-std=c++17"
            }
        }
    }
}
```

**Note:**

- This step is only required for packages that use `libc++`.
- If you link a shared library that depends on `libc++_shared.so`, then your Android app should use `libc++_shared.so` too.

## configure with Android Gradle Plugin Groovy DSL

**step1. build and install libpng**

Suppose you want to build and install `libpng`, the following command will build `libpng` with `android-21` API and build for `aarch64` and `armv7a` arch respectively.

```bash
ndk-pkg install android-21-aarch64,armv7a/libpng
```

**step2. export the installed libpng package as the google prefab aar and deploy it to your Maven Local Repository**

```bash
ndk-pkg deploy  android-21-aarch64,armv7a/libpng
```

**step3. enable prefab feature for Android Gradle Plugin**

```gradle
android {
    buildFeatures {
        prefab true
    }
}
```

**step4. enable `Maven Local Repository` for Gradle**

```gradle
allprojects {
    repositories {
        maven {
            mavenLocal()
        }
    }
}
```

**step5. add dependencies in build.gradle**

Every package's coordinate for Gradle is `com.fpliu.ndk.pkg.prefab.android.21:<PACKAGE-NAME>:<PACKAGE-VERSION>`, for example, `libpng` package has a version `1.6.37`, we could use it as follows:

```gradle
dependencies {
    implementation 'com.fpliu.ndk.pkg.prefab.android.21:libpng:1.6.37'
}
```

**step6. invoke [find_package(PACKAGE-NAME [REQUIRED] CONFIG)](https://cmake.org/cmake/help/latest/command/find_package.html) command in your Android project's CMakeLists.txt**

Every package provides several cmake imported targets:

|TARGET-NAME|example|summary|
|-|-|-|
|`<PACKAGE-NAME>::headers`|`libpng::headers`|C/C++ header files only|
|`<PACKAGE-NAME>::lib*.a`|`libpng::libpng16.a`|static library|
|`<PACKAGE-NAME>::lib*.so`|`libpng::libpng16.so`|shared library|
|`<PACKAGE-NAME>::*`|`libpng::libpng`|base on .pc files|

Following is a piece of codes show you how to link `libpng.a` which is provided by `libpng` package:

```cmake
find_package(libpng REQUIRED CONFIG)
target_link_libraries(app libpng::libpng.a)
```

or

```cmake
find_package(libpng CONFIG)
if (libpng_FOUND)
    target_link_libraries(app libpng::libpng.a)
endif()
```

**step7. configure C++ standard and STL in build.gradle**

```gradle
android {
    defaultConfig {
        externalNativeBuild {
            cmake {
                arguments '-DANDROID_STL=c++_shared'
                cppFlags  '-std=c++17'
            }
        }
    }
}
```

**Note:**

- This step is only required for packages that use `libc++`.
- If you link a shared library that depends on `libc++_shared.so`, then your Android app should use `libc++_shared.so` too.

**References:**

- <https://github.com/google/prefab>
- <https://developer.android.com/studio/projects/android-library#aar-contents>
- <https://developer.android.com/studio/build/dependencies?agpversion=4.1#using-native-dependencies>

**Examples:**

- <https://github.com/leleliu008/ndk-pkg-prefab-example-ffmpeg>
- <https://github.com/leleliu008/ndk-pkg-prefab-example-libphonenumber>
