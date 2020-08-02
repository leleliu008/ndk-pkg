version = $(shell bin/ndk-pkg --version | awk '{print $$2}' | head -n 1)
filename= ndk-pkg-$(version).tar.gz

dist: bin/ndk-pkg zsh-completion/_ndk-pkg
	@tar zvcf $(filename) $^ && \
	command -v openssl > /dev/null && \
    openssl sha256 $(filename) && exit 0; \
    command -v sha256sum > /dev/null && \
    ha256sum $(filename)

copy:
	cp $$(brew --prefix ndk-pkg)/bin/ndk-pkg bin/
	cp $$(brew --prefix ndk-pkg)/share/zsh/site-functions/_ndk-pkg zsh-completion/_ndk-pkg

clean:
	rm -f  ndk-pkg-$(version)*.tar.gz
	rm -rf src
	rm -rf pkg

.PHONY: clean copy
