#!/bin/sh

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
    print "${COLOR_YELLOW}🔥  $*\n${COLOR_OFF}" >&2
}

error() {
    print "${COLOR_RED}[✘] $*\n${COLOR_OFF}" >&2
}

die() {
    print "${COLOR_RED}[✘] $*\n${COLOR_OFF}" >&2
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

die_if_command_not_found() {
    for item in $@
    do
        command_exists_in_filesystem $item || die "$item command not found."
    done
}

main() {
    for arg in $@
    do
        case $arg in
            -x) set -x ;;
        esac
    done

    set -e

    die_if_command_not_found tar gzip git gh

    unset RELEASE_VERSION_MAJOR_PLUS_PLUS
    unset RELEASE_VERSION_MINOR_PLUS_PLUS
    unset RELEASE_VERSION_PATCH_PLUS_PLUS

    while [ -n "$1" ]
    do
        case $1 in
            -x) ;;
            --major++) RELEASE_VERSION_MAJOR_PLUS_PLUS=1 ;;
            --minor++) RELEASE_VERSION_MINOR_PLUS_PLUS=1 ;;
            --patch++) RELEASE_VERSION_PATCH_PLUS_PLUS=1 ;;
            *) die "unrecognized argument: $1"
        esac
        shift
    done

    unset RELEASE_VERSION
    unset RELEASE_VERSION_MAJOR
    unset RELEASE_VERSION_MINOR
    unset RELEASE_VERSION_PATCH

    RELEASE_VERSION=$(bin/ndk-pkg --version)
    RELEASE_VERSION_MAJOR=$(printf '%s\n' "$RELEASE_VERSION" | cut -d. -f1)
    RELEASE_VERSION_MINOR=$(printf '%s\n' "$RELEASE_VERSION" | cut -d. -f2)
    RELEASE_VERSION_PATCH=$(printf '%s\n' "$RELEASE_VERSION" | cut -d. -f3)

    if [ ${RELEASE_VERSION_MAJOR_PLUS_PLUS-0} -eq 1 ] ; then
        RELEASE_VERSION_MAJOR=$(expr $RELEASE_VERSION_MAJOR + 1)
    fi

    if [ ${RELEASE_VERSION_MINOR_PLUS_PLUS-0} -eq 1 ] ; then
        RELEASE_VERSION_MINOR=$(expr $RELEASE_VERSION_MINOR + 1)
    fi

    if [ ${RELEASE_VERSION_PATCH_PLUS_PLUS-0} -eq 1 ] ; then
        RELEASE_VERSION_PATCH=$(expr $RELEASE_VERSION_PATCH + 1)
    fi

    RELEASE_VERSION="$RELEASE_VERSION_MAJOR.$RELEASE_VERSION_MINOR.$RELEASE_VERSION_PATCH"

    unset RELEASE_FILE_NAME
    RELEASE_FILE_NAME="ndk-pkg-$RELEASE_VERSION.tar.gz"

    run tar zvcf "$RELEASE_FILE_NAME" bin/ndk-pkg zsh-completion/_ndk-pkg

    unset RELEASE_FILE_SHA256SUM
    RELEASE_FILE_SHA256SUM=$(sha256sum "$RELEASE_FILE_NAME")

    success "sha256sum($RELEASE_FILE_NAME)=$RELEASE_FILE_SHA256SUM"

    if [ ${RELEASE_VERSION_MAJOR_PLUS_PLUS-0} -eq 1 ] || [ ${RELEASE_VERSION_MINOR_PLUS_PLUS-0} -eq 1 ] || [ ${RELEASE_VERSION_PATCH_PLUS_PLUS-0} -eq 1 ] ; then
        sed_in_place "s|MY_VERSION=[0-9].[0-9].[0-9]|MY_VERSION=$RELEASE_VERSION|" bin/ndk-pkg
        sed_in_place "s|RELEASE_VERSION='[0-9].[0-9].[0-9]'|RELEASE_VERSION='$RELEASE_VERSION'|" install.sh

        run git add bin/ndk-pkg install.sh
        run git commit -m "'publish new version $RELEASE_VERSION'"
        run git push origin master
    fi

    run gh release create v"$RELEASE_VERSION" "ndk-pkg-$RELEASE_VERSION.tar.gz" --notes "'release $RELEASE_VERSION'"

    run git clone git@github.com:leleliu008/homebrew-fpliu.git

    run cd homebrew-fpliu

    sed_in_place "/sha256   /c \  sha256   \"$RELEASE_FILE_SHA256SUM\"" Formula/ndk-pkg.rb
    sed_in_place "s@[0-9]\.[0-9]\.[0-9]@$RELEASE_VERSION@g" Formula/ndk-pkg.rb

    run git add Formula/ndk-pkg.rb
    run git commit -m "'publish new version ndk-pkg-$RELEASE_VERSION'"
    run git push origin master

    run cd ..

    run rm -rf homebrew-fpliu
    run rm "$RELEASE_FILE_NAME"
}

main $@
