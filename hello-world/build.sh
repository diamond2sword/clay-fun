#!/bin/bash

(exit $1) && {
	rm -rf build
	mkdir -p build/clay                                                       \
	&& clang                                                                  \
	-Wall                                                                     \
	-Werror                                                                   \
	-Os                                                                       \
	-nostdlib                                                                 \
	-DCLAY_WASM                                                               \
	-mbulk-memory                                                             \
	--target=wasm32                                                           \
	-Wl,--strip-all                                                           \
	-Wl,--export-dynamic                                                      \
	-Wl,--no-entry                                                            \
	-Wl,--export=__heap_base                                                  \
	-Wl,--initial-memory=6553600                                              \
	-o build/clay/index.wasm                                                  \
	main.c
}

mkdir -p build/clay
for file in fonts images index.html script.js; do
	cp -rf $file build/clay/$file
done
