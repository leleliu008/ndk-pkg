#!/bin/sh

COLOR_RED='\033[0;31m'          # Red
COLOR_GREEN='\033[0;32m'        # Green
COLOR_YELLOW='\033[0;33m'       # Yellow
COLOR_BLUE='\033[0;94m'         # Blue
COLOR_PURPLE='\033[0;35m'       # Purple
COLOR_OFF='\033[0m'             # Reset

print() {
    printf '%b' "$*"
}

echo() {
    printf '%b\n' "$*"
}

info() {
    printf '%b\n' "💠  $*"
}

note() {
    printf '%b\n' "${COLOR_YELLOW}🔔  $*${COLOR_OFF}" >&2
}

warn() {
    printf '%b\n' "${COLOR_YELLOW}🔥  $*${COLOR_OFF}" >&2
}

success() {
    printf '%b\n' "${COLOR_GREEN}[✔] $*${COLOR_OFF}"
}

error() {
    printf '%b\n' "${COLOR_RED}💔  $*${COLOR_OFF}" >&2
}

die() {
    printf '%b\n' "${COLOR_RED}💔  $*${COLOR_OFF}" >&2
    exit 1
}

step() {
    STEP_NUM=$(expr ${STEP_NUM-0} + 1)
    STEP_MESSAGE="$@"
    echo
    echo "${COLOR_PURPLE}=>> STEP ${STEP_NUM} : ${STEP_MESSAGE} ${COLOR_OFF}"
}

run() {
    echo "$COLOR_PURPLE==>$COLOR_OFF $COLOR_GREEN$@$COLOR_OFF"
    eval "$*"
}

sed_in_place() {
    echo "${COLOR_PURPLE}==>${COLOR_OFF} ${COLOR_GREEN}sed -i $@${COLOR_OFF}"
    sed -i "$@"
}

main() {
    set -e

    for arg in "$@"
    do
        case $arg in
            -x) set -x ; break
        esac
    done

    for cmd in sha256sum tar gzip xz git gh
    do
        command -v "$cmd" > /dev/null || die "command not found: $cmd"
    done

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

    RELEASE_VERSION=$(./ndk-pkg --version)
    RELEASE_VERSION_MAJOR=$(printf '%s\n' "$RELEASE_VERSION" | cut -d. -f1)
    RELEASE_VERSION_MINOR=$(printf '%s\n' "$RELEASE_VERSION" | cut -d. -f2)
    RELEASE_VERSION_PATCH=$(printf '%s\n' "$RELEASE_VERSION" | cut -d. -f3)

    if [ ${RELEASE_VERSION_MAJOR_PLUS_PLUS-0} -eq 0 ] && [ ${RELEASE_VERSION_MINOR_PLUS_PLUS-0} -eq 0 ] && [ ${RELEASE_VERSION_PATCH_PLUS_PLUS-0} -eq 0 ] ; then
        die "new release version must be bigger than old version($RELEASE_VERSION)"
    fi

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

    sed_in_place "s|NDKPKG_VERSION=[0-9]\+.[0-9]\+.[0-9]\+|NDKPKG_VERSION=$RELEASE_VERSION|" ndk-pkg

    run rm -rf "ndk-pkg-$RELEASE_VERSION"

    run install -d "ndk-pkg-$RELEASE_VERSION/bin/"
    run install -d "ndk-pkg-$RELEASE_VERSION/share/zsh/site-functions/"

    run cp ndk-pkg "ndk-pkg-$RELEASE_VERSION/bin/ndk-pkg"
    run cp ndk-pkg-zsh-completion "ndk-pkg-$RELEASE_VERSION/share/zsh/site-functions/_ndk-pkg"

    RELEASE_FILE_NAME="ndk-pkg-$RELEASE_VERSION.tar.gz"

    run tar zvcf "$RELEASE_FILE_NAME" "ndk-pkg-$RELEASE_VERSION"

    run rm -rf "ndk-pkg-$RELEASE_VERSION"

    RELEASE_FILE_SHA256SUM=
    RELEASE_FILE_SHA256SUM="$(sha256sum "$RELEASE_FILE_NAME")"

    success "sha256sum($RELEASE_FILE_NAME)=$RELEASE_FILE_SHA256SUM"

    run git add bin/ndk-pkg
    run git commit -m "'publish new version $RELEASE_VERSION'"
    run git push origin master

    run gh release create v"$RELEASE_VERSION" "$RELEASE_FILE_NAME" --notes "'release $RELEASE_VERSION'"

    run rm "$RELEASE_FILE_NAME"
}

main "$@"
