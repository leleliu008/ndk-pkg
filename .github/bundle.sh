#!/bin/sh

set -ex

if [ "$GITHUB_ACTIONS" != true ] ; then
    echo "this script only run via GitHub Actions."
    exit 1
fi

curl -LO https://raw.githubusercontent.com/adobe-fonts/source-code-pro/release/OTF/SourceCodePro-Light.otf
curl -LO https://git.savannah.gnu.org/cgit/config.git/plain/config.sub
curl -LO https://git.savannah.gnu.org/cgit/config.git/plain/config.guess

chmod +x config.sub config.guess

install -d out/

for item in *.exe
do
    mv "$item" "out/${item%.exe}"
done

tar vxf uppm*.tar.xz -C out --strip-components=1

mv out/bin/uppm *.otf core/fonts.conf config.sub config.guess out/

rm -rf out/share/ out/.ppkg/

DIRNAME="ndk-pkg-core-$1-$2"
mv out "$DIRNAME"
tar cJvf "$DIRNAME.tar.xz" "$DIRNAME"
