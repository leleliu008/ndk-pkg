version = $(shell ./ndk-pkg --version | awk '{print $$2}' | head -n 1)
filename= ndk-pkg-$(version).tar.gz

dist: ndk-pkg zsh-completion/_ndk-pkg
	@tar zvcf $(filename) $^ && \
	command -v openssl > /dev/null && \
    openssl sha256 $(filename) && exit 0; \
    command -v sha256sum > /dev/null && \
    ha256sum $(filename)

publish: homebrew/ndk-pkg.rb
	cp $< /usr/local/Homebrew/Library/Taps/homebrew/homebrew-core/Formula/ndk-pkg.rb

clean:
	rm $(filename)

.PHONY: clean
