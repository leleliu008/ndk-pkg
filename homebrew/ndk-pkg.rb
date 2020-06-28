class NdkPkg < Formula
  desc     "a package manager for Android NDK."
  homepage "https://github.com/leleliu008/ndk-pkg"
  url      "https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/ndk-pkg-0.1.0.tar.gz"
  version  "0.1.0"
  sha256   "0d9b61141b989f196d0b222353b620e4a9de42b84d5d87dacd271d274fc6394b"
  head     "https://github.com/leleliu008/ndk-pkg.git"
  depends_on "curl"

  def install
    bin.install 'ndk-pkg'
    zsh_completion.install "zsh-completion/_ndk-pkg" => "_ndk-pkg"
  end

  test do
    system "#{bin}/ndk-pkg" '--help'
  end
end
