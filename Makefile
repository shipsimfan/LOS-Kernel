all: kernel-all

clean: kernel-clean

run: all
	make -C ./kernel run

%-all: %
	make -C ./$^

%-clean: %
	make -C ./$^ clean

.PHONY: kernel