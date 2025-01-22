#!/bin/bash

[[ $1 ]] || {
	rm -rf build
	mkdir -p build/clay
	eval "$(cat << EOF | tr '\n' ' '
		clang
		-Wall
		-Werror
		-Os
		-DCLAY_WASM
		-mbulk-memory
		--target=wasm32
		-nostdlib
		-Wl,--strip-all
		-Wl,--export-dynamic
		-Wl,--no-entry
		-Wl,--export=__heap_base
		-Wl,--initial-memory=6553600
		-o build/clay/index.wasm
		src/main.c
EOF
	)"
}

for file in fonts images index.html main.js; do
	cp -rf $file build/clay/$file
done

[[ $cur_port ]] || cur_port=8081
http-server build -p $((--cur_port)) -a 127.0.0.1
