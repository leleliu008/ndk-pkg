#include <unistd.h>

#include "core/log.h"

int ndkpkg_help() {
    if (isatty(STDOUT_FILENO)) {
        const char * str = ""
        COLOR_GREEN
        "A package manager for Android NDK to build C/C++/Rust/Go project.\n\n"
        "ndk-pkg <ACTION> [ARGUMENT...]\n\n"
        "ndk-pkg --help\n"
        "ndk-pkg -h\n"
        COLOR_OFF
        "    show help of this command.\n\n"
        COLOR_GREEN
        "ndk-pkg --version\n"
        "ndk-pkg -V\n"
        COLOR_OFF
        "    show version of this command.\n\n"
        COLOR_GREEN
        "ndk-pkg env\n"
        COLOR_OFF
        "    show basic information about this software.\n\n"
        COLOR_GREEN
        "ndk-pkg buildinfo\n"
        COLOR_OFF
        "    show build information about this software.\n\n"
        COLOR_GREEN
        "ndk-pkg sysinfo\n"
        COLOR_OFF
        "    show basic information about your current running operation system.\n\n"
        COLOR_GREEN
        "ndk-pkg ndkinfo [--ndk-home=<ANDROID-NDK-HOME>]\n"
        COLOR_OFF
        "    If --ndk-home=<ANDROID-NDK-HOME> is unspecified, then ANDROID_NDK_ROOT and ANDROID_NDK_HOME environment variable would be checked in order to determine the Android NDK's root directory.\n\n"
        COLOR_GREEN
        "ndk-pkg gen-url-transform-sample\n"
        COLOR_OFF
        "    generate url-transform sample.\n\n"
        COLOR_GREEN
        "ndk-pkg integrate zsh [-v] [--output-dir=<DIR>]\n"
        COLOR_OFF
        "    download a zsh completion script file to a approprivate location.\n\n"
        "    to apply this feature, you may need to run the command 'autoload -U compinit && compinit' in your terminal (your current running shell should be zsh).\n\n"
        COLOR_GREEN
        "ndk-pkg update\n"
        COLOR_OFF
        "    update all available formula repositories.\n\n"
        COLOR_GREEN
        "ndk-pkg upgrade-self\n"
        COLOR_OFF
        "    upgrade this software.\n\n"
        COLOR_GREEN
        "ndk-pkg cleanup\n"
        COLOR_OFF
        "    cleanup the unused cache.\n\n\n"
        COLOR_GREEN
        "ndk-pkg formula-repo-list\n"
        COLOR_OFF
        "    list all available formula repositories.\n\n"
        COLOR_GREEN
        "ndk-pkg formula-repo-add  <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n"
        COLOR_OFF
        "    create a new empty formula repository then sync with server.\n\n"
        COLOR_GREEN
        "ndk-pkg formula-repo-del  <FORMULA-REPO-NAME>\n"
        COLOR_OFF
        "    delete the given formula repository.\n\n"
        COLOR_GREEN
        "ndk-pkg formula-repo-init <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n"
        COLOR_OFF
        "    create a new empty formula repository.\n\n"
        COLOR_GREEN
        "ndk-pkg formula-repo-conf <FORMULA-REPO-NAME>      [--url=VALUE | --branch=VALUE --pin/--unpin --enable/--disable]\n"
        COLOR_OFF
        "    change the config of the given formula repository.\n\n"
        COLOR_GREEN
        "ndk-pkg formula-repo-sync <FORMULA-REPO-NAME>\n"
        COLOR_OFF
        "    sync the given formula repository with server.\n\n"
        COLOR_GREEN
        "ndk-pkg formula-repo-info <FORMULA-REPO-NAME>\n"
        COLOR_OFF
        "    show information of the given formula repository.\n\n\n"
        COLOR_GREEN
        "ndk-pkg search <REGULAR-EXPRESSION> [-v]\n"
        COLOR_OFF
        "    search all available packages whose name matches the given regular expression pattern.\n\n\n"
        COLOR_GREEN
        "ndk-pkg ls-available [-v]\n"
        COLOR_OFF
        "    list the available packages.\n\n"
        COLOR_GREEN
        "ndk-pkg ls-installed [-v]\n"
        COLOR_OFF
        "    list the installed packages.\n\n"
        COLOR_GREEN
        "ndk-pkg ls-outdated [-v]\n"
        COLOR_OFF
        "    list the outdated  packages.\n\n\n"
        COLOR_GREEN
        "ndk-pkg is-available <PACKAGE-NAME>\n"
        COLOR_OFF
        "    check if the given package is available.\n\n"
        COLOR_GREEN
        "ndk-pkg is-installed <PACKAGE-SPEC>\n"
        COLOR_OFF
        "    check if the given package is installed.\n\n"
        COLOR_GREEN
        "ndk-pkg is-outdated  <PACKAGE-SPEC>\n"
        COLOR_OFF
        "    check if the given package is outdated.\n\n\n"
        COLOR_GREEN
        "ndk-pkg formula-view <PACKAGE-NAME> [--no-color]\n"
        COLOR_OFF
        "    view the formula of the given package.\n\n"
        COLOR_GREEN
        "ndk-pkg formula-edit <PACKAGE-NAME> [--editor=EDITOR]\n"
        COLOR_OFF
        "    edit the formula of the given package.\n\n\n"
        COLOR_GREEN
        "ndk-pkg info-available <PACKAGE-NAME> [--json | --yaml | <KEY>]\n"
        COLOR_OFF
        "    show information of the given available package.\n\n"
        COLOR_GREEN
        "ndk-pkg info-installed <PACKAGE-SPEC> [--json | --yaml | <KEY>]\n"
        COLOR_OFF
        "    show information of the given installed package.\n\n\n"
        COLOR_GREEN
        "ndk-pkg depends <PACKAGE-NAME> [-t <OUTPUT-TYPE>] [-o <OUTPUT-PATH>]\n"
        COLOR_OFF
        "    show packages that are depended by the given package.\n\n"
        "    <OUTPUT-TYPE> should be one of <dot|box|svg|png>\n\n"
        "    If -o <OUTPUT-PATH> option is given, the result will be written to file, otherwise, the result will be written to stdout.\n\n"
        "   <OUTPUT-PATH> can be either the filepath or directory. If it is an existing directory or ends with slash, then it will be treated as a directory, otherwise, it will be treated as a filepath.\n\n"
        "    If <OUTPUT-PATH> is treated as a directory, then it will be expanded to <OUTPUT-PATH>/<PACKAGE-NAME>-dependencies.<OUTPUT-TYPE>\n\n"
        "    If <OUTPUT-PATH> is treated as a filepath, and if -t <OUTPUT-TYPE> option is not given, if <OUTPUT-PATH> ends with one of .dot|.box|.svg|.png, <OUTPUT-TYPE> will be the <OUTPUT-PATH> suffix, otherwise, <OUTPUT-TYPE> will be box.\n\n"
        "    If -t <OUTPUT-TYPE> and -o <OUTPUT-PATH> options both are not given, <OUTPU-TYPE> will be box and output to stdout.\n\n\n"
        COLOR_GREEN
        "ndk-pkg fetch <PACKAGE-NAME> [-v]\n"
        COLOR_OFF
        "    download resources of the given package to the local cache.\n\n\n"
        COLOR_GREEN
        "ndk-pkg install android-<ANDROID-API>-<ANDROID-ABIs>/<PACKAGE-NAME>... [INSTALL-OPTIONS]\n"
        COLOR_OFF
        "    install the given packages.\n\n"
        COLOR_RED
        "    C and C++ compiler for native build might be installed by yourself using your system's default package manager before running this command.\n\n"
        COLOR_OFF
        "    <ANDROID-ABIs> is a comma-separated list, any combination of arm64-v8a, armeabi-v7a, x86_64, x86\n\n"
        "    INSTALL-OPTIONS:\n\n"
        COLOR_PURPLE
        "        --ndk-home=<ANDROID-NDK-HOME>\n"
        COLOR_OFF
        "            If --ndk-home=<ANDROID-NDK-HOME> is unspecified, then ANDROID_NDK_ROOT and ANDROID_NDK_HOME environment variable would be checked in order to determine the Android NDK's root directory, if they both are not set or set a empty string or are invalid android ndk home directory, a proper version of Android NDK will be automatically installed for you via uppm.\n\n"
        COLOR_PURPLE
        "        --jobs=N\n"
        COLOR_OFF
        "            specify the number of jobs you can run in parallel.\n\n"
        COLOR_PURPLE
        "        -q\n"
        COLOR_OFF
        "            silent mode. no any messages will be output to terminal.\n\n"
        COLOR_PURPLE
        "        -v\n"
        COLOR_OFF
        "            verbose mode. many messages will be output to terminal.\n\n"
        COLOR_PURPLE
        "        -vv\n"
        COLOR_OFF
        "            very verbose mode. many many messages will be output to terminal.\n\n"
        COLOR_PURPLE
        "        --build-type=<debug|release>\n"
        COLOR_OFF
        "            specify the build type.\n\n"
        COLOR_PURPLE
        "        --link-type=<static-only|static-most|shared-only|shared-most>\n"
        COLOR_OFF
        "            specify the link strategy.\n\n"
        "            a fully  statically linked executable is a executable that without any additional library dependencies even the standard C library.\n"
        "            a mostly statically linked executable is a executable that without any additional library dependencies except the standard C library.\n\n"
        "            a fully  statically linked executable is easy to distribute to all version of Android devices.\n"
        "            a mostly statically linked executable is easy to distribute to one version of Android devices.\n\n"
        COLOR_PURPLE
        "        --disable-ccache\n"
        COLOR_OFF
        "            do not use ccache.\n\n"
        COLOR_PURPLE
        "        --export-compile-commands-json\n"
        COLOR_OFF
        "            generates compile_commands.json\n\n"
        COLOR_PURPLE
        "        --upgrade\n"
        COLOR_OFF
        "            If this package has been already installed, then it will be upgraded if it could be.\n\n"
        COLOR_PURPLE
        "        --dry-run\n"
        COLOR_OFF
        "            do not actually install the package.\n\n"
        COLOR_PURPLE
        "        --keep-session-dir\n"
        COLOR_OFF
        "            do not delete the session directory even if this package is successfully installed.\n\n"
        "    USAGE-EXAMPLES:\n\n"
        "        ndk-pkg install android-21-arm64-v8a/zlib\n"
        "        ndk-pkg install android-21-arm64-v8a,armeabi-v7a/zlib\n"
        "        ndk-pkg install android-21-arm64-v8a,armeabi-v7a,x86_64/zlib\n"
        "        ndk-pkg install android-21-arm64-v8a,armeabi-v7a,x86_64,x86/zlib\n\n"
        COLOR_GREEN
        "ndk-pkg   upgrade android-<ANDROID-API>-<ANDROID-ABIs>/<PACKAGE-NAME>... [INSTALL-OPTIONS]\n"
        COLOR_OFF
        "    upgrade the given packages or all outdated packages.\n\n"
        COLOR_GREEN
        "ndk-pkg reinstall android-<ANDROID-API>-<ANDROID-ABIs>/<PACKAGE-NAME>... [INSTALL-OPTIONS]\n"
        COLOR_OFF
        "    reinstall the given packages.\n\n"
        COLOR_GREEN
        "ndk-pkg uninstall <PACKAGE-SPEC>...\n"
        COLOR_OFF
        "    uninstall the given packages.\n\n\n"
        COLOR_GREEN
        "ndk-pkg logs <PACKAGE-SPEC>\n"
        COLOR_OFF
        "    show logs of the given installed package.\n\n"
        COLOR_GREEN
        "ndk-pkg tree <PACKAGE-SPEC> [--dirsfirst | -a | -L N]\n"
        COLOR_OFF
        "    list files of the given installed package in a tree-like format.\n\n"
        COLOR_GREEN
        "ndk-pkg pack <PACKAGE-SPEC> [--keep-session-dir] [-t <OUTPUT-TYPE>] [-o <OUTPUT-PATH>]\n"
        COLOR_OFF
        "    pack the given installed package.\n\n"
        "    <OUTPUT-TYPE> should ends with one of tar.gz tar.xz tar.lz tar.bz2 zip\n\n"
        "    <OUTPUT-PATH> can be either the filepath or directory which indicates where the packed archive file would be written to.\n\n"
        "    If <OUTPUT-PATH> is . .. ./ ../ or ends with slash(/), then it would be treated as a directory, otherwise, it would be treated as a filepath.\n\n"
        "    If <OUTPUT-PATH> is treated as a filepath, then it should ends with one of .tar.gz .tar.xz .tar.lz .tar.bz2 .tgz .txz .tlz .tbz2 .zip , in this case, -t <OUTPUT-TYPE> option would be ignored if it also is specified.\n\n"
        "    If <OUTPUT-PATH> is treated as a directory, then it would be expanded to <OUTPUT-PATH>/<PACKAGE-NAME>-<PACKAGE-VERSION>-<TARGET-PLATFORM-NAME>-<TARGET-PLATFORM-ARCH><OUTPUT-TYPE>\n\n"
        "    If -o <OUTPUT-PATH> option is unspecified, <OUTPUT-PATH> would be assigned to ./<PACKAGE-NAME>-<PACKAGE-VERSION>-<TARGET-PLATFORM-NAME>-<TARGET-PLATFORM-ARCH><OUTPUT-TYPE>\n\n"
        "    If -t <OUTPUT-TYPE> option is unspecified, <OUTPUT-TYPE> would be assigned to .tar.xz\n\n\n"
        COLOR_GREEN
        "ndk-pkg export android-<ANDROID-API>-<ANDROID-ABIs>/<PACKAGE-NAME> [--ndk-home=<ANDROID-NDK-HOME>] [-o <OUTPUT-PATH>]\n"
        COLOR_OFF
        "    export the given installed package as google prefab aar.\n\n"
        "    For details about google prefab, please visit https://google.github.io/prefab/\n\n"
        "    For details about the aar file contains google prefab, please visit https://developer.android.com/studio/projects/android-library#aar-contents\n\n"
        "    <ANDROID-ABIs> is a comma-separated list, any combination of arm64-v8a, armeabi-v7a, x86_64, x86\n\n"
        "    If --ndk-home=<ANDROID-NDK-HOME> is unspecified, then ANDROID_NDK_ROOT and ANDROID_NDK_HOME environment variable would be checked in order to determine the Android NDK's root directory, if they both are not set or set a empty string or are invalid android ndk home directory, a proper version of Android NDK will be automatically installed for you via uppm.\n\n"
        "    <OUTPUT-PATH> can be either the filepath or directory where the aar file will be written to.\n\n"
        "    If <OUTPUT-PATH> is an existing directory or ends with slash, then it will be treated as a directory, otherwise, it will be treated as a filepath.\n\n"
        "    If <OUTPUT-PATH> is treated as a directory, then it would be expanded to <OUTPUT-PATH>/<PACKAGE-NAME>-<PACKAGE-VERSION>.aar\n\n\n"
        COLOR_GREEN
        "ndk-pkg deploy android-<ANDROID-API>-<ANDROID-ABIs>/<PACKAGE-NAME> [--ndk-home=<ANDROID-NDK-HOME>] [--debug] [--keep-session-dir] [--dry-run] [--local=<DIR>] [--remote < REMOTE-CONFIG-FILE]\n"
        COLOR_OFF
        "    export the given installed package as google prefab aar then deploy it to Maven Repository.\n\n"
        "    For details about google prefab, please visit https://google.github.io/prefab/\n\n"
        "    For details about the aar file contains google prefab, please visit https://developer.android.com/studio/projects/android-library#aar-contents\n\n"
        "    <ANDROID-ABIs> is a comma-separated list, any combination of arm64-v8a, armeabi-v7a, x86_64, x86\n\n"
        "    If --ndk-home=<ANDROID-NDK-HOME> is unspecified, then ANDROID_NDK_ROOT and ANDROID_NDK_HOME environment variable would be checked in order to determine the Android NDK's root directory, if they both are not set or set a empty string or are invalid android ndk home directory, a proper version of Android NDK will be automatically installed for you via uppm.\n\n"
        "    If --remote is specified, the aar file would be deployed to Sonatype OSSRH (https://s01.oss.sonatype.org/)\n\n"
        "    If --local=<DIR> is specified, the aar file would be deployed to the specified location of Maven Local Repository.\n\n"
        "    If neither --local=<DIR> nor --remote is specified, the aar file would be deployed to your default Maven Local Repository (usually under ~/.m2/repository/ directory, depending on your settings.xml configuration)\n\n"
        "    REMOTE-CONFIG-FILE contains following content:\n\n"
        "        SERVER_ID=\n"
        "        SERVER_URL=\n"
        "        SERVER_USERNAME=\n"
        "        SERVER_PASSWORD=\n"
        "        GPG_PASSPHRASE=\n\n\n"
        COLOR_GREEN
        "ndk-pkg util zlib-deflate -L <LEVEL> < input/file/path\n"
        COLOR_OFF
        "    compress data using zlib deflate algorithm.\n\n"
        "    LEVEL >= 1 && LEVEL <= 9\n\n"
        "    The smaller the LEVEL, the faster the speed and the lower the compression ratio.\n\n"
        COLOR_GREEN
        "ndk-pkg util zlib-inflate < input/file/path\n"
        COLOR_OFF
        "    decompress data using zlib inflate algorithm.\n\n"
        COLOR_GREEN
        "ndk-pkg util base16-encode <STR>\n"
        COLOR_OFF
        "    encode <STR> using base16 algorithm.\n\n"
        COLOR_GREEN
        "ndk-pkg util base16-encode < input/file/path\n"
        COLOR_OFF
        "    encode data using base16 algorithm.\n\n"
        COLOR_GREEN
        "ndk-pkg util base16-decode <BASE16-ENCODED-SUM>\n"
        COLOR_OFF
        "    decode <BASE16-ENCODED-SUM> using base16 algorithm.\n\n"
        COLOR_GREEN
        "ndk-pkg util base16-decode < input/file/path\n"
        COLOR_OFF
        "    decode data using base16 algorithm.\n\n"
        COLOR_GREEN
        "ndk-pkg util base64-encode <STR>\n"
        COLOR_OFF
        "    encode <STR> using base64 algorithm.\n\n"
        COLOR_GREEN
        "ndk-pkg util base64-encode < input/file/path\n"
        COLOR_OFF
        "    encode data using base64 algorithm.\n\n"
        COLOR_GREEN
        "ndk-pkg util base64-decode <BASE64-ENCODED-SUM>\n"
        COLOR_OFF
        "    decode <BASE64-ENCODED-SUM> using base64 algorithm.\n\n"
        COLOR_GREEN
        "ndk-pkg util base64-decode < input/file/path\n"
        COLOR_OFF
        "    decode data using base64 algorithm.\n\n"
        COLOR_GREEN
        "ndk-pkg util sha256sum <input/file/path>\n"
        COLOR_OFF
        "    calculate sha256sum of file.\n\n"
        COLOR_GREEN
        "ndk-pkg util sha256sum < input/file/path\n"
        COLOR_OFF
        "    calculate sha256sum of file.\n\n"
        COLOR_GREEN
        "ndk-pkg util which <COMMAND-NAME> [-a]\n"
        COLOR_OFF
        "    find <COMMAND-NAME> in PATH.\n\n\n"

        "=====================\n"
        "Environment variables\n"
        "=====================\n\n"
        COLOR_GREEN
        "HOME\n"
        COLOR_OFF
        "    This environment variable already have been set on the most operating systems, if not set or set a empty string, you will receive an error message.\n\n"
        COLOR_GREEN
        "PATH\n"
        COLOR_OFF
        "    This environment variable already have been set on the most operating systems, if not set or set a empty string, you will receive an error message.\n\n"
        COLOR_GREEN
        "ANDROID_NDK_ROOT and ANDROID_NDK_HOME\n"
        COLOR_OFF
        "    If --ndk-home=<ANDROID-NDK-HOME> is unspecified, then ANDROID_NDK_ROOT and ANDROID_NDK_HOME environment variable would be checked in order to determine the Android NDK's root directory, if they both are not set or set a empty string or are invalid android ndk home directory, a proper version of Android NDK will be automatically installed for you via uppm.\n\n"
        COLOR_GREEN
        "SSL_CERT_FILE\n"
        COLOR_OFF
        "    In general, you don't need to set this environment variable, but, if you encounter the reporting the SSL certificate is invalid, trying to run below commands in your terminal will do the trick.\n\n"
        "    curl -LO https://curl.se/ca/cacert.pem\n"
        "    export SSL_CERT_FILE=$PWD/cacert.pem\n\n"
        COLOR_GREEN
        "GOPROXY\n"
        COLOR_OFF
        "    Example: export GOPROXY='https://goproxy.cn'\n\n"
        COLOR_GREEN
        "NDKPKG_URL_TRANSFORM\n"
        COLOR_OFF
        "    If you want to change the request url, you can set this environment variable. It is very useful for chinese users.\n\n"
        "    Example: export NDKPKG_URL_TRANSFORM=/path/of/url-transform\n\n"

        "    /path/of/url-transform command would be invoked as /path/of/url-transform <URL>\n\n"
        "    /path/of/url-transform command should output a <URL>\n\n"
        "    a url-transform sample can be generated via running ndk-pkg gen-url-transform-sample\n\n"
        COLOR_GREEN
        "NDKPKG_XTRACE\n"
        COLOR_OFF
        "    for debugging purposes. to enable set -x: export NDKPKG_XTRACE=1\n\n"

        COLOR_GREEN
        "NDKPKG_DEFAULT_TARGET\n"
        COLOR_OFF
        "    some ACTIONs of ndk-pkg need <PACKAGE-SPEC> to be specified.\n\n"
        COLOR_PURPLE
        "    <PACKAGE-SPEC> : "
        COLOR_OFF
        "a formatted string that has form: <TARGET>/<PACKAGE-NAME>, represents an installed package.\n\n"
        COLOR_PURPLE
        "    <PACKAGE-NAME> : "
        COLOR_OFF
        "should match regular expression pattern ^[A-Za-z0-9+-_.@]{1,50}$\n\n"
        COLOR_PURPLE
        "    <TARGET>       : "
        COLOR_OFF
        "a formatted string that has form: android-<ANDROID-API>-<ANDROID-ABI>\n\n"
        COLOR_PURPLE
        "    <ANDROID-API>  : "
        COLOR_OFF
        "indicates which minimum Android SDK API level was built with. Reference: https://developer.android.com/tools/releases/platforms\n\n"
        COLOR_PURPLE
        "    <ANDROID-ABI>  : "
        COLOR_OFF
        "indicates which Android ABI was built for. Reference: https://developer.android.com/ndk/guides/abis\n\n"
        "    To simplify the usage, you are allowed to omit <TARGET>/. If <TARGET>/ is omitted, environment variable NDKPKG_DEFAULT_TARGET would be checked, if this environment variable is not set, then android-21-arm64-v8a will be used as the default.\n\n"
        "    Example: export NDKPKG_DEFAULT_TARGET='android-21-arm64-v8a'\n\n\n"

        "=======\n"
        "Caveats\n"
        "=======\n\n"
        "This software is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.\n\n"
        "This software can NOT run on Android, musl-libc based GNU/Linux, FreeBSD, OpenBSD, NetBSD due to lack of Android NDK for these platforms.\n\n"
        "This software can NOT run on Cygwin and MSYS2 due to CMake: Builds hosted on 'CYGWIN' not supported. Android-Determine.cmake\n\n"
        "Please do NOT place your own files under ~/.ndk-pkg directory, as ndk-pkg will change files under ~/.ndk-pkg directory without notice.\n\n"
        "Please do NOT run ndk-pkg command in parallel to avoid generating dirty data.\n"
        ;

        printf("%s\n", str);
    } else {
        const char * str = ""
        "A package manager for Android NDK to build C/C++/Rust/Go project.\n\n"
        "ndk-pkg <ACTION> [ARGUMENT...]\n\n"
        "ndk-pkg --help\n"
        "ndk-pkg -h\n"
        "    show help of this command.\n\n"
        "ndk-pkg --version\n"
        "ndk-pkg -V\n"
        "    show version of this command.\n\n"
        "ndk-pkg env\n"
        "    show basic information about this software.\n\n"
        "ndk-pkg buildinfo\n"
        "    show build information about this software.\n\n"
        "ndk-pkg sysinfo\n"
        "    show basic information about your current running operation system.\n\n"
        "ndk-pkg update\n"
        "    update all available formula repositories.\n\n"
        "ndk-pkg search <REGULAR-EXPRESSION> [-v]\n"
        "    search all available packages whose name matches the given regular expression pattern.\n\n"
        "ndk-pkg info-available <PACKAGE-NAME>\n"
        "    show information of the given available package.\n\n"
        "ndk-pkg info-installed <PACKAGE-NAME>\n"
        "    show information of the given installed package.\n\n"
        "ndk-pkg tree <PACKAGE-NAME> [--dirsfirst | -a | -L N]\n"
        "    list files of the given installed package in a tree-like format.\n\n"
        "ndk-pkg pack <PACKAGE-NAME> -t <7z|zip|tar.gz|tar.xz|tar.bz2>\n"
        "    pack the given installed package.\n\n"
        "ndk-pkg logs <PACKAGE-NAME>\n"
        "    show logs of the given installed package.\n\n"
        "ndk-pkg depends <PACKAGE-NAME> [-t <OUTPUT-TYPE>] [-o <OUTPUT-PATH>]\n"
        "    show packages that are depended by the given package.\n\n"
        "    <OUTPUT-TYPE> should be one of <dot|box|svg|png>\n\n"
        "    If -o <OUTPUT-PATH> option is given, the result will be written to file, otherwise, the result will be written to stdout.\n\n"
        "   <OUTPUT-PATH> can be either the filepath or directory. If it is an existing directory or ends with slash, then it will be treated as a directory, otherwise, it will be treated as a filepath.\n\n"
        "    If <OUTPUT-PATH> is treated as a directory, then it will be expanded to <OUTPUT-PATH>/<PACKAGE-NAME>-dependencies.<OUTPUT-TYPE>\n\n"
        "    If <OUTPUT-PATH> is treated as a filepath, and if -t <OUTPUT-TYPE> option is not given, if <OUTPUT-PATH> ends with one of .dot|.box|.svg|.png, <OUTPUT-TYPE> will be the <OUTPUT-PATH> suffix, otherwise, <OUTPUT-TYPE> will be box.\n\n"
        "    If -t <OUTPUT-TYPE> and -o <OUTPUT-PATH> options both are not given, <OUTPU-TYPE> will be box and output to stdout.\n\n"
        "ndk-pkg fetch   <PACKAGE-NAME> [-v]\n"
        "    download resources of the given package to the local cache.\n\n"
        "ndk-pkg   install <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-session-dir]\n"
        "    install the given packages.\n\n"
        "ndk-pkg   upgrade [PACKAGE-NAME]... [--jobs=N -q -v --dry-run --keep-session-dir]\n"
        "    upgrade the given packages or all outdated packages.\n\n"
        "ndk-pkg reinstall <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-session-dir]\n"
        "    reinstall the given packages.\n\n"
        "ndk-pkg uninstall <PACKAGE-NAME>\n"
        "    uninstall the given packages.\n\n"
        "ndk-pkg ls-available [-v]\n"
        "    list the available packages.\n\n"
        "ndk-pkg ls-installed [-v]\n"
        "    list the installed packages.\n\n"
        "ndk-pkg ls-outdated [-v]\n"
        "    list the outdated  packages.\n\n"
        "ndk-pkg is-available <PACKAGE-NAME>\n"
        "    check if the given package is available.\n\n"
        "ndk-pkg is-installed <PACKAGE-NAME>\n"
        "    check if the given package is installed.\n\n"
        "ndk-pkg is-outdated  <PACKAGE-NAME>\n"
        "    check if the given package is outdated.\n\n"
        "ndk-pkg formula-view <PACKAGE-NAME> [--no-color]\n"
        "    view the formula of the given package.\n\n"
        "ndk-pkg formula-edit <PACKAGE-NAME> [--editor=EDITOR]\n"
        "    edit the formula of the given package.\n\n"
        "ndk-pkg formula-repo-add  <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n"
        "    create a new empty formula repository then sync with server.\n\n"
        "ndk-pkg formula-repo-del <FORMULA-REPO-NAME>\n"
        "    delete the given formula repository.\n\n"
        "ndk-pkg formula-repo-init <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n"
        "    create a new empty formula repository.\n\n"
        "ndk-pkg formula-repo-conf <FORMULA-REPO-NAME>      [--url=VALUE | --branch=VALUE --pin/--unpin --enable/--disable]\n"
        "    change the config of the given formula repository.\n\n"
        "ndk-pkg formula-repo-sync <FORMULA-REPO-NAME>\n"
        "    sync the given formula repository with server.\n\n"
        "ndk-pkg formula-repo-info <FORMULA-REPO-NAME>\n"
        "    show information of the given formula repository.\n\n"
        "ndk-pkg formula-repo-list\n"
        "    list all available formula repositories.\n\n"
        "ndk-pkg gen-url-transform-sample\n"
        "    generate url-transform sample.\n\n"
        "ndk-pkg cleanup\n"
        "    cleanup the unused cache.\n"
        "ndk-pkg util zlib-deflate -L <LEVEL> < input/file/path\n"
        "    compress data using zlib deflate algorithm.\n\n"
        "    LEVEL >= 1 && LEVEL <= 9\n\n"
        "    The smaller the LEVEL, the faster the speed and the lower the compression ratio.\n\n"
        "ndk-pkg util zlib-inflate < input/file/path\n"
        "    decompress data using zlib inflate algorithm.\n\n"
        "ndk-pkg util base16-encode <STR>\n"
        "    encode <STR> using base16 algorithm.\n\n"
        "ndk-pkg util base16-encode < input/file/path\n"
        "    encode data using base16 algorithm.\n\n"
        "ndk-pkg util base16-decode <BASE16-ENCODED-SUM>\n"
        "    decode <BASE16-ENCODED-SUM> using base16 algorithm.\n\n"
        "ndk-pkg util base16-decode < input/file/path\n"
        "    decode data using base16 algorithm.\n\n"
        "ndk-pkg util base64-encode <STR>\n"
        "    encode <STR> using base64 algorithm.\n\n"
        "ndk-pkg util base64-encode < input/file/path\n"
        "    encode data using base64 algorithm.\n\n"
        "ndk-pkg util base64-decode <BASE64-ENCODED-SUM>\n"
        "    decode <BASE64-ENCODED-SUM> using base64 algorithm.\n\n"
        "ndk-pkg util base64-decode < input/file/path\n"
        "    decode data using base64 algorithm.\n\n"
        "ndk-pkg util sha256sum <input/file/path>\n"
        "    calculate sha256sum of file.\n\n"
        "ndk-pkg util sha256sum < input/file/path\n"
        "    calculate sha256sum of file.\n\n"
        "ndk-pkg util which <COMMAND-NAME> [-a]\n"
        "    find <COMMAND-NAME> in PATH.\n"
        ;

        printf("%s\n", str);
    }

    return 0;
}
