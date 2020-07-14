#!/bin/sh

VERSION='0.1.0'
FILE_NAME="ndk-pkg-${VERSION}.tar.gz"
URL="https://github.com/leleliu008/ndk-pkg/releases/download/v0.1.0/${FILE_NAME}"

Color_Red='\033[0;31m'          # Red
Color_Green='\033[0;32m'        # Green
Color_Purple='\033[0;35m'       # Purple
Color_Off='\033[0m'             # Reset

msg() {
    printf "%b\n" "$1"
}

info() {
    msg "${Color_Purple}$@${Color_Off}"
}

success() {
    msg "${Color_Green}[✔] $@${Color_Off}"
}

error_exit() {
    msg "${Color_Red}🔥 $@${Color_Off}"
    exit 1
}

own() {
    ls -ld "$1" | awk '{print $3":"$4}'
}

main() {
    INSTALL_DIR='/usr/local/opt/ndk-pkg'
    if [ -d "$INSTALL_DIR" ] ; then
        error_exit "ndk-pkg is already installed in /usr/local/opt/ndk-pkg"
    else
        mkdir -p "$INSTALL_DIR" && cd "$INSTALL_DIR"
    fi
    
    if command -v curl > /dev/null ; then
        info "Downloading $URL"
        curl -LO "$URL"
    elif command -v wget > /dev/null ; then
        info "Downloading $URL"
        wget "$URL"
    else
        error_exit "please install curl or wget utility."
    fi

    if [ $? -eq 0 ] ; then
        info "Downloaded at $PWD/$FILE_NAME"
    else
        error_exit "Download occured error."
    fi
    
    info "Uncompressing $PWD/$FILE_NAME"
    tar xf "$FILE_NAME"
    
    if [ $? -eq 0 ] ; then
        info "Uncompressed in $PWD"
        
        chown -R $(own .) .
        chmod 111 bin/ndk-pkg
        chmod 400 zsh-completion/_ndk-pkg
         
        if [ -f "/usr/local/bin/ndk-pkg" ] ; then
            error_exit "/usr/local/bin/ndk-pkg is already exist."
        else
            ln -sf `pwd`/bin/ndk-pkg /usr/local/bin/ndk-pkg
        fi
        
        if [ -f "/usr/share/zsh/site-functions/_ndk-pkg" ] ; then
            error_exit "/usr/share/zsh/site-functions/_ndk-pkg is already exist."
        else
            ln -sf `pwd`/zsh-completion/_ndk-pkg /usr/share/zsh/site-functions/_ndk-pkg
        fi
         
        success "Installed success.\n"
        msg "${Color_Purple}Note${Color_Off} : I have provide a zsh-completion script for ndk-pkg. when you've typed ndk-pkg then type TAB key, it will auto complete the rest for you. to apply this feature, you may need to run the command ${Color_Purple}autoload -U compinit && compinit${Color_Off}"
    else
        error_exit "tar vxf $FILE_NAME occured error."
    fi
}

main
