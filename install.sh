#!/bin/sh

VERSION='0.1.0'
FILE_NAME="ndk-pkg-${VERSION}.tar.gz"
URL="https://github.com/leleliu008/ndk-pkg/releases/download/v${VERSION}/${FILE_NAME}"
INSTALL_DIR=/usr/local/opt/ndk-pkg
DEST_LINK_BIN=/usr/local/bin/ndk-pkg
DEST_LINK_ZSH_COMPLETION=/usr/local/share/zsh/site-functions/_ndk-pkg

COLOR_RED='\033[0;31m'          # Red
COLOR_GREEN='\033[0;32m'        # Green
COLOR_PURPLE='\033[0;35m'       # Purple
COLOR_OFF='\033[0m'             # Reset

msg() {
    printf "%b" "$*"
}

info() {
    msg "${COLOR_PURPLE}$*\n${COLOR_OFF}"
}

success() {
    msg "${COLOR_GREEN}[✔] $*\n${COLOR_OFF}"
}

die() {
    msg "${COLOR_RED}🔥 $*\n${COLOR_OFF}"
    exit 1
}

own() {
    ls -ld "$1" | awk '{print $3":"$4}'
}

on_exit() {
    [ "$SUCCESS" = 'true' ] && return 0
    [ "$IS_CREATED_BY_ME_INSTALL_DIR" = 'true' ] && rm -rf "$INSTALL_DIR"
    [ "$IS_CREATED_BY_ME_LINK_BIN" = 'true' ] && rm "$DEST_LINK_BIN"
    [ "$IS_CREATED_BY_ME_LINK_ZSH_COMPLETION" = 'true' ] && rm "$DEST_LINK_ZSH_COMPLETION"
}

download() {
    if command -v curl > /dev/null ; then
        info "Downloading $URL"
        curl -LO "$URL"
    elif command -v wget > /dev/null ; then
        info "Downloading $URL"
        wget "$URL"
    else
        die "please install curl or wget utility."
    fi
}

main() {
    command -v ndk-pkg    && die "ndk-pkg is already installed."
    [ -d "$INSTALL_DIR" ] && die "ndk-pkg is already installed. in $INSTALL_DIR"
    
    unset SUCCESS
    unset IS_CREATED_BY_ME_INSTALL_DIR
    unset IS_CREATED_BY_ME_LINK_BIN
    unset IS_CREATED_BY_ME_LINK_ZSH_COMPLETION
    
    trap on_exit EXIT
    
    if mkdir -p "$INSTALL_DIR" ; then
        IS_CREATED_BY_ME_INSTALL_DIR=true
        cd "$INSTALL_DIR" || exit 1
    fi
     
    if download ; then
        info "Downloaded at $PWD/$FILE_NAME"
    else
        die "Download occured error."
    fi
    
    info "Uncompressing $PWD/$FILE_NAME"
    if tar xf "$FILE_NAME" ; then
        info "Uncompressed in $PWD"
    else
        die "tar vxf $FILE_NAME occured error."
    fi

    chown -R "$(own .)" .
    chmod 755 bin/ndk-pkg
    chmod 444 zsh-completion/_ndk-pkg
    
    if [ -f "$DEST_LINK_BIN" ] ; then
        die "$DEST_LINK_BIN is already exist."
    else
        [ -d '/usr/local/bin' ] || mkdir -p /usr/local/bin
        ln -s "$INSTALL_DIR/bin/ndk-pkg" "$DEST_LINK_BIN" &&
        IS_CREATED_BY_ME_LINK_BIN=true
    fi
    
    if [ -f "$DEST_LINK_ZSH_COMPLETION" ] ; then
        die "$DEST_LINK_ZSH_COMPLETION is already exist."
    else
        [ -d /usr/local/share/zsh/site-functions ] || mkdir -p /usr/local/share/zsh/site-functions
        ln -s "$INSTALL_DIR/zsh-completion/_ndk-pkg" "$DEST_LINK_ZSH_COMPLETION" &&
        IS_CREATED_BY_ME_LINK_ZSH_COMPLETION=true
    fi

    SUCCESS=true 
    success "Installed success."
    msg "${COLOR_PURPLE}Note${COLOR_OFF} : I have provide a zsh-completion script for ${COLOR_PURPLE}ndk-pkg${COLOR_OFF}. when you've typed ${COLOR_PURPLE}ndk-pkg${COLOR_OFF} then type ${COLOR_PURPLE}TAB${COLOR_OFF} key, it will auto complete the rest for you. to apply this feature, you may need to run the command ${COLOR_PURPLE}autoload -U compinit && compinit${COLOR_OFF}"
}

main
