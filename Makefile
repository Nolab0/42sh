build:
	meson setup builddir
	ninja -C builddir

42sh: build
	@echo test

check: build
	./builddir/tests

clean:
	rm -f builddir