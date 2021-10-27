all:
	@if [ -e src/Makefile ]; then \
		$(MAKE) -C src $(MAKEFLAGS); \
	else \
		if [ -e mf/Makefile.$$(uname) ]; then \
			ln -s ../mf/Makefile.$$(uname) src/Makefile && \
			$(MAKE) -C src $(MAKEFLAGS); \
		else \
			echo "Run: make <target>" && \
			echo "Available targets are:" && \
			ls mf/ | sed 's/Makefile\.//g'; \
		fi \
	fi

.PHONY: clean install distclean

install:
	$(MAKE) -C src $(MAKEFLAGS) install

uninstall:
	$(MAKE) -C src $(MAKEFLAGS) uninstall

clean:
	$(MAKE) -C src $(MAKEFLAGS) clean

distclean:
	-$(MAKE) -C src $(MAKEFLAGS) clean
	-rm src/Makefile
	
.DEFAULT:
	@if [ -e src/Makefile ]; then rm src/Makefile; fi
	@if ! [ -f mf/Makefile.$@ ]; then \
		echo "Invalid target name: $@" && exit 1; fi
	ln -s ../mf/Makefile.$@ src/Makefile
	$(MAKE) -C src $(MAKEFLAGS)
