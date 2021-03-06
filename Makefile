# Copyright 2020, Trevor Sundberg. See LICENSE.md

# These rules aren't backed by files and will always run
.PHONY: wasm2brs doom files mandelbrot javascript rust cmake test clean run_test all

# This rule must be first so it runs when you don't specify a target
all: wasm2brs doom files mandelbrot javascript rust cmake test

# Because we call into cmake, we don't know whether some rules need to be updated
# For example in rule build/wasm2brs/wasm2brs we don't know if brs-writer.cc changed
# unless we run the cmake every time. The solution is to FORCE cmake to run every time
# by using an empty rule. This also has the advantage over a PHONY rule that anyone
# who depends on a rule that uses FORCE (e.g. build/wasm2brs/wasm2brs) is not themselves
# forced to rebuild each time, but they would if we used a PHONY.
FORCE: ;

# --- clean
clean:
	$(call clean-project)
	rm -rf build
	rm -rf test/node_modules

define clean-project
	rm -rf project/source/*.out*
	rm -rf project/source/*.wad
	rm -rf project/manifest
endef

# --- wasm2brs
wasm2brs: build/wasm2brs/wasm2brs

build/wasm2brs/wasm2brs: build/wasm2brs/Makefile FORCE
	GNUMAKEFLAGS=--no-print-directory cmake --build ./build/wasm2brs --parallel

build/wasm2brs/Makefile:
	mkdir -p build/wasm2brs
	cd build/wasm2brs && cmake ../..

# --- test
test: build/test/index.js

run_test: build/test/index.js build/wasm2brs/wasm2brs
	$(call clean-project)
	NODE_PATH=test/node_modules node build/test/index.js $(ARGS)

build/test/index.js: test/index.ts test/tsconfig.json test/node_modules
	rm -rf build/test/ && cd test && npm run build

test/node_modules: test/package.json
	cd test && npm install && touch node_modules

# --- doom
doom: build/doom/doom-wasm.out.brs
	$(call clean-project)
	cp build/doom/doom-wasm.out*.brs project/source/
	cp samples/doom/doom.brs project/source/doom.out.brs
	cp samples/doom/doom1.wad project/source/doom1.wad
	cp samples/doom/manifest project/manifest

build/doom/doom-wasm.out.brs: build/doom/doom.wasm build/wasm2brs/wasm2brs
	./build/wasm2brs/third_party/binaryen/bin/wasm-opt -g -O4 ./build/doom/doom.wasm -o ./build/doom/doom-opt.wasm
	./build/wasm2brs/wasm2brs -o build/doom/doom-wasm.out.brs ./build/doom/doom-opt.wasm

build/doom/doom.wasm: build/doom/Makefile FORCE
	GNUMAKEFLAGS=--no-print-directory cmake --build ./build/doom --parallel

build/doom/Makefile:
	mkdir -p build/doom
	cd build/doom && wasimake cmake ../../samples/doom

# --- files
files: build/files/files-wasm.out.brs
	$(call clean-project)
	cp build/files/files-wasm.out*.brs project/source/
	cp samples/files/files.brs project/source/files.out.brs
	cp samples/files/manifest project/manifest

build/files/files-wasm.out.brs: build/files/files.wasm build/wasm2brs/wasm2brs
	./build/wasm2brs/third_party/binaryen/bin/wasm-opt -g -Oz ./build/files/files.wasm -o ./build/files/files-opt.wasm
	./build/wasm2brs/wasm2brs -o build/files/files-wasm.out.brs ./build/files/files-opt.wasm

build/files/files.wasm: samples/files/files.cc
	mkdir -p build/files
	wasic++ -g -Oz samples/files/files.cc -o ./build/files/files.wasm

# --- cmake
cmake: build/cmake/cmake-wasm.out.brs
	$(call clean-project)
	cp build/cmake/cmake-wasm.out*.brs project/source/
	cp samples/cmake/cmake.brs project/source/cmake.out.brs
	cp samples/cmake/manifest project/manifest

build/cmake/cmake-wasm.out.brs: build/cmake/Makefile build/wasm2brs/wasm2brs FORCE
	GNUMAKEFLAGS=--no-print-directory cmake --build ./build/cmake --parallel

build/cmake/Makefile:
	mkdir -p build/cmake
	cd build/cmake && wasimake cmake ../../samples/cmake

# --- mandelbrot
mandelbrot: build/mandelbrot/mandelbrot-wasm.out.brs
	$(call clean-project)
	cp build/mandelbrot/mandelbrot-wasm.out*.brs project/source/
	cp samples/mandelbrot/mandelbrot.brs project/source/mandelbrot.out.brs
	cp samples/mandelbrot/manifest project/manifest

build/mandelbrot/mandelbrot-wasm.out.brs: build/mandelbrot/mandelbrot.wasm build/wasm2brs/wasm2brs
	./build/wasm2brs/third_party/binaryen/bin/wasm-opt -O4 ./build/mandelbrot/mandelbrot.wasm -o ./build/mandelbrot/mandelbrot-opt.wasm
	./build/wasm2brs/wasm2brs -o build/mandelbrot/mandelbrot-wasm.out.brs ./build/mandelbrot/mandelbrot-opt.wasm

build/mandelbrot/mandelbrot.wasm: samples/mandelbrot/mandelbrot.c
	mkdir -p build/mandelbrot
	clang -Ofast --target=wasm32 -nostdlib -Wl,--no-entry samples/mandelbrot/mandelbrot.c -o ./build/mandelbrot/mandelbrot.wasm

# --- javascript
javascript: build/javascript/javascript-wasm.out.brs
	$(call clean-project)
	cp build/javascript/javascript-wasm.out*.brs project/source/
	cp samples/javascript/javascript.brs project/source/javascript.out.brs
	cp samples/javascript/manifest project/manifest

build/javascript/javascript-wasm.out.brs: build/javascript/javascript.wasm build/wasm2brs/wasm2brs
	./build/wasm2brs/third_party/binaryen/bin/wasm-opt -g -O4 ./build/javascript/javascript.wasm -o ./build/javascript/javascript-opt.wasm
	./build/wasm2brs/wasm2brs -o build/javascript/javascript-wasm.out.brs ./build/javascript/javascript-opt.wasm

build/javascript/javascript.wasm: build/javascript/Makefile FORCE
	GNUMAKEFLAGS=--no-print-directory cmake --build ./build/javascript --parallel

build/javascript/Makefile:
	mkdir -p build/javascript
	cd build/javascript && wasimake cmake ../../samples/javascript

# --- rust
rust: build/rust/rust-wasm.out.brs
	$(call clean-project)
	cp build/rust/rust-wasm.out*.brs project/source/
	cp samples/rust/rust.brs project/source/rust.out.brs
	cp samples/rust/manifest project/manifest

build/rust/rust-wasm.out.brs: build/rust/rust.wasm build/wasm2brs/wasm2brs
	./build/wasm2brs/third_party/binaryen/bin/wasm-opt -g -O4 ./build/rust/rust.wasm -o ./build/rust/rust-opt.wasm
	./build/wasm2brs/wasm2brs -o build/rust/rust-wasm.out.brs ./build/rust/rust-opt.wasm

build/rust/rust.wasm: samples/rust/rust.rs
	mkdir -p build/rust
	cd samples/rust && rustc --target wasm32-wasi rust.rs -o ../../build/rust/rust.wasm