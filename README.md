# ndk-pkg

A package manager for [Android NDK](https://developer.android.google.cn/ndk) to build projects that are written in C/C++/Rust/Golang.

## Caveats

Please read these caveats carefully before starting to use this software.

- This software is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.

- This software can NOT run on [Android](https://www.android.com/), [musl-libc](http://musl.libc.org/) based [GNU/Linux](https://www.gnu.org/gnu/linux-and-gnu.en.html), [FreeBSD](https://www.freebsd.org/), [OpenBSD](https://www.openbsd.org/), [NetBSD](https://www.netbsd.org/) due to lack of [Android NDK](https://developer.android.google.cn/ndk) for these platforms.

- This software can NOT run on [Cygwin](http://cygwin.org/) and [MSYS2](https://www.msys2.org/) due to `CMake: Builds hosted on 'CYGWIN' not supported.` [Android-Determine.cmake](https://github.com/Kitware/CMake/blob/master/Modules/Platform/Android-Determine.cmake#L271-L291)

- Please do NOT place your own files under `~/.ndk-pkg` directory, as `ndk-pkg` will change files under `~/.ndk-pkg` directory without notice.

- Please do NOT run `ndk-pkg` command in parallell to avoid generating dirty data.

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

**step3. run `ndk-pkg` command in the ndk-pkg docker container**

```bash
docker exec -it ndk-pkg ndk-pkg setup
docker exec -it ndk-pkg ndk-pkg upgrade-self
docker exec -it ndk-pkg ndk-pkg update
```

If all goes well, then next you can start to install packages whatever you want, for example, let's install `curl` package:

```bash
docker exec -it ndk-pkg ndk-pkg install curl:android-21:arm64-v8a
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

_instructions installing ndk-pkg on WSL is same as described in section [Install ndk-pkg via cURL](https://github.com/leleliu008/ndk-pkg#install-ndk-pkg-via-curl)_

**Things You Should Do Immediately After Installing ndk-pkg on WSL** :

```bash
sudo install -o $(whoami) -d /mnt/d/ndk-pkg
ln -sf /mnt/d/ndk-pkg ~/.ndk-pkg
```

## ~/.ndk-pkg

all relevant directories and files are located under `~/.ndk-pkg` directory.

a typical hierarchical structure under `~/.ndk-pkg` directory is as follows:

```text
~/.ndk-pkg
├── core
│   ├── bin
│   │   └── uppm
│   ├── etc
│   │   ├── cacert.pem
│   │   └── profile
│   ├── libexec
│   │   ├── wrapper-native-c++
│   │   ├── wrapper-native-cc
│   │   ├── wrapper-native-objc
│   │   ├── wrapper-target-c++
│   │   └── wrapper-target-cc
│   ├── share
│   │   └── zsh
│   │       └── site-functions
│   │           └── _uppm
│   └── xxxx
├── downloads
│   ├── 8f74213b56238c85a50a5329f77e06198771e70dd9a739779f4c02f65d971313.tgz
│   ├── b3a24de97a8fdbc835b9833169501030b8977031bcb54b3b3ac13740f846ab30.tgz
│   └── c642ae9b75fee120b2d96c712538bd2cf283228d2337df2cf2988e3c02678ef4.tgz
├── installed
│   ├── 356a9aa408c53bb93f85a768b1a8a2dd5cb4851992b9a59964bafeefca808881
│   │   ├── .ndk-pkg
│   │   │   ├── FAQ
│   │   │   ├── LICENSE
│   │   │   ├── MANIFEST.txt
│   │   │   ├── METADATA.yml
│   │   │   └── README
│   │   ├── include
│   │   │   ├── zconf.h
│   │   │   └── zlib.h
│   │   ├── lib
│   │   │   ├── pkgconfig
│   │   │   │   └── zlib.pc
│   │   │   ├── libz.a
│   │   │   └── libz.so
│   │   ├── lib-for-apk
│   │   │   ├── cmake
│   │   │   │   └── zlib
│   │   │   │       ├── zlibConfig.cmake
│   │   │   │       └── zlibConfigVersion.cmake
│   │   │   ├── libz.a
│   │   │   └── libz.so
│   │   └── share
│   │       └── man
│   │           └── man3
│   │               └── zlib.3
│   ├── 1f6e3471cc3fed6ac255d262c4b151fff1e73e178e266476eced394cd7a7a286
│   │   ├── .ndk-pkg
│   │   │   ├── FAQ
│   │   │   ├── LICENSE
│   │   │   ├── MANIFEST.txt
│   │   │   ├── METADATA.yml
│   │   │   └── README
│   │   └── ...
│   │       └── ...
│   ├── f1dd85386c380267b40759f291da65ecfde1375f0c64ada692aebc3fbe1834d8
│   │   └── ...
│   │       └── ...
│   ├── zlib:android-21:arm64-v8a -> 356a9aa408c53bb93f85a768b1a8a2dd5cb4851992b9a59964bafeefca808881
│   ├── zlib:android-21:armeabi-v7a -> 1f6e3471cc3fed6ac255d262c4b151fff1e73e178e266476eced394cd7a7a286
│   ├── zlib:android-21:x86 -> f1dd85386c380267b40759f291da65ecfde1375f0c64ada692aebc3fbe1834d8
│   └── zlib:android-21:x86_64 -> fe901c866e12f2f6816c269b774f9ca48a83f5c269e515a320fae7a7bcc0d791
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
│   └── offical-core
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
    - [curl](https://curl.se/docs/manpage.html)
    - [git](https://git-scm.com/docs/git)
    - [GNU tar](https://www.gnu.org/software/tar/manual/tar.html)
    - [gzip](https://www.gnu.org/software/gzip/manual/gzip.html)
    - [lzip](https://www.nongnu.org/lzip/)
    - [bzip2](https://linux.die.net/man/1/bzip2)
    - [xz](https://linux.die.net/man/1/xz)
    - [zip](https://linux.die.net/man/1/zip)
    - [unzip](https://linux.die.net/man/1/unzip)
    - [yq](https://mikefarah.gitbook.io/yq/)
    - [jq](https://stedolan.github.io/jq/manual/)
    - [tree](https://linux.die.net/man/1/tree)

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
    ndk-pkg info curl src-ft
    ndk-pkg info curl src-fp

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
    ndk-pkg formula curl src-ft
    ndk-pkg formula curl src-fp
    ```

- **show receipt of the given installed package**

    ```bash
    ndk-pkg receipt curl
    ndk-pkg receipt curl:android-21:arm64-v8a
    ndk-pkg receipt curl:android-21:arm64-v8a --yaml
    ndk-pkg receipt curl:android-21:arm64-v8a --json
    ndk-pkg receipt curl:android-21:arm64-v8a --path
    ndk-pkg receipt curl:android-21:arm64-v8a version
    ndk-pkg receipt curl:android-21:arm64-v8a license
    ndk-pkg receipt curl:android-21:arm64-v8a summary
    ndk-pkg receipt curl:android-21:arm64-v8a web-url
    ndk-pkg receipt curl:android-21:arm64-v8a git-url
    ndk-pkg receipt curl:android-21:arm64-v8a git-sha
    ndk-pkg receipt curl:android-21:arm64-v8a git-ref
    ndk-pkg receipt curl:android-21:arm64-v8a src-url
    ndk-pkg receipt curl:android-21:arm64-v8a src-sha
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
    ndk-pkg install curl:android-33:arm64-v8a
    ndk-pkg install curl:android-33:arm64-v8a,x86_64 --link-type=static-only
    ```

- **reinstall packages**

    ```bash
    ndk-pkg reinstall curl
    ndk-pkg reinstall curl:android-33:arm64-v8a --link-type=static-only
    ```

- **uninstall packages**

    ```bash
    ndk-pkg uninstall curl
    ndk-pkg uninstall curl:android-33:arm64-v8a
    ```

- **upgrade the outdated packages**

    ```bash
    ndk-pkg upgrade curl
    ndk-pkg upgrade curl:android-33:arm64-v8a --link-type=static-only
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

    **Caveats**: `ndk-pkg` does NOT save your changes, which means that your changes may be lost after the formula repository is updated!

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
    ndk-pkg is-installed curl:android-21:arm64-v8a
    ```

- **check if the given package is outdated**

    ```bash
    ndk-pkg is-outdated  curl
    ndk-pkg is-outdated  curl:android-21:arm64-v8a
    ```

- **list installed files of the given installed package in a tree-like format**

    ```bash
    ndk-pkg tree curl
    ndk-pkg tree curl:android-21:arm64-v8a -L 3
    ```

- **show logs of the given installed package**

    ```bash
    ndk-pkg logs curl
    ndk-pkg logs curl curl:android-21:arm64-v8a
    ```

- **pack the given installed package**

    ```bash
    ndk-pkg pack curl
    ndk-pkg pack curl:android-21:arm64-v8a
    ndk-pkg pack curl:android-21:arm64-v8a -t tar.xz
    ndk-pkg pack curl:android-21:arm64-v8a -t tar.gz
    ndk-pkg pack curl:android-21:arm64-v8a -t tar.lz
    ndk-pkg pack curl:android-21:arm64-v8a -t tar.bz2
    ndk-pkg pack curl:android-21:arm64-v8a -t zip
    ndk-pkg pack curl:android-21:arm64-v8a -t zip -o a/
    ndk-pkg pack curl:android-21:arm64-v8a -o a/x.zip
    ```

- **export the given installed package as the google prefab aar**

    ```bash
    ndk-pkg export curl:android-21:arm64-v8a,x86_64 -o .
    ndk-pkg export curl:android-21:arm64-v8a,x86_64 -o curl-8.1.2.aar
    ```

- **export the given installed package as the google prefab aar then deploy it to Maven Local Repository**

    ```bash
    ndk-pkg depoly curl:android-21:arm64-v8a,x86_64
    ndk-pkg depoly curl:android-21:arm64-v8a,x86_64 --debug
    ndk-pkg depoly curl:android-21:arm64-v8a,x86_64 --local=/somewhere
    ```

- **export the given installed package as the google prefab aar then deploy it to Sonatype OSSRH**

    ```bash
    ndk-pkg depoly curl:android-21:arm64-v8a,x86_64 --remote < ~/OSSRH-config
    ndk-pkg depoly curl:android-21:arm64-v8a,x86_64 --remote <<EOF
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

- **NDKPKG_DEFAULT_TARGET_ANDROID_SPEC**

    some sub-commands of `ndk-pkg` need `<PACKAGE-SPEC>` to be specified. `<PACKAGE-SPEC>` has the form `<PACKAGE-NAME>:<TARGET-ANDROID-SPEC>`, To simplify the usage, you are allowed to omit `:<TARGET-ANDROID-SPEC>`. If `:<TARGET-ANDROID-SPEC>` is omitted, this environment variable will be used, if this environment variable is not set, then `android-21:arm64-v8a` will be used as the default.

    `<TARGET-ANDROID-SPEC>` has the form `android-<MIN-SDK-API-LEVEL>:<ANDROID-ABI>`

    **Example**:

    ```bash
    export NDKPKG_DEFAULT_TARGET_ANDROID_SPEC='android-21:arm64-v8a'
    ```

    **References**:

  - <https://developer.android.com/tools/releases/platforms>
  - <https://developer.android.com/ndk/guides/abis>

**Note:** some commonly used environment variables are override by this software, these are `CC`, `CXX`, `CPP`, `AS`, `AR`, `LD`, `CFLAGS`, `CPPFLAGS`, `LDFLAGS`, `PKG_CONFIG_LIBDIR`, `PKG_CONFIG_PATH`, `ACLOCAL_PATH`

## ndk-pkg formula

a ndk-pkg formula is a [YAML](https://yaml.org/spec/1.2.2/) format file which is used to config a ndk-pkg package's meta-information including one sentence description, package version, installation instructions, etc.

a ndk-pkg formula's filename suffix must be `.yml`

a ndk-pkg formula'a filename prefix would be treated as the package name.

a ndk-pkg formula'a filename prefix must match the regular expression pattern `^[A-Za-z0-9+-._@]{1,50}$`

a ndk-pkg formula's file content must follow [the ndk-pkg formula scheme](https://github.com/leleliu008/ndk-pkg-formula-repository-offical-core)

## ndk-pkg formula repository

a ndk-pkg formula repository is a git repository.

a ndk-pkg formula repository's root directory should have a `formula` named sub directory, this repository's formulas all should be located in this directory.

a ndk-pkg formula repository's local path is `~/.ndk-pkg/repos.d/${ndk-pkgFormulaRepoName}`

ndk-pkg supports multiple formula repositories.

## ndk-pkg formula repository's config

After a ndk-pkg formula repository is successfully fetched from server to local, a config file for this repository would be created at `~/.ndk-pkg/repos.d/${ndk-pkgFormulaRepoName}/.ndk-pkg-formula-repo.yml`

a typical ndk-pkg formula repository's config is as follows:

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

ndk-pkg offical formula repository is hosted at <https://github.com/leleliu008/ndk-pkg-formula-repository-offical-core>

ndk-pkg offical formula repository would be automatically fetched to local cache as name `offical-core` when you run `ndk-pkg update` command.

**Note:** If you find that a package is not in ndk-pkg offical formula repository yet, PR is welcomed.

## Using my prefab aars that have been published to GitHub-Hosted Maven Repository alongside with Android Gradle Plugin

I have published some commonly used packages as google prefab aar to <https://github.com/leleliu008/ndk-pkg-prefab-aar-maven-repo>

## Using my prefab aars that have been published to Maven Central Repository alongside with Android Gradle Plugin

I have published some commonly used packages as google prefab aar to `Maven Central Repository`.

To get the full list of my published packages, please visit the follwoing websites:

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

## Create the google prefab aar then deploy it to Maven Local Reposotory and use it alongside with Android Gradle Plugin

If my published packages don't meet your needs, you can use this software to install packages then deploy them to `Maven Local Reposotory`.

In the next two sections, I will show you how to configure with `Android Gradle Plugin` in `Kotlin DSL` and `Groovy DSL` respectively.

## configure with Android Gradle Plugin Kotlin DSL

**step1. build and install libpng**

Suppose you want to build and install `libpng`, the following command will build `libpng` with `android-21` API and build for `arm64-v8a` and `armeabi-v7a` ABI respectively.

```bash
ndk-pkg install libpng:android-21:arm64-v8a,armeabi-v7a
```

**step2. export the installed libpng package as the google prefab aar and deploy it to your Maven Local Reposotory**

```bash
ndk-pkg deploy  libpng:android-21:arm64-v8a,armeabi-v7a
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

Suppose you want to build and install `libpng`, the following command will build `libpng` with `android-21` API and build for `arm64-v8a` and `armeabi-v7a` ABI respectively.

```bash
ndk-pkg install libpng:android-21:arm64-v8a,armeabi-v7a
```

**step2. export the installed libpng package as the google prefab aar and deploy it to your Maven Local Reposotory**

```bash
ndk-pkg deploy  libpng:android-21:arm64-v8a,armeabi-v7a
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
