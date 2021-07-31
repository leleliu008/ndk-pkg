#!/bin/sh


# https://github.com/leleliu008/ndk-pkg/blob/master/install.sh


unset CURRENT_SCRIPT_DIR
unset CURRENT_SCRIPT_FILENAME
unset CURRENT_SCRIPT_FILEPATH

CURRENT_SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd) || exit 1
CURRENT_SCRIPT_FILENAME=$(basename "$0")
CURRENT_SCRIPT_FILEPATH="$CURRENT_SCRIPT_DIR/$CURRENT_SCRIPT_FILENAME"

COLOR_RED='\033[0;31m'          # Red
COLOR_GREEN='\033[0;32m'        # Green
COLOR_YELLOW='\033[0;33m'       # Yellow
COLOR_BLUE='\033[0;34m'         # Blue
COLOR_PURPLE='\033[0;35m'       # Purple
COLOR_OFF='\033[0m'             # Reset

print() {
    printf "%b" "$*"
}

echo() {
    print "$*\n"
}

info() {
    echo "$COLOR_PURPLE==>$COLOR_OFF $COLOR_GREEN$@$COLOR_OFF"
}

success() {
    print "${COLOR_GREEN}[✔] $*\n${COLOR_OFF}"
}

warn() {
    print "${COLOR_YELLOW}🔥  $*\n${COLOR_OFF}"
}

error() {
    print "${COLOR_RED}[✘] $*\n${COLOR_OFF}"
}

die() {
    print "${COLOR_RED}[✘] $*\n${COLOR_OFF}"
    exit 1
}

# check if file exists
# $1 FILEPATH
file_exists() {
    [ -n "$1" ] && [ -e "$1" ]
}

# check if command exists in filesystem
# $1 command name or path
command_exists_in_filesystem() {
    case $1 in
        */*) executable "$1" ;;
        *)   command -v "$1" > /dev/null
    esac
}

executable() {
    file_exists "$1" && [ -x "$1" ]
}

die_if_file_is_not_exist() {
    file_exists "$1" || die "$1 is not exists."
}

die_if_not_executable() {
    executable "$1" || die "$1 is not executable."
}

step() {
    STEP_NUM=$(expr ${STEP_NUM-0} + 1)
    STEP_MESSAGE="$@"
    echo
    echo "${COLOR_PURPLE}=>> STEP ${STEP_NUM} : ${STEP_MESSAGE} ${COLOR_OFF}"
}

run() {
    info "$*"
    eval "$*"
}

list() {
    for item in $@
    do
        echo "$item"
    done
}

list_length() {
    echo $#
}

shiftn() {
    shift "$1" && shift && echo "$@"
}

sed_in_place() {
    if command -v gsed > /dev/null ; then
        unset SED_IN_PLACE_ACTION
        SED_IN_PLACE_ACTION="$1"
        shift
        # contains ' but not contains \'
        if printf "$SED_IN_PLACE_ACTION" | hexdump -v -e '1/1 "%02X" " "' | grep -q 27 && ! printf "$SED_IN_PLACE_ACTION" | hexdump -v -e '1/1 "%02X" ""' | grep -q '5C 27' ; then
            run gsed -i "\"$SED_IN_PLACE_ACTION\"" $@
        else
            run gsed -i "'$SED_IN_PLACE_ACTION'" $@
        fi
    elif command -v sed  > /dev/null ; then
        if sed -i 's/a/b/g' $(mktemp) 2> /dev/null ; then
            unset SED_IN_PLACE_ACTION
            SED_IN_PLACE_ACTION="$1"
            shift
            if printf "$SED_IN_PLACE_ACTION" | hexdump -v -e '1/1 "%02X" " "' | grep -q 27 && ! printf "$SED_IN_PLACE_ACTION" | hexdump -v -e '1/1 "%02X" ""' | grep -q '5C 27' ; then
                run sed -i "\"$SED_IN_PLACE_ACTION\"" $@
            else
                run sed -i "'$SED_IN_PLACE_ACTION'" $@
            fi
        else
            unset SED_IN_PLACE_ACTION
            SED_IN_PLACE_ACTION="$1"
            shift
            if printf "$SED_IN_PLACE_ACTION" | hexdump -v -e '1/1 "%02X" " "' | grep -q 27 && ! printf "$SED_IN_PLACE_ACTION" | hexdump -v -e '1/1 "%02X" ""' | grep -q '5C 27' ; then
                run sed -i '""' "\"$SED_IN_PLACE_ACTION\"" $@
            else
                run sed -i '""' "'$SED_IN_PLACE_ACTION'" $@
            fi
        fi
    else
        die "please install sed utility."
    fi
}

getvalue() {
    if [ $# -eq 0 ] ; then
        cut -d= -f2
    else
        echo "$1" | cut -d= -f2
    fi
}

trim() {
    if [ $# -eq 0 ] ; then
        sed 's/^[[:space:]]*//' | sed 's/[[:space:]]*$//'
    else
        if [ -n "$*" ] ; then
            echo "$*" | sed 's/^[[:space:]]*//' | sed 's/[[:space:]]*$//'
        fi
    fi
}

tolower() {
    if [ $# -eq 0 ] ; then
        if command -v tr > /dev/null ; then
            tr A-Z a-z
        elif command -v  awk > /dev/null ; then
            awk '{print(tolower($0))}'
        elif command -v gawk > /dev/null ; then
            gawk '{print(tolower($0))}'
        else
            die "please install GNU CoreUtils or awk."
        fi
    else
        if [ -z "$*" ] ; then
            return 0
        fi
        if command -v tr > /dev/null ; then
            echo "$*" | tr A-Z a-z
        elif command -v  awk > /dev/null ; then
            echo "$*" | awk '{print(tolower($0))}'
        elif command -v gawk > /dev/null ; then
            echo "$*" | gawk '{print(tolower($0))}'
        elif command -v python > /dev/null ; then
            python  -c 'import sys; print(sys.argv[1].lower());' "$*"
        elif command -v python3 > /dev/null ; then
            python3 -c 'import sys; print(sys.argv[1].lower());' "$*"
        elif command -v python2 > /dev/null ; then
            python2 -c 'import sys; print(sys.argv[1].lower());' "$*"
        elif command -v perl > /dev/null ; then
            perl -e 'print @ARGV[0],"\n"' "$1"
        elif command -v node > /dev/null ; then
            node -e 'console.log(process.argv[2].toLowerCase())' - "$*"
        else
            die "please install GNU CoreUtils or awk."
        fi
    fi
}

nproc() {
    if command nproc --version > /dev/null 2>&1 ; then
        command nproc
    elif test -f /proc/cpuinfo ; then
        grep -c processor /proc/cpuinfo
    elif command -v sysctl > /dev/null ; then
        sysctl -n machdep.cpu.thread_count
    else
        echo 4
    fi
}

own() {
    if command -v awk > /dev/null ; then
        ls -ld "$1" | awk '{print $3":"$4}'
    else
        ls -ld "$1" | cut -d ' ' -f4,6 | tr ' ' ':'
    fi
}

is_integer () {
    case "${1#[+-]}" in
        (*[!0123456789]*) return 1 ;;
        ('')              return 1 ;;
        (*)               return 0 ;;
    esac
}

format_unix_timestamp() {
   date -jf "%s" "$1" "$2" 2> /dev/null ||
   date -d      "@$1" "$2"
}

# }}}
##############################################################################
# {{{ md5sum

#examples:
# printf ss | md5sum
# cat FILE  | md5sum
# md5sum < FILE
md5sum() {
    if [ $# -eq 0 ] ; then
        if echo | command md5sum > /dev/null 2>&1 ; then
             command md5sum | cut -d ' ' -f1
        elif command -v openssl > /dev/null ; then
             openssl md5 | rev | cut -d ' ' -f1 | rev
        else
            return 1
        fi
    else
        if command -v openssl > /dev/null ; then
             openssl md5    "$1" | cut -d ' ' -f2
        elif echo | command md5sum > /dev/null 2>&1 ; then
             command md5sum "$1" | cut -d ' ' -f1
        else
            die "please install openssl or GNU CoreUtils."
        fi
    fi
}

# }}}
##############################################################################
# {{{ sha256sum

#examples:
# printf ss | sha256sum
# cat FILE  | sha256sum
# sha256sum < FILE
sha256sum() {
    if [ $# -eq 0 ] ; then
        if echo | command sha256sum > /dev/null 2>&1 ; then
             command sha256sum | cut -d ' ' -f1
        elif command -v openssl > /dev/null ; then
             openssl sha256 | rev | cut -d ' ' -f1 | rev
        else
            return 1
        fi
    else
        die_if_file_is_not_exist "$1"
        if command -v openssl > /dev/null ; then
             openssl sha256    "$1" | cut -d ' ' -f2
        elif echo | command sha256sum > /dev/null 2>&1 ; then
             command sha256sum "$1" | cut -d ' ' -f1
        else
            die "please install openssl or GNU CoreUtils."
        fi
    fi
}

# $1 FILEPATH
# $2 expect sha256sum
file_exists_and_sha256sum_matched() {
    die_if_file_is_not_exist "$1"
    [ -z "$2" ] && die "please specify expected sha256sum."
    [ "$(sha256sum $1)" = "$2" ]
}

# $1 FILEPATH
# $2 expect sha256sum
die_if_sha256sum_mismatch() {
    file_exists_and_sha256sum_matched "$1" "$2" || die "sha256sum mismatch.\n    expect : $2\n    actual : $(sha256sum $1)"
}

# }}}
##############################################################################
# {{{ map

# $1 map_name
__map_name_ref() {
    die_if_map_name_is_not_specified "$1"
    printf "map_%s\n" "$(printf '%s' "$1" | md5sum)"
}

# $1 map_name
# $2 key
__map_key_ref() {
    die_if_map_name_is_not_specified   "$1"
    die_if_map_key__is_not_specified   "$2"
    printf "%s_%s\n" "$(__map_name_ref "$1")" "$(printf '%s' "$2" | md5sum)"
}

# $1 map_name
# $2 key
map_contains() {
    die_if_map_name_is_not_specified "$1"
    die_if_map_key__is_not_specified "$2"
    for item in $(eval echo \$$(__map_name_ref "$1"))
    do
        [ "$item" = "$2" ] && return 0
    done
}

# $1 map_name
# $2 key
# $3 value
map_set() {
    die_if_map_name_is_not_specified "$1"
    die_if_map_key__is_not_specified "$2"
    if ! map_contains "$1" "$2" ; then
        unset __MAP_NAME_REF__
        __MAP_NAME_REF__="$(__map_name_ref "$1")"
        __MAP_NAME_REF_VALUE__="$(eval echo \$$__MAP_NAME_REF__)"
        eval "$__MAP_NAME_REF__=\"$__MAP_NAME_REF_VALUE__ $2\""
    fi
    eval "$(__map_key_ref "$1" "$2")=$3"
}

# $1 map_name
# $2 key
# output: value
map_get() {
    die_if_map_name_is_not_specified "$1"
    die_if_map_key__is_not_specified "$2"
    eval echo "\$$(__map_key_ref "$1" "$2")"
}

# $1 map_name
# $2 key
# output: value
map_remove() {
    die_if_map_name_is_not_specified "$1"
    die_if_map_key__is_not_specified "$2"

    unset __MAP_NAME_REF__
    __MAP_NAME_REF__="$(__map_name_ref "$1")"

    unset __MAP_KEYS__
    __MAP_KEYS__="$(map_keys "$1")"

    unset $__MAP_NAME_REF__

    for item in $__MAP_KEYS__
    do
        if [ "$item" = "$2" ] ; then
            continue
        else
            eval "$__MAP_NAME_REF__='$(eval echo \$$__MAP_NAME_REF__) $item'"
        fi
    done
    eval "unset $(__map_key_ref "$1" "$2")"
}

# $1 map_name
map_clear() {
    die_if_map_name_is_not_specified "$1"

    unset __MAP_NAME_REF__
    __MAP_NAME_REF__="$(__map_name_ref "$1")"

    for item in $(eval echo "\$$__MAP_NAME_REF__")
    do
        eval "unset $(__map_key_ref "$1" "$item")"
    done
    eval "unset $__MAP_NAME_REF__"
}

# $1 map_name
# output: key list
map_keys() {
    die_if_map_name_is_not_specified "$1"
    eval echo "\$$(__map_name_ref "$1")"
}

# $1 map_name
# output: key list length
map_size() {
    die_if_map_name_is_not_specified "$1"
    list_length $(map_keys "$1")
}

# $1 map_name
die_if_map_name_is_not_specified() {
    [ -z "$1" ] && die "please specify a map name."
}

# $1 key
die_if_map_key__is_not_specified() {
    [ -z "$1" ] && die "please specify a map key."
}

# }}}
##############################################################################
# {{{ fetch

__get_available_fetch_tool() {
    for tool in curl wget http lynx aria2c axel
    do
        if command_exists_in_filesystem "$tool" ; then
            echo "$tool"
            return 0
        fi
    done
    return 1
}

__fetch_via_git() {
    if [ -d "$FETCH_OUTPUT_PATH" ] ; then
        run cd  "$FETCH_OUTPUT_PATH" || return 1
        if      git rev-parse 2> /dev/null ; then
            run git pull &&
            run git submodule update --recursive
        else
            run cd .. &&
            run rm -rf "$FETCH_OUTPUT_NAME" &&
            run git clone --recursive "$FETCH_URL" "$FETCH_OUTPUT_NAME"
        fi
    else
        if [ ! -d "$FETCH_OUTPUT_DIR" ] ; then
            run install -d "$FETCH_OUTPUT_DIR" || return 1
        fi
        run cd "$FETCH_OUTPUT_DIR" || return 1
        run git clone --recursive "$FETCH_URL" "$FETCH_OUTPUT_NAME"
    fi
}

__fetch_archive_via_tools() {
    if [ -f "$FETCH_OUTPUT_PATH" ] ; then
        if [ -n "$FETCH_SHA256" ] ; then
            if file_exists_and_sha256sum_matched "$FETCH_OUTPUT_PATH" "$FETCH_SHA256" ; then
                success "$FETCH_OUTPUT_PATH already have been downloaded."
                return 0
            fi
        fi
        rm -f "$FETCH_OUTPUT_PATH"
    fi

    AVAILABLE_FETCH_TOOL=$(__get_available_fetch_tool)

    if [ -z "$AVAILABLE_FETCH_TOOL" ] ; then
        handle_dependency required command curl || return 1
        if command_exists_in_filesystem curl ; then
            AVAILABLE_FETCH_TOOL=curl
        else
            return 1
        fi
    fi

    case $AVAILABLE_FETCH_TOOL in
        curl)  run curl --fail --retry 20 --retry-delay 30 --location -o "$FETCH_OUTPUT_PATH" "'$FETCH_URL'" ;;
        wget)  run wget --timeout=60 -O "$FETCH_OUTPUT_PATH" "'$FETCH_URL'" ;;
        http)  run http --timeout=60 -o "$FETCH_OUTPUT_PATH" "'$FETCH_URL'" ;;
        lynx)  run lynx -source "$FETCH_URL" > "\"$FETCH_OUTPUT_PATH\"" ;;
        aria2c)run aria2c -d "$FETCH_OUTPUT_DIR" -o "$FETCH_OUTPUT_NAME" "'$FETCH_URL'" ;;
        axel)  run axel -o "$FETCH_OUTPUT_PATH" "'$FETCH_URL'" ;;
    esac

    [ $? -eq 0 ] || return 1

    if [ -n "$FETCH_SHA256" ] ; then
        die_if_sha256sum_mismatch "$FETCH_OUTPUT_PATH" "$FETCH_SHA256"
    fi
}

# fetch <URL> [--sha256=SHA256] <--output-path=PATH>
# fetch <URL> [--sha256=SHA256] <--output-dir=DIR> <--output-name=NAME>
# fetch <URL> [--sha256=SHA256] <--output-dir=DIR> [--output-name=NAME]
# fetch <URL> [--sha256=SHA256] [--output-dir=DIR] <--output-name=NAME>
fetch() {
    unset FETCH_URL
    unset FETCH_SHA256
    unset FETCH_OUTPUT_DIR
    unset FETCH_OUTPUT_NAME
    unset FETCH_OUTPUT_PATH

    if [ -z "$1" ] ; then
        die "please specify a fetch url."
    else
        FETCH_URL="$1"
    fi

    shift

    while [ -n "$1" ]
    do
        case $1 in
            --sha256=*)
                FETCH_SHA256=$(getvalue "$1")
                ;;
            --output-dir=*)
                FETCH_OUTPUT_DIR=$(getvalue "$1")
                if [ -z "$FETCH_OUTPUT_DIR" ] ; then
                    die "--output-dir argument's value must be not empty."
                fi
                ;;
            --output-name=*)
                FETCH_OUTPUT_NAME=$(getvalue "$1")
                if [ -z "$FETCH_OUTPUT_NAME" ] ; then
                    die "--output-name argument's value must be not empty."
                fi
                ;;
            --output-path=*)
                FETCH_OUTPUT_PATH=$(getvalue "$1")
                if [ -z "$FETCH_OUTPUT_PATH" ] ; then
                    die "--output-path argument's value must be not empty."
                fi
        esac
        shift
    done

    if [ -z "$FETCH_OUTPUT_PATH" ] ; then
        [ -z "$FETCH_OUTPUT_DIR" ]  && FETCH_OUTPUT_DIR="$PWD"
        [ -z "$FETCH_OUTPUT_NAME" ] && FETCH_OUTPUT_NAME=$(basename "$FETCH_URL")

        FETCH_OUTPUT_PATH="$FETCH_OUTPUT_DIR/$FETCH_OUTPUT_NAME"
    else
        FETCH_OUTPUT_DIR="$(dirname $FETCH_OUTPUT_PATH)"
        FETCH_OUTPUT_NAME="$(basename $FETCH_OUTPUT_PATH)"
    fi

    if [ ! -d "$FETCH_OUTPUT_DIR" ] ; then
        run install -d "$FETCH_OUTPUT_DIR"
    fi

    case $FETCH_URL in
        *.git) __fetch_via_git ;;
        *)     __fetch_archive_via_tools ;;
    esac
}

# }}}
##############################################################################
# {{{ __upgrade_self

# __upgrade_self <UPGRAGE_URL>
__upgrade_self() {
    set -e

    unset CURRENT_SCRIPT_REALPATH

    # if file exists and is a symbolic link
    if [ -L "$CURRENT_SCRIPT_FILEPATH" ] ; then
        # https://unix.stackexchange.com/questions/136494/whats-the-difference-between-realpath-and-readlink-f#:~:text=GNU%20coreutils%20introduced%20a%20realpath,in%20common%20with%20GNU%20readlink%20.
        if command -v realpath > /dev/null ; then
            CURRENT_SCRIPT_REALPATH=$(realpath $CURRENT_SCRIPT_FILEPATH)
        elif command -v readlink > /dev/null && readlink -f xx > /dev/null 2>&1 ; then
            CURRENT_SCRIPT_REALPATH=$(readlink -f $CURRENT_SCRIPT_FILEPATH)
        else
            handle_dependency required command realpath
            CURRENT_SCRIPT_REALPATH=$(realpath $CURRENT_SCRIPT_FILEPATH)
        fi
    else
        CURRENT_SCRIPT_REALPATH="$CURRENT_SCRIPT_FILEPATH"
    fi

    info "mktemp -d"
    WORKING_DIR=$(mktemp -d)

    run cd $WORKING_DIR

    fetch "$1" --output-path="$WORKING_DIR/self"

    __upgrade_self_exit() {
        if [ -w "$CURRENT_SCRIPT_REALPATH" ] ; then
            run      install -m 555 self "$CURRENT_SCRIPT_REALPATH"
        else
            run sudo install -m 555 self "$CURRENT_SCRIPT_REALPATH"
        fi

        run rm -rf $WORKING_DIR
    }

    trap __upgrade_self_exit EXIT
}

# }}}
##############################################################################
# {{{ __integrate_zsh_completions

# __integrate_zsh_completions <ZSH_COMPLETIONS_SCRIPT_URL>
__integrate_zsh_completions() {
    set -e

    ZSH_COMPLETIONS_SCRIPT_FILENAME="_$CURRENT_SCRIPT_FILENAME"
    ZSH_COMPLETIONS_SCRIPT_OUT_DIR='/usr/local/share/zsh/site-functions'
    ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH="$ZSH_COMPLETIONS_SCRIPT_OUT_DIR/$ZSH_COMPLETIONS_SCRIPT_FILENAME"

    info "mktemp -d"
    WORKING_DIR=$(mktemp -d)

    run cd $WORKING_DIR

    fetch "$1" --output-path="$WORKING_DIR/$ZSH_COMPLETIONS_SCRIPT_FILENAME"

    if [ ! -d "$ZSH_COMPLETIONS_SCRIPT_OUT_DIR" ] ; then
        run install -d "$ZSH_COMPLETIONS_SCRIPT_OUT_DIR"
    fi

    # if file exists and is a symbolic link
    if [ -L "$ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH" ] ; then
        # https://unix.stackexchange.com/questions/136494/whats-the-difference-between-realpath-and-readlink-f#:~:text=GNU%20coreutils%20introduced%20a%20realpath,in%20common%20with%20GNU%20readlink%20.
        if command -v realpath > /dev/null ; then
            ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH=$(realpath $ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH)
        elif command -v readlink > /dev/null && readlink -f xx > /dev/null 2>&1 ; then
            ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH=$(readlink -f $ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH)
        else
            handle_dependency required command realpath
            ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH=$(realpath $ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH)
        fi
    fi

    if [ -w "$ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH" ] ; then
        run      install -m 644 "$ZSH_COMPLETIONS_SCRIPT_FILENAME" "$ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH"
    else
        run sudo install -m 644 "$ZSH_COMPLETIONS_SCRIPT_FILENAME" "$ZSH_COMPLETIONS_SCRIPT_OUT_FILEPATH"
    fi

    run rm -rf $WORKING_DIR

    echo
    warn "you need to run command ${COLOR_GREEN}autoload -U compinit && compinit${COLOR_OFF} ${COLOR_YELLOW}in zsh to make it work.${COLOR_OFF}"
}

# }}}
##############################################################################
# {{{ os

__get_os_name_from_uname_a() {
    if command -v uname > /dev/null ; then
        unset V
        V=$(uname -a | cut -d ' ' -f2)
        case $V in
            opensuse*) return 1 ;;
            *-*) echo "$V" | cut -d- -f1 ;;
            *)   return 1
        esac
    else
        return 1
    fi
}

__get_os_version_from_uname_a() {
    if command -v uname > /dev/null ; then
        unset V
        V=$(uname -a | cut -d ' ' -f2)
        case $V in
            opensuse*) return 1 ;;
            *-*) echo "$V" | cut -d- -f2 ;;
            *)   return 1
        esac
    else
        return 1
    fi
}

# https://www.freedesktop.org/software/systemd/man/os-release.html
__get_os_name_from_etc_os_release() {
    if [ -f /etc/os-release ] ; then
        unset F
        F=$(mktemp) &&
        cat /etc/os-release > "$F" &&
        echo 'echo "$ID"'  >> "$F" &&
        sh "$F"
    else
        return 1
    fi
}

__get_os_version_from_etc_os_release() {
    if [ -f /etc/os-release ] ; then
        unset F
        F=$(mktemp) &&
        cat /etc/os-release > "$F" &&
        echo 'echo "$VERSION_ID"'  >> "$F" && {
            unset V
            V=$(sh "$F")
            if [ -z "$V" ] ; then
                echo 'rolling'
            else
                echo "$V"
            fi
        }
    else
        return 1
    fi
}

# https://refspecs.linuxfoundation.org/LSB_3.0.0/LSB-PDA/LSB-PDA/lsbrelease.html
__get_os_name_from_lsb_release() {
    if command -v lsb_release > /dev/null ; then
        lsb_release --id | cut -f2
    else
        return 1
    fi
}

__get_os_version_from_lsb_release() {
    if command -v lsb_release > /dev/null ; then
        lsb_release --release | cut -f2
    else
        return 1
    fi
}

__get_os_name_from_getprop() {
    if command -v getprop > /dev/null && command -v app_process > /dev/null ; then
        echo 'android'
    else
        return 1
    fi
}

__get_os_version_from_getprop() {
    if command -v getprop > /dev/null ; then
        getprop ro.build.version.release
    else
        return 1
    fi
}

__get_os_arch_from_getprop() {
    if command -v getprop > /dev/null ; then
        getprop ro.product.cpu.abi
    else
        return 1
    fi
}

__get_os_arch_from_uname() {
    if command -v uname > /dev/null ; then
        uname -m 2> /dev/null
    else
        return 1
    fi
}

__get_os_arch_from_arch() {
    if command -v arch > /dev/null ; then
        arch
    else
        return 1
    fi
}

os() {
    if [ $# -eq 0 ] ; then
        printf "current-machine-os-kind : %s\n" "$(os kind)"
        printf "current-machine-os-type : %s\n" "$(os type)"
        printf "current-machine-os-name : %s\n" "$(os name)"
        printf "current-machine-os-vers : %s\n" "$(os vers)"
        printf "current-machine-os-arch : %s\n" "$(os arch)"
        printf "current-machine-os-libc : %s\n" "$(os libc)"
    elif [ $# -eq 1 ] ; then
        case $1 in
            -h|--help)
                cat <<'EOF'
os -h | --help
os -V | --version
os kind
os type
os arch
os libc
os name
os vers
EOF
                ;;
            -V|--version) echo '2021.03.28.23' ;;
            kind)
                case $(uname | tr A-Z a-z) in
                    msys*)    echo "windows" ;;
                    mingw32*) echo "windows" ;;
                    mingw64*) echo "windows" ;;
                    cygwin*)  echo 'windows' ;;
                    *)  uname | tr A-Z a-z
                esac
                ;;

            type)
                case $(uname | tr A-Z a-z) in
                    msys*)    echo "msys"    ;;
                    mingw32*) echo "mingw32" ;;
                    mingw64*) echo "mingw64" ;;
                    cygwin*)  echo 'cygwin'  ;;
                    *)  uname | tr A-Z a-z
                esac
                ;;
            name)
                case $(os kind) in
                    freebsd) echo 'FreeBSD' ;;
                    openbsd) echo 'OpenBSD' ;;
                    netbsd)  echo 'NetBSD'  ;;
                    darwin)  sw_vers -productName ;;
                    linux)
                        __get_os_name_from_uname_a ||
                        __get_os_name_from_etc_os_release ||
                        __get_os_name_from_lsb_release
                        ;;
                    windows)
                        systeminfo | grep 'OS Name:' | cut -d: -f2 | head -n 1 | sed 's/^[[:space:]]*//' ;;
                    *)  uname | tr A-Z a-z
                esac
                ;;
            arch)
                __get_os_arch_from_uname ||
                __get_os_arch_from_arch  ||
                __get_os_arch_from_getprop
                ;;
            libc)
                case $(os kind) in
                    linux)
                        # https://pubs.opengroup.org/onlinepubs/7908799/xcu/getconf.html
                        if command -v getconf > /dev/null ; then
                            if getconf GNU_LIBC_VERSION > /dev/null 2>&1 ; then
                                echo glibc
                                return 0
                            fi
                        fi
                        if command -v ldd > /dev/null ; then
                            if ldd --version 2>&1 | head -n 1 | grep -q GLIBC ; then
                                echo glibc
                                return 0
                            fi
                            if ldd --version 2>&1 | head -n 1 | grep -q musl ; then
                                echo musl
                                return 0
                            fi
                        fi
                        return 1
                esac
                ;;
            vers)
                case $(os kind) in
                    freebsd) freebsd-version ;;
                    openbsd) uname -r ;;
                    netbsd)  uname -r ;;
                    darwin)  sw_vers -productVersion ;;
                    linux)
                        __get_os_version_from_uname_a ||
                        __get_os_version_from_etc_os_release ||
                        __get_os_version_from_lsb_release
                        ;;
                    windows)
                        systeminfo | grep 'OS Version:' | cut -d: -f2 | head -n 1 | sed 's/^[[:space:]]*//' | cut -d ' ' -f1 ;;
                esac
                ;;
            *)  echo "$1: not support item."; return 1
        esac
    else
        echo "os command only support one item."; return 1
    fi
}

# }}}
##############################################################################
# {{{ version

version_of_python_module() {
    unset PIP_COMMAND
    PIP_COMMAND=$(command -v pip3 || command -v pip)
    if [ -z "$PIP_COMMAND" ] ; then
        die "can't found pip command."
    else
        "$PIP_COMMAND" show $1 | grep 'Version:' | cut -d ' ' -f2
    fi
}

# retrive the version of a command from it's name or path
version_of_command() {
    case $(basename "$1") in
        cmake) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 ;;
         make) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 ;;
        gmake) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 ;;
       rustup) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
        cargo) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
           go) "$1"   version | cut -d ' ' -f3 | cut -c3- ;;
         tree) "$1" --version | cut -d ' ' -f2 | cut -c2- ;;
   pkg-config) "$1" --version 2> /dev/null | head -n 1 ;;
       m4|gm4) "$1" --version 2> /dev/null | head -n 1 | awk '{print($NF)}';;
    autopoint) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
     automake|aclocal)
               "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
     autoconf|autoheader|autom4te|autoreconf|autoscan|autoupdate|ifnames)
               "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
      libtool) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
   libtoolize|glibtoolize)
               "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
      objcopy) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f5 ;;
         flex) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
        bison) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
         yacc) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
         nasm) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 ;;
         yasm) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
        patch) "$1" --version 2> /dev/null | head -n 1 | awk '{print($NF)}' ;;
        gperf) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 ;;
        groff) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
     help2man) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 ;;
 sphinx-build) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
         file) "$1" --version 2> /dev/null | head -n 1 | cut -d '-' -f2 ;;
      itstool) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
       protoc) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
        xmlto) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 ;;
      xmllint) ;;
     xsltproc) ;;
     rst2man|rst2man.py|rst2man-3|rst2man-3.6|rst2man-3.7|rst2man-3.8|rst2man-3.9)
               "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 ;;
         gzip) "$1" --version 2>&1 | head -n 1 | awk '{print($NF)}' ;;
         lzip) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
           xz) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
          zip) "$1" --version 2> /dev/null | sed -n '2p' | cut -d ' ' -f4 ;;
        unzip) "$1" -v        2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
        bzip2) "$1" --help 2>&1 | head -n 1 | cut -d ' ' -f8 | cut -d ',' -f1 ;;
          tar)
            VERSION_MSG=$("$1" --version 2> /dev/null | head -n 1)
            case $VERSION_MSG in
                  tar*) echo "$VERSION_MSG" | cut -d ' ' -f4 ;;
               bsdtar*) echo "$VERSION_MSG" | cut -d ' ' -f2 ;;
            esac
            ;;
          git) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 ;;
         curl) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
     awk|gawk) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f3 | tr , ' ' ;;
     sed|gsed) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
         cpan) ;;
         find) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
         diff) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
         grep) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 | cut -d '-' -f1 ;;
         ruby) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
         perl) "$1" -v | sed -n '2p' | sed 's/.*v\([0-9]\.[0-9][0-9]\.[0-9]\).*/\1/' ;;
    python|python2|python3)
               "$1" --version 2>&1 | head -n 1 | cut -d ' ' -f2 ;;
         pip)  "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
         pip3) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
         node) "$1" --version 2> /dev/null | head -n 1 | cut -d 'v' -f2 ;;
          zsh) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f2 ;;
         bash) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 | cut -d '(' -f1 ;;
       base64) "$1" --version 2> /dev/null | head -n 1 | cut -d ' ' -f4 ;;
            *) "$1" --version 2> /dev/null | head -n 1
    esac
}

# retrive the major part of the version of the given command
# Note: the version of the given command must have form: major.minor.patch
version_major_of_command() {
    version_of_command "$1" | cut -d. -f1
}

# retrive the minor part of the version of the given command
# Note: the version of the given command must have form: major.minor.patch
version_minor_of_command() {
    version_of_command "$1" | cut -d. -f2
}

# retrive the major part of the given version
# Note: the given version must have form: major.minor.patch
version_major_of_version() {
    echo "$1" | cut -d. -f1
}

# retrive the minor part of the given version
# Note: the given version must have form: major.minor.patch
version_minor_of_version() {
    echo "$1" | cut -d. -f2
}

version_sort() {
    # https://pubs.opengroup.org/onlinepubs/9699919799/utilities/sort.html
    # https://man.netbsd.org/NetBSD-8.1/i386/sort.1
    #
    # sort: unrecognized option: V
    # BusyBox v1.29.3 (2019-01-24 07:45:07 UTC) multi-call binary.
    # Usage: sort [-nrugMcszbdfiokt] [-o FILE] [-k start[.offset][opts][,end[.offset][opts]] [-t CHAR] [FILE]...
    if  echo | (sort -V > /dev/null 2>&1) ; then
        echo "$@" | tr ' ' '\n' | sort -V
    else
        echo "$@" | tr ' ' '\n' | sort -t. -n -k1,1 -k2,2 -k3,3 -k4,4
    fi
}

# check if match the condition
#
# condition:
# eq  equal
# ne  not equal
# gt  greater than
# lt  less than
# ge  greater than or equal
# le  less than or equal
#
# examples:
# version_match 1.15.3 eq 1.16.0
# version_match 1.15.3 lt 1.16.0
# version_match 1.15.3 gt 1.16.0
# version_match 1.15.3 le 1.16.0
# version_match 1.15.3 ge 1.16.0
version_match() {
    case $2 in
        eq)  [ "$1"  = "$3" ] ;;
        ne)  [ "$1" != "$3" ] ;;
        le)
            [ "$1" = "$3" ] && return 0
            [ "$1" = "$(version_sort "$1" "$3" | head -n 1)" ]
            ;;
        ge)
            [ "$1" = "$3" ] && return 0
            [ "$1" = "$(version_sort "$1" "$3" | tail -n 1)" ]
            ;;
        lt)
            [ "$1" = "$3" ] && return 1
            [ "$1" = "$(version_sort "$1" "$3" | head -n 1)" ]
            ;;
        gt)
            [ "$1" = "$3" ] && return 1
            [ "$1" = "$(version_sort "$1" "$3" | tail -n 1)" ]
            ;;
        *)  die "version_compare: $2: not supported operator."
    esac
}

# check if the version of give installed command match the condition
#
# condition:
# eq  equal
# ne  not equal
# gt  greater than
# lt  less than
# ge  greater than or equal
# le  less than or equal
#
# examples:
# command_exists_in_filesystem_and_version_matched automake eq 1.16.0
# command_exists_in_filesystem_and_version_matched automake lt 1.16.0
# command_exists_in_filesystem_and_version_matched automake gt 1.16.0
# command_exists_in_filesystem_and_version_matched automake le 1.16.0
# command_exists_in_filesystem_and_version_matched automake ge 1.16.0
# command_exists_in_filesystem_and_version_matched automake
command_exists_in_filesystem_and_version_matched() {
    if command_exists_in_filesystem "$1" ; then
        if [ "$NATIVE_OS_TYPE" = 'cygwin' ] ; then
            case $(command -v "$1") in
                /cygdrive/*) return 1
            esac
        fi
        if [ $# -eq 3 ] ; then
            version_match "$(version_of_command "$1")" "$2" "$3"
        fi
    else
        return 1
    fi
}

# }}}
##############################################################################
# {{{ package_manager

# check if the version of give package match the condition
#
# condition:
# eq  equal
# ne  not equal
# gt  greater than
# lt  less than
# ge  greater than or equal
# le  less than or equal
#
# examples:
# package_exists_and_version_matched apt automake eq 1.16.0
# package_exists_and_version_matched apt automake lt 1.16.0
# package_exists_and_version_matched apt automake gt 1.16.0
# package_exists_and_version_matched apt automake le 1.16.0
# package_exists_and_version_matched apt automake ge 1.16.0
# package_exists_and_version_matched apt automake
package_exists_in_repo_and_version_matched() {
    if package_exists_in_repo "$1" "$2" ; then
        if [ $# -eq 4 ] ; then
            case $1 in
                apt|yum|dnf) version_match "$(version_of_package "$1" "$2")" "$3" "$4" ;;
                *)       return 0 ;;
            esac
        fi
    else
        return 1
    fi
}

# check if the give package is in the give repo
#
# examples:
# package_exists_in_repo apt automake
package_exists_in_repo() {
    case $1 in
        apt) apt show "$2" > /dev/null 2>&1 ;;
        yum) yum info "$2" > /dev/null 2>&1 ;;
        dnf) dnf info "$2" > /dev/null 2>&1 ;;
    esac
}

# get the version of the give package in the give repo
#
# examples:
# version_of_package apt automake
version_of_package() {
    case $1 in
        apt) apt show "$2" 2> /dev/null | grep 'Version: '      | head -n 1 | cut -d ' ' -f2 | cut -d- -f1 ;;
        yum) yum info "$2" 2> /dev/null | grep 'Version     :'  | head -n 1 | cut -d : -f2 | sed 's/^[[:space:]]//' ;;
        dnf) dnf info "$2" 2> /dev/null | grep 'Version      :' | head -n 1 | cut -d : -f2 | sed 's/^[[:space:]]//' ;;
    esac
}

# }}}
##############################################################################
# {{{ get_XX_package_name_by_command_name

# https://cygwin.com/packages/package_list.html
get_choco_package_name_by_command_name() {
    case $1 in
      cc|gcc) echo 'gcc-g++' ;;
       gmake) echo 'make' ;;
         gm4) echo 'm4'    ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
    sphinx-build) echo 'python38-sphinx' ;;
    glibtool|libtoolize|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
        *)      echo "$1"
    esac
}

get_pkg_add_package_name_by_command_name() {
    case $1 in
          cc) echo 'gcc'   ;;
         gm4) echo 'm4'    ;;
        make) echo 'gmake' ;;
        perl) echo 'perl5' ;;
       gperf) echo 'gperf' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
    realpath) echo 'coreutils';;
     objcopy) echo 'binutils' ;;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
    pip3|pip) echo 'py3-pip' ;;
    libtool|libtoolize|glibtool|glibtoolize)
              echo "libtool" ;;
    autoreconf|autoconf|autoreconf-2.69|autoconf-2.69)
              echo "autoconf-2.69p3" ;;
    automake|autoheader|automake-1.16|autoheader-1.16)
            if [ "$NATIVE_OS_VERS" = '6.9' ] ; then
                echo "automake-1.16.3"
            else
                echo "automake-1.16.2"
            fi
            ;;
    autopoint) echo "gettext" ;;
    pkg-config) echo "pkgconf" ;;
        *) echo "$1"
    esac
}

get_pkgin_package_name_by_command_name() {
    case $1 in
          cc) echo 'gcc'   ;;
         gm4) echo 'm4'    ;;
        make) echo 'gmake' ;;
        perl) echo 'perl5' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
    realpath) echo 'coreutils';;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
    pip3|pip) echo 'py38-pip'  ;;
    python3)  echo 'python38' ;;
    glibtool|libtoolize|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
    pkg-config) echo "pkgconf"  ;;
        *)      echo "$1"
    esac
}

get_pkg_package_name_by_command_name() {
    case $1 in
          cc) echo 'gcc'   ;;
         gm4) echo 'm4'    ;;
        make) echo 'gmake' ;;
        perl) echo 'perl5' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
    pip3|pip) echo 'py38-pip' ;;
    libtool|libtoolize|glibtool|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
    pkg-config) echo "pkgconf"  ;;
        *)      echo "$1"
    esac
}

get_emerge_package_name_by_command_name() {
    case $1 in
          cc) echo 'gcc' ;;
         gm4) echo 'm4'    ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
      xz)     echo 'xz-utils' ;;
      rst2man|rst2html)
              echo "docutils" ;;
    sphinx-build)
              echo "sphinx" ;;
    pip3|pip) echo "dev-python/pip" ;;
    libtool|libtoolize|glibtool|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
    pkg-config) echo "pkgconf"  ;;
        *)      echo "$1"
    esac
}

__get_pacman_package_name_by_command_name() {
    case $1 in
          cc) echo 'gcc'      ;;
         gm4) echo 'm4'       ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
      rst2man|rst2html)
              echo "python-docutils" ;;
    sphinx-build)
              echo "python-sphinx" ;;
    pip3|pip) echo "python-pip" ;;
    libtool|libtoolize|glibtool|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
    pkg-config) echo "pkgconf"  ;;
        *)      echo "$1"
    esac
}

__mingw_w64_i686() {
    if pacman -S -i "mingw-w64-i686-$1" > /dev/null 2>&1 ; then
        echo "mingw-w64-i686-$1"
    else
        echo "$1"
    fi
}

__mingw_w64_x86_64() {
    if pacman -S -i "mingw-w64-x86_64-$1" > /dev/null 2>&1 ; then
        echo "mingw-w64-x86_64-$1"
    else
        echo "$1"
    fi
}

get_pacman_package_name_by_command_name() {
    if [ "$1" = 'make' ] || [ "$1" = 'gmake' ] ; then
        echo make
    fi
    case $NATIVE_OS_TYPE in
        mingw32) __mingw_w64_i686   $(__get_pacman_package_name_by_command_name "$1") ;;
        mingw64) __mingw_w64_x86_64 $(__get_pacman_package_name_by_command_name "$1") ;;
        *) __get_pacman_package_name_by_command_name "$1"
    esac
}

get_xbps_package_name_by_command_name() {
    case $1 in
      cc|gcc) echo 'gcc' ;;
         gm4) echo 'm4'    ;;
       gperf) echo 'gperf' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
    realpath) echo 'coreutils';;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
      rst2man|rst2html)
              echo "python3-docutils" ;;
    pip|pip3) echo "python3-pip" ;;
    glibtool|libtoolize|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
    pkg-config) echo "pkgconf"  ;;
        *)      echo "$1"
    esac
}

get_apk_package_name_by_command_name() {
    case $1 in
      cc|gcc) echo 'gcc libc-dev' ;;
         gm4) echo 'm4'    ;;
       gperf) echo 'gperf' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
    realpath) echo 'coreutils';;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
      rst2man|rst2html)
              echo "py3-docutils" ;;
    sphinx-build)
              echo "sphinx"  ;;
    pip3|pip) echo 'py3-pip' ;;
    glibtool|libtoolize|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
    pkg-config) echo "pkgconf"  ;;
        *) echo "$1"
    esac
}

get_zypper_package_name_by_command_name() {
    case $1 in
      cc|gcc) echo 'gcc' ;;
         gm4) echo 'm4'    ;;
       gperf) echo 'gperf' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
    realpath) echo 'coreutils';;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
      rst2man|rst2html)
              echo "python3-docutils" ;;
    sphinx-build)
              echo "python3-Sphinx" ;;
    pip3|pip) echo "python3-pip" ;;
    glibtool|libtoolize|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
    autopoint)  echo "gettext"  ;;
        *)      echo "$1"
    esac
}

get_dnf_package_name_by_command_name() {
    case $1 in
      cc|gcc) echo 'gcc' ;;
         gm4) echo 'm4'    ;;
       gperf) echo 'gperf' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
    realpath) echo 'coreutils';;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
      rst2man|rst2html)
              echo "python3-docutils" ;;
    sphinx-build)
              echo "python3-sphinx" ;;
    glibtool|libtoolize|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
        *)      echo "$1"
    esac
}

get_yum_package_name_by_command_name() {
    case $1 in
      cc|gcc) echo 'gcc' ;;
         gm4) echo 'm4'    ;;
       gperf) echo 'gperf' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
    realpath) echo 'coreutils';;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
    sphinx-build) echo "python-sphinx" ;;
    glibtool|libtoolize|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
        *)      echo "$1"
    esac
}

get_apt_get_package_name_by_command_name() {
    get_apt_package_name_by_command_name $@
}

get_apt_package_name_by_command_name() {
    case $1 in
      cc|gcc) echo 'gcc' ;;
         gm4) echo 'm4'    ;;
       gperf) echo 'gperf' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
        xz)   echo 'xz-utils' ;;
     objcopy) echo 'binutils' ;;
    realpath) echo 'coreutils';;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
      rst2man|rst2html)
              echo "python3-docutils" ;;
    sphinx-build)
              echo "python3-sphinx" ;;
    pip3|pip) echo "python3-pip" ;;
    glibtool|libtoolize|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
        *)      echo "$1"
    esac
}

get_brew_package_name_by_command_name() {
    case $1 in
      cc|gcc) echo 'gcc' ;;
         gm4) echo 'm4'    ;;
       gperf) echo 'gperf' ;;
        gsed) echo 'gnu-sed'  ;;
        find) echo 'findutils';;
        diff) echo 'diffutils';;
     objcopy) echo 'binutils' ;;
    realpath) echo 'coreutils';;
      protoc) echo 'protobuf' ;;
      ps2pdf) echo "ghostscript" ;;
     rst2man.py|rst2html.py)
              echo "docutils" ;;
    grpc_cpp_plugin)
                echo "grpc" ;;
    glibtool|libtoolize|glibtoolize)
                echo "libtool"  ;;
    autoreconf) echo "autoconf" ;;
    autoheader) echo "automake" ;;
    autopoint)  echo "gettext"  ;;
    python|python3)
                echo 'python@3.9' ;;
        *)      echo "$1"
    esac
}

get_pip3_package_name_by_command_name() {
    get_pip_package_name_by_command_name $@
}

get_pip_package_name_by_command_name() {
    case $1 in
        sphinx-build) echo "sphinx"   ;;
        rst2man.py)   echo "docutils" ;;
    esac
}

# }}}
##############################################################################
# {{{ __get_available_package_manager_list

__add_available_package_manager() {
    if command_exists_in_filesystem "$1" ; then
        if [ -z "$AVAILABLE_PACKAGE_MANAGER_LIST" ] ; then
            AVAILABLE_PACKAGE_MANAGER_LIST="$2"
        else
            AVAILABLE_PACKAGE_MANAGER_LIST="$AVAILABLE_PACKAGE_MANAGER_LIST $2"
        fi
    fi
}

__get_available_package_manager_list() {
    if [ -z "$AVAILABLE_PACKAGE_MANAGER_LIST" ] ; then
        for pm in brew apt dnf zypper apk xbps-install emerge pacman choco pkg pkgin pkg_add
        do
            case $pm in
                apt)
                    if command_exists_in_filesystem apt && apt show apt > /dev/null 2>&1 ; then
                        __add_available_package_manager apt apt
                    else
                        __add_available_package_manager apt-get apt-get
                    fi
                    ;;
                dnf)
                    if command_exists_in_filesystem dnf ; then
                        __add_available_package_manager dnf dnf
                    else
                        __add_available_package_manager yum yum
                    fi
                    ;;
                xbps-install)
                    __add_available_package_manager xbps-install xbps
                    ;;
                *)  __add_available_package_manager "$pm" "$pm"
            esac
        done
    fi
    echo "$AVAILABLE_PACKAGE_MANAGER_LIST"
}

# }}}
##############################################################################
# {{{ __install_required

# $1 package manager name
# $2 package name
# examples:
# __install_package_via_package_manager apt make ge 3.80
# __install_package_via_package_manager apt make
__install_package_via_package_manager() {
    [ -z "$2" ] && return 1

    package_exists_in_repo_and_version_matched $@ || return 1

    case $1 in
        pip3)
            case $NATIVE_OS_KIND in
                *bsd|linux) run pip3 install --user -U "$2" ;;
                *)          run pip3 install        -U "$2"
            esac
            ;;
        pip)
            case $NATIVE_OS_KIND in
                *bsd|linux) run pip  install --user -U "$2" ;;
                *)          run pip  install        -U "$2"
            esac
            ;;
        brew)
            if run brew install "$2" ; then
                if [ "$(brew info --json=v2 "$2" | grep keg_only | sed 's/ //g' | cut -d: -f2 | sed 's/,//')" = 'true' ] ; then
                    if [ -d "$(brew --prefix "$2")/bin" ] ; then
                        export PATH="$(brew --prefix "$2")/bin:$PATH"
                    fi
                fi
            else
                return 1
            fi
            ;;
        pkg)     run $sudo pkg install -y "$2" ;;
        pkgin)   run $sudo pkgin -y install "$2" ;;
        pkg_add) run $sudo pkg_add "$2" ;;
        apt)     run $sudo apt     -y install "$2" ;;
        apt-get) run $sudo apt-get -y install "$2" ;;
        dnf)     run $sudo dnf -y install "$2" ;;
        yum)     run $sudo yum -y install "$2" ;;
        zypper)  run $sudo zypper install -y "$2" ;;
        apk)     run $sudo apk add "$2" ;;
        xbps)    run $sudo xbps-install -Sy "$2" ;;
        emerge)  run $sudo emerge "$2" ;;
        pacman)  run $sudo pacman -Syy --noconfirm && run $sudo pacman -S --noconfirm "$2" ;;
        choco)   run choco install -y --source cygwin "$2" ;;
    esac
    echo
}

# $1 package manager name
# $2 command name
# examples:
# __install_command_via_package_manager apt make ge 3.80
# __install_command_via_package_manager apt make
__install_command_via_package_manager() {
    unset __PACKAGE_NAME__
    __PACKAGE_NAME__="$(eval get_$(echo "$1" | tr - _)_package_name_by_command_name $2)"

    [ -z "$__PACKAGE_NAME__" ] && return 1

    package_exists_in_repo_and_version_matched "$1" "$__PACKAGE_NAME__" $3 $4 || return 1

    print "🔥  ${COLOR_YELLOW}required command${COLOR_OFF} ${COLOR_GREEN}$(shiftn 1 $@)${COLOR_OFF}${COLOR_YELLOW}, but${COLOR_OFF} ${COLOR_GREEN}$2${COLOR_OFF} ${COLOR_YELLOW}command not found, try to install it via${COLOR_OFF} ${COLOR_GREEN}$1${COLOR_OFF}\n"

    __install_package_via_package_manager "$1" "$__PACKAGE_NAME__" $3 $4
}

# examples:
# __install_command_via_available_package_manager python3 ge 3.5
# __install_command_via_available_package_manager make
__install_command_via_available_package_manager() {
    command_exists_in_filesystem_and_version_matched $@ && return 0
 
    if [ -z "$AVAILABLE_PACKAGE_MANAGER_LIST" ] ; then
        AVAILABLE_PACKAGE_MANAGER_LIST=$(__get_available_package_manager_list)
        if [ -z "$AVAILABLE_PACKAGE_MANAGER_LIST" ] ; then
            warn "no package manager found."
            return 1
        else
            echo "    Found $(list_length $AVAILABLE_PACKAGE_MANAGER_LIST) package manager : ${COLOR_GREEN}$AVAILABLE_PACKAGE_MANAGER_LIST${COLOR_OFF}"
        fi
    fi
    for pm in $AVAILABLE_PACKAGE_MANAGER_LIST
    do
        __install_command_via_package_manager "$pm" $@ && return 0
    done
    return 1
}

handle_dependency_from_url() {
    case $1 in
        *.zip)
            handle_dependency required command unzip
            ;;
        *.tar.xz)
            handle_dependency required command tar
            handle_dependency required command xz
            ;;
        *.tar.gz)
            handle_dependency required command tar
            handle_dependency required command gzip
            ;;
        *.tar.lz)
            handle_dependency required command tar
            handle_dependency required command lzip
            ;;
        *.tar.bz2)
            handle_dependency required command tar
            handle_dependency required command bzip2
            ;;
        *.tgz)
            handle_dependency required command tar
            handle_dependency required command gzip
            ;;
        *.txz)
            handle_dependency required command tar
            handle_dependency required command xz
    esac
}

get_suffix_from_filename() {
    case $1 in
        *.zip)     echo '.zip' ;;
        *.tar.xz)  echo '.tar.xz' ;;
        *.tar.gz)  echo '.tar.gz' ;;
        *.tar.lz)  echo '.tar.lz' ;;
        *.tar.bz2) echo '.tar.bz2' ;;
        *.tgz)     echo '.tgz' ;;
        *.txz)     echo '.txz' ;;
    esac
}

# examples:
# pkg-config ge 0.18
# python3    ge 3.5
# make
__install_command_via_fetch_prebuild_binary() {
    unset PREBUILD_BINARY_FETCH_URL
    PREBUILD_BINARY_FETCH_URL=$(__get_prebuild_binary_fetch_url_by_command_name "$1")
    if [ -z "$PREBUILD_BINARY_FETCH_URL" ] ; then
        return 1
    fi

    handle_dependency_from_url "$PREBUILD_BINARY_FETCH_URL"

    print "🔥  ${COLOR_YELLOW}required command${COLOR_OFF} ${COLOR_GREEN}$@${COLOR_OFF}${COLOR_YELLOW}, but${COLOR_OFF} ${COLOR_GREEN}$1${COLOR_OFF} ${COLOR_YELLOW}command not found, try to install it via${COLOR_OFF} ${COLOR_GREEN}fetch prebuild binary${COLOR_OFF}\n"

    unset PREBUILD_BINARY_FETCH_URL
    PREBUILD_BINARY_FETCH_URL=$(__get_prebuild_binary_fetch_url_by_command_name "$1")

    unset PREBUILD_BINARY_FILENAME_PREFIX
    unset PREBUILD_BINARY_FILENAME_SUFFIX
    unset PREBUILD_BINARY_FILENAME
    unset PREBUILD_BINARY_FILEPATH

    PREBUILD_BINARY_FILENAME=$(basename "$PREBUILD_BINARY_FETCH_URL")
    PREBUILD_BINARY_FILENAME_SUFFIX=$(get_suffix_from_filename "$PREBUILD_BINARY_FILENAME")
    PREBUILD_BINARY_FILENAME_PREFIX=$(basename "$PREBUILD_BINARY_FILENAME" "$PREBUILD_BINARY_FILENAME_SUFFIX")

    if [ -z "$PREBUILD_BINARY_INSTALL_PREFIX_DIR" ] ; then
        PREBUILD_BINARY_INSTALL_PREFIX_DIR='/opt'
    fi

    unset PREBUILD_BINARY_INSTALL_DIR
    PREBUILD_BINARY_INSTALL_DIR="$PREBUILD_BINARY_INSTALL_PREFIX_DIR/$PREBUILD_BINARY_FILENAME_PREFIX"

    PREBUILD_BINARY_FILEPATH="$PREBUILD_BINARY_INSTALL_DIR/$PREBUILD_BINARY_FILENAME"

    if [ -d "$PREBUILD_BINARY_INSTALL_PREFIX_DIR" ] ; then
        if [ -d "$PREBUILD_BINARY_INSTALL_DIR" ] ; then
            if [ -r "$PREBUILD_BINARY_INSTALL_DIR" ] && [ -w "$PREBUILD_BINARY_INSTALL_DIR" ] && [ -x "$PREBUILD_BINARY_INSTALL_DIR" ] ; then
                rm -rf "$PREBUILD_BINARY_INSTALL_DIR" || return 1
                install -o $(whoami) -d "$PREBUILD_BINARY_INSTALL_DIR" || return 1
            else
                sudo rm -rf "$PREBUILD_BINARY_INSTALL_DIR" || return 1
                sudo install -o $(whoami) -d "$PREBUILD_BINARY_INSTALL_DIR" || return 1
            fi
        else
            if [ -r "$PREBUILD_BINARY_INSTALL_PREFIX_DIR" ] && [ -w "$PREBUILD_BINARY_INSTALL_PREFIX_DIR" ] && [ -x "$PREBUILD_BINARY_INSTALL_PREFIX_DIR" ] ; then
                     install -o $(whoami) -d "$PREBUILD_BINARY_INSTALL_DIR" || return 1
            else
                sudo install -o $(whoami) -d "$PREBUILD_BINARY_INSTALL_DIR" || return 1
            fi
        fi
    else
        if [ -r / ] && [ -w / ] && [ -x / ] ; then
                 install -o $(whoami) -d "$PREBUILD_BINARY_INSTALL_DIR" || return 1
        else
            sudo install -o $(whoami) -d "$PREBUILD_BINARY_INSTALL_DIR" || return 1
        fi
    fi

    fetch "$PREBUILD_BINARY_FETCH_URL" --output-dir="$PREBUILD_BINARY_INSTALL_DIR" --output-name="$PREBUILD_BINARY_FILENAME" || return 1

    case $PREBUILD_BINARY_FILENAME_SUFFIX in
        .zip)
            run unzip  "$PREBUILD_BINARY_FILEPATH" -d "$PREBUILD_BINARY_INSTALL_DIR" || return 1 ;;
        .tar.xz|.tar.gz|.tar.lz|.tar.bz2|.tgz|.txz|.tlz)
            run tar xf "$PREBUILD_BINARY_FILEPATH" -C "$PREBUILD_BINARY_INSTALL_DIR" --strip-components 1 || return 1 ;;
    esac

    if [ -d "$PREBUILD_BINARY_INSTALL_DIR/bin" ] ; then
        export PATH="$PREBUILD_BINARY_INSTALL_DIR/bin:$PATH"
    fi

    case $1 in
        autoconf)
            for xx in $(grep '/root/.zpkg/install.d/autoconf' -rl "$PREBUILD_BINARY_INSTALL_DIR")
            do
                sed_in_place "s|/root/.zpkg/install.d/autoconf|$PREBUILD_BINARY_INSTALL_DIR|" "$xx"
            done
            ;;
    esac
}

# examples:
# __get_prebuild_binary_fetch_url_by_command_name python3 ge 3.5
# __get_prebuild_binary_fetch_url_by_command_name cmake
__get_prebuild_binary_fetch_url_by_command_name() {
    # https://github.com/leleliu008/python-prebuild
    # https://cmake.org/download
{
    cat <<EOF
python    |glibc|linux |x86_64|https://github.com/leleliu008/python-prebuild/releases/download/3.9.5/python-3.9.5-x86_64-linux-glibc.tar.xz
python3   |glibc|linux |x86_64|https://github.com/leleliu008/python-prebuild/releases/download/3.9.5/python-3.9.5-x86_64-linux-glibc.tar.xz
git       |glibc|linux |x86_64|https://github.com/leleliu008/git-prebuild/releases/download/2.30.2/git-2.30.2-x86_64-linux-glibc.tar.xz
autoconf  |glibc|linux |x86_64|https://github.com/leleliu008/autoconf-prebuild/releases/download/2.69/autoconf-2.69-glibc-linux-x86_64.tar.gz
autoreconf|glibc|linux |x86_64|https://github.com/leleliu008/autoconf-prebuild/releases/download/2.69/autoconf-2.69-glibc-linux-x86_64.tar.gz
cmake     |glibc|linux |x86_64|https://github.com/Kitware/CMake/releases/download/v3.20.2/cmake-3.20.2-linux-x86_64.tar.gz
cmake     |     |darwin|      |https://github.com/Kitware/CMake/releases/download/v3.20.2/cmake-3.20.2-macos-universal.tar.gz
EOF
} | while read LINE
    do
        LINE=$(echo "$LINE" | sed 's/[[:space:]]//g')

        unset __PB_COMMAND__
        unset __PB_OS_LIBC__
        unset __PB_OS_KIND__
        unset __PB_OS_ARCH__
        unset __PB_URL__

        __PB_COMMAND__=$(echo "$LINE" | cut -d '|' -f1)

        if [ "$__PB_COMMAND__" != "$1" ] ; then
            continue
        fi

        __PB_OS_LIBC__=$(echo "$LINE" | cut -d '|' -f2)

        if [ -n "$__PB_OS_LIBC__" ] && [ "$__PB_OS_LIBC__" != "$NATIVE_OS_LIBC" ] ; then
            continue
        fi

        __PB_OS_KIND__=$(echo "$LINE" | cut -d '|' -f3)

        if [ "$__PB_OS_KIND__" != "$NATIVE_OS_KIND" ] ; then
            continue
        fi

        __PB_OS_ARCH__=$(echo "$LINE" | cut -d '|' -f4)

        if [ -n "$__PB_OS_ARCH__" ] && [ "$__PB_OS_ARCH__" != "$NATIVE_OS_ARCH" ] ; then
            continue
        fi

        __PB_URL__=$(echo "$LINE" | cut -d '|' -f5)

        echo "$__PB_URL__"
    done
}

__install_command_via_run_install_script() {
    case $1 in
        rustup)
            # https://www.rust-lang.org/tools/install

            unset __RUSTUP_INSTALL_SCRIPT_RUN_SHELL__
            __RUSTUP_INSTALL_SCRIPT_RUN_SHELL__=$(command -v bash || command -v zsh || command -v dash)
            if [ -z "$__RUSTUP_INSTALL_SCRIPT_RUN_SHELL__" ] ; then
                handle_dependency required command bash || return 1
            fi
            __RUSTUP_INSTALL_SCRIPT_RUN_SHELL__=$(command -v bash)
            if [ -z "$__RUSTUP_INSTALL_SCRIPT_RUN_SHELL__" ] ; then
                warn "install bash failed."
                return 1
            fi

            print "🔥  ${COLOR_YELLOW}required command${COLOR_OFF} ${COLOR_GREEN}$1${COLOR_OFF}${COLOR_YELLOW}, but${COLOR_OFF} ${COLOR_GREEN}$1${COLOR_OFF} ${COLOR_YELLOW}command not found, try to install it via running install script.${COLOR_OFF}\n"

            unset __RUSTUP_INSTALL_SCRIPT_DIR__
            __RUSTUP_INSTALL_SCRIPT_DIR__=$(mktemp -d) || return 1
            fetch 'https://sh.rustup.rs' --output-dir="$__RUSTUP_INSTALL_SCRIPT_DIR__" --output-name='rustup-init' || return 1

            run "$__RUSTUP_INSTALL_SCRIPT_RUN_SHELL__ $__RUSTUP_INSTALL_SCRIPT_DIR__/rustup-init -y" || return 1

            export CARGO_HOME=$HOME/.cargo
            export PATH="$CARGO_HOME/bin:$PATH"
            ;;
        *)  return 1
    esac
}

__install_command_via_pip() {
    [ -z "$(get_pip3_package_name_by_command_name "$1")" ] && return 1

    handle_dependency required command pip3:pip

    (
        unset __PIP_COMMAND__
        __PIP_COMMAND__="$(command -v pip3.8)"

        if [ -n "$__PIP_COMMAND__" ] && [ -d '/usr/local/bin' ] ; then
            $sudo ln -sf "$__PIP_COMMAND__" /usr/local/bin/pip3
        fi
    )

    if   command_exists_in_filesystem pip3 ; then
        __install_package_via_package_manager pip3 "$(get_pip3_package_name_by_command_name "$1")"
    elif command_exists_in_filesystem pip ; then
        __install_package_via_package_manager pip  "$(get_pip3_package_name_by_command_name "$1")"
    else
        return 1
    fi
}

# examples:
# __install_command python3 ge 3.5
# __install_command make
__install_command() {
    command_exists_in_filesystem_and_version_matched $@ && return 0

    __install_command_via_run_install_script $@         && return 0
    __install_command_via_pip $@                        && return 0
    __install_command_via_available_package_manager $@  && return 0
    __install_command_via_fetch_prebuild_binary $@      && return 0
    return 1
}

# examples:
# handle_dependency required command   pkg-config ge 0.18
# handle_dependency required command   python     ge 3.5
# handle_dependency required module.py libxml2    ge 2.19
#
# handle_dependency optional command   pkg-config ge 0.18
# handle_dependency optional command   python     ge 3.5
# handle_dependency optional module.py libxml2    ge 2.19
handle_dependency() {
    [ "$1" = 'required' ] || return 0

    shift

    case $1 in
        command)
            shift
            case $1 in
                *:*)
                    for command in $(echo "$1" | tr ':' ' ')
                    do
                        if command_exists_in_filesystem_and_version_matched "$command" $2 $3 ; then
                            map_set MAP_REQUIRED_DEPENDENCIES "$1" "$command"
                            return 0
                        fi
                    done
                    for command in $(echo "$1" | tr ':' ' ')
                    do
                        if __install_command "$command" $2 $3 ; then
                            map_set MAP_REQUIRED_DEPENDENCIES "$1" "$command"
                            return 0
                        fi
                    done
                    return 1
                    ;;
                *)  __install_command $@
            esac
            ;;
        module.py)
            shift
            python_module install "$1"
            ;;
        module.pl)
            shift
            perl_module install "$1"
            ;;
        *) die "$1 not support."
    esac
}

__handle_required_dependencies() {
    step "handle required dependencies"

    for dependency in $REQUIRED_DEPENDENCY_LIST
    do
        handle_dependency $(__decode_dependency "$dependency") || return 1
    done
}

# }}}
##############################################################################
# {{{ __printf_dependencies

# examples:
# __printf_dependency required command   pkg-config ge 0.18
# __printf_dependency required command   python     ge 3.5
# __printf_dependency required module.py libxml2    ge 2.19
#
# __printf_dependency optional command   pkg-config ge 0.18
# __printf_dependency optional command   python     ge 3.5
# __printf_dependency optional module.py libxml2    ge 2.19
__printf_dependency() {
    printf "%-10s %-15s %-2s %-10s %-10s %s\n" "$1" "$2" "$3" "$4" "$5" "$6"
}

# examples:
# printf_dependency required command   pkg-config ge 0.18
# printf_dependency required command   python     ge 3.5
# printf_dependency required module.py libxml2    ge 2.19
#
# printf_dependency optional command   pkg-config ge 0.18
# printf_dependency optional command   python     ge 3.5
# printf_dependency optional module.py libxml2    ge 2.19
printf_dependency() {
    case $2 in
        command)
            case $3 in
                *:*)
                    if [ "$1" = 'required' ] ; then
                        REQUIRED_ITEM="$(map_get MAP_REQUIRED_DEPENDENCIES "$3")"
                        __printf_dependency "$2" "$REQUIRED_ITEM" "$4" "$5" "$(version_of_command $REQUIRED_ITEM)" "$(command -v $REQUIRED_ITEM)"
                    else
                        for item in $(echo "$3" | tr ':' ' ')
                        do
                            __printf_dependency "$2" "$item" "$4" "$5" "$(version_of_command $item)" "$(command -v $item)"
                        done
                    fi
                    ;;
                *)  __printf_dependency "$2" "$3" "$4" "$5" "$(version_of_command $3)" "$(command -v $3)"
            esac
            ;;
        module.py)
            __printf_dependency "$2" "$3" "$4" "$5" "$(python_module get version "$item")" "$(python_module get location "$item")"
            ;;
        module.pl)
            __printf_dependency "$2" "$3" "$4" "$5" "$(perl_module get version "$item")" "$(perl_module get location "$item")"
            ;;
        *)  die "$2: type not support."
    esac
}

__printf_required_dependencies() {
    step "printf required dependencies"
    if [ -z "$REQUIRED_DEPENDENCY_LIST" ] ; then
        warn "no required dependencies."
    else
        __printf_dependency TYPE NAME OP EXPECT ACTUAL LOCATION
        for dependency in $REQUIRED_DEPENDENCY_LIST
        do
            printf_dependency $(__decode_dependency "$dependency")
        done
    fi
}

__printf_optional_dependencies() {
    step "printf optional dependencies"
    if [ -z "$OPTIONAL_DEPENDENCY_LIST" ] ; then
        warn "no optional dependencies."
    else
        __printf_dependency TYPE NAME OP EXPECT ACTUAL LOCATION
        for dependency in $OPTIONAL_DEPENDENCY_LIST
        do
            printf_dependency $(__decode_dependency "$dependency")
        done
    fi
}

# }}}
##############################################################################
# {{{ python_module

# examples:
# python_module is  installed libxml2
# python_module get version   libxml2
# python_module get location  libxml2
# python_module install       libxml2
python_module() {
    case $1 in
        is)
            [ $# -eq 3 ] || die "[python_module is] command accept 2 arguments."

            handle_dependency required command python3:python3.9:python3.8:python3.7:python3.6:python3.5:python
            handle_dependency required command pip3:pip3.9:pip3.8:pip3.7:pip3.6:pip3.5:pip

            __PYTHON_COMMAND__=$(command -v python3 || command -v python3.9 || command -v python3.8 || command -v python3.7 || command -v python3.6 || command -v python3.5 || command -v python || echo python)
            __PIP_COMMAND__=$(command -v pip3 || command -v pip3.9 || command -v pip3.8 || command -v pip3.7 || command -v pip3.6 || command -v pip3.5 || command -v pip || echo pip)

            case $2 in
                installed)  "$__PYTHON_COMMAND__" -c "import $3" 2> /dev/null ;;
                *) die "python_module is $2: not support."
            esac
            ;;
        get)
            [ $# -eq 3 ] || die "[python_module get] command accept 2 arguments."

            handle_dependency required command python3:python3.9:python3.8:python3.7:python3.6:python3.5:python
            handle_dependency required command pip3:pip3.9:pip3.8:pip3.7:pip3.6:pip3.5:pip

            __PYTHON_COMMAND__=$(command -v python3 || command -v python3.9 || command -v python3.8 || command -v python3.7 || command -v python3.6 || command -v python3.5 || command -v python || echo python)
            __PIP_COMMAND__=$(command -v pip3 || command -v pip3.9 || command -v pip3.8 || command -v pip3.7 || command -v pip3.6 || command -v pip3.5 || command -v pip || echo pip)

            case $2 in
                version)  "$__PIP_COMMAND__" show "$3" 2> /dev/null | grep 'Version:'  | cut -d ' ' -f2 ;;
                location) "$__PIP_COMMAND__" show "$3" 2> /dev/null | grep 'Location:' | cut -d ' ' -f2 ;;
                *) die "python_module get $2: not support."
            esac
            ;;
        install)
            [ -z "$2" ] && die "please specify a python module name."
            if ! python_module is installed "$2" ; then
                warn "required python module ${COLOR_GREEN}$2${COLOR_OFF}, but ${COLOR_GREEN}$2${COLOR_OFF} python module not found, try to install it via ${COLOR_GREEN}$__PIP_COMMAND__${COLOR_OFF}"
                run "$__PIP_COMMAND__" install -U pip  || return 1
                run "$__PIP_COMMAND__" install -U "$2" || return 1
            fi
            ;;
        *)  die "python_module $1: not support."
    esac
}

# }}}
##############################################################################
# {{{ perl_module

# examples:
# perl_module is  installed libxml2
# perl_module get version   libxml2
# perl_module get location  libxml2
# perl_module install       libxml2
perl_module() {
    case $1 in
        is)
            [ $# -eq 4 ] || die "perl_module command accept 4 arguments."
            handle_dependency required command perl
            case $2 in
                installed)  perl -M"$3" -le 'print "installed"' > /dev/null 2>&1 ;;
                *) die "perl_module is $2: not support."
            esac
            ;;
        get)
            ;;
        install)
            if ! perl_module is installed "$3" ; then
                handle_dependency required command cpan:cpanm
                if   command_exists_in_filesystem cpan  ; then
                    cpan -i "$3"
                elif command_exists_in_filesystem cpanm ; then
                    cpanm "$3"
                else
                    die "no perl module installer found."
                fi
            fi
            ;;
        *)  die "perl_module $1: not support."
    esac
}

# }}}
##############################################################################
# {{{ encode/decode dependency

__encode_dependency() {
    if [ $# -eq 0 ] ; then
        tr ' ' '|'
    else
        printf "%s" "$*" | tr ' ' '|'
    fi
}

__decode_dependency() {
    if [ $# -eq 0 ] ; then
        tr '|' ' '
    else
        printf "%s" "$*" | tr '|' ' '
    fi
}

# }}}
##############################################################################
# {{{ regist dependency

# regist dependency
#
# required this is a required dependency
# optional this is a optional dependency
#
# command  this dependency is a command
# python   this dependency is a python  module
# python2  this dependency is a python2 module
# python3  this dependency is a python3 module
# perl     this dependency is a perl module
#
# gt VERSION
# ge VERSION
# lt VERSION
# le VERSION
# eq VERSION
# ne VERSION
#
# examples:
# regist_dependency required command pkg-config ge 0.18
# regist_dependency required command python     ge 3.5
# regist_dependency required python  libxml2    ge 2.19
#
# regist_dependency optional command pkg-config ge 0.18
# regist_dependency optional command python     ge 3.5
# regist_dependency optional python  libxml2    ge 2.19
regist_dependency() {
    case $1 in
        required)
            if [ -z "$REQUIRED_DEPENDENCY_LIST" ] ; then
                REQUIRED_DEPENDENCY_LIST="$(__encode_dependency "$*")"
            else
                REQUIRED_DEPENDENCY_LIST="$REQUIRED_DEPENDENCY_LIST $(__encode_dependency "$*")"
            fi
            ;;
        optional)
            if [ -z "$OPTIONAL_DEPENDENCY_LIST" ] ; then
                OPTIONAL_DEPENDENCY_LIST=$(__encode_dependency "$*")
            else
                OPTIONAL_DEPENDENCY_LIST="$OPTIONAL_DEPENDENCY_LIST $(__encode_dependency "$*")"
            fi
    esac
}

# }}}
##############################################################################
# {{{ main

main() {
    set -e

    unset INSTALLED_NDK_PKG
    INSTALLED_NDK_PKG=$(command -v ndk-pkg) || true

    if [ -n "$INSTALLED_NDK_PKG" ] ; then
        die "ndk-pkg is already installed at $INSTALLED_NDK_PKG."
    fi

    VERSION='0.1.0'
    FILE_NAME="ndk-pkg-${VERSION}.tar.gz"
    URL="https://github.com/leleliu008/ndk-pkg/releases/download/v${VERSION}/${FILE_NAME}"

    INSTALL_BIN_DIR='/usr/local/bin'
    INSTALL_BIN_FILEPATH="$INSTALL_BIN_DIR/ndk-pkg"

    INSTALL_ZSH_COMPLETION_DIR='/usr/local/share/zsh/site-functions'
    INSTALL_ZSH_COMPLETION_FILEPATH="$INSTALL_ZSH_COMPLETION_DIR/_ndk-pkg"

    INSTALL_SUCCESS=false

    if [ -e "$INSTALL_BIN_FILEPATH" ] ; then
        die "$INSTALL_BIN_FILEPATH is already exist."
    fi

    if [ -e "$INSTALL_ZSH_COMPLETION_FILEPATH" ] ; then
        die "$INSTALL_ZSH_COMPLETION_FILEPATH is already exist."
    fi
    
    on_exit() {
        if [ "$INSTALL_SUCCESS" = true ] ; then
            return 0
        else
            if [ -f "$INSTALL_BIN_FILEPATH" ] ; then
                if [ -w "$INSTALL_BIN_FILEPATH" ] ; then
                         rm -f "$INSTALL_BIN_FILEPATH" 2>/dev/null
                else
                    sudo rm -f "$INSTALL_BIN_FILEPATH" 2>/dev/null
                fi
            fi
            if [ -f "$INSTALL_ZSH_COMPLETION_FILEPATH" ] ; then
                if [ -w "$INSTALL_ZSH_COMPLETION_FILEPATH" ] ; then
                         rm -f "$INSTALL_ZSH_COMPLETION_FILEPATH" 2>/dev/null
                else
                    sudo rm -f "$INSTALL_ZSH_COMPLETION_FILEPATH" 2>/dev/null
                fi
            fi
        fi
    }

    trap on_exit EXIT

    info "mktemp -d"
    WORKING_DIR=$(mktemp -d)

    run cd $WORKING_DIR

    fetch "$URL"
    
    tar xf "$FILE_NAME"

    if [ ! -d "$INSTALL_BIN_DIR" ] ; then
        run install -d "$INSTALL_BIN_DIR"
    fi

    if [ -w "$INSTALL_BIN_DIR" ] ; then
        run      install -m 555 bin/ndk-pkg "$INSTALL_BIN_FILEPATH"
    else
        run sudo install -m 555 bin/ndk-pkg "$INSTALL_BIN_FILEPATH"
    fi

    if [ ! -d "$INSTALL_ZSH_COMPLETION_DIR" ] ; then
        run install -d "$INSTALL_ZSH_COMPLETION_DIR"
    fi

    if [ -w "$INSTALL_ZSH_COMPLETION_DIR" ] ; then
        run      install -m 644 zsh-completion/_ndk-pkg "$INSTALL_ZSH_COMPLETION_FILEPATH"
    else
        run sudo install -m 644 zsh-completion/_ndk-pkg "$INSTALL_ZSH_COMPLETION_FILEPATH"
    fi

    INSTALL_SUCCESS=true

    echo
    success "Installed Success.\n"
    print "${COLOR_PURPLE}Note${COLOR_OFF} : ${COLOR_GREEN}$INSTALL_ZSH_COMPLETION_FILEPATH${COLOR_OFF} is a zsh-completion script for ${COLOR_PURPLE}ndk-pkg${COLOR_OFF}. In zsh, when you've typed ${COLOR_PURPLE}ndk-pkg${COLOR_OFF} then type ${COLOR_PURPLE}TAB${COLOR_OFF} key, it will auto complete the rest for you. to apply this feature, you may need to run the command ${COLOR_PURPLE}autoload -U compinit && compinit${COLOR_OFF}\n"
}

main
