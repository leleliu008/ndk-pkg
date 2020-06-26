class NdkPkg < Formula
  desc     "a package manager for Android NDK."
  homepage "https://github.com/leleliu008/ndk-pkg"
  url      "https://raw.githubusercontent.com/leleliu008/ndk-pkg/master/ndk-pkg-0.1.0.tar.gz"
  version  "0.1.0"
  sha256   "506338632ce4e8ff4fe0f38562c191406cbd29c0f7254fa3886621d5d0982c6b"
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
