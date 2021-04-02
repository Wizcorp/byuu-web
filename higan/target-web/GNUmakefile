ifeq (, $(shell which ccache))
compiler = emcc
else
$(info Using ccache)
compiler = ccache emcc
endif

name := byuu-web
debug_host := 127.0.0.1:8000
wasm := true
debug := false
profiler := false
synchro := true
simulated_synchro := false
eventinstruction_notify := false
simd := false
lto := true

PYTHON=$(shell which python3 || which python)

ifeq ($(wasm), true)
  flags += -s WASM=1 
  ifeq ($(debug), true)
	flags += -g4 -DWEB_DEBUG -s ASSERTIONS=2 -s SAFE_HEAP=1 --source-map-base http://$(debug_host)/ 
  endif
  ifeq ($(profiler), true) 
	flags += --profiling --cpuprofiler 
	# flags += --memoryprofiler
  endif
else
  flags += -s WASM=0 -s AGGRESSIVE_VARIABLE_ELIMINATION=1
endif

ifeq ($(synchro),true)
  flags += -DSCHEDULER_SYNCHRO

  # Filter out libco
  new_objects := $(filter-out libco, $(objects))
  objects := $(new_objects)

  ifeq ($(simulated_synchro), true)
    # Run synchro code on the standard scheduler. Useful for debugging
	# and porting new emulation cores
  	flags += -DSIMULATED_SYNCHRO
  endif
endif

ifeq ($(eventinstruction_notify), false)
  flags += -DNO_EVENTINSTRUCTION_NOTIFY
endif

ifeq ($(simd), true)
  flags += -msimd128
endif
	
flags += \
	-DICARUS_LIBRARY \
	-ffast-math \
	-ffinite-math-only \
	-ffp-model=fast \
	--bind \
	-s ENVIRONMENT=web \
	-s DISABLE_EXCEPTION_CATCHING=1 \
	-s USE_SDL=2 \
	-s USE_SDL_MIXER=0 \
	-s USE_SDL_IMAGE=0 \
	-s TOTAL_MEMORY=160MB \
	-s ALLOW_MEMORY_GROWTH=0

objects += web-resource web-emulator web-video web-audio web-platform web
objects += icarus icarus-resource
objects := $(objects:%=obj/%.o)

obj/web-resource.o: $(ui)/resource/resource.cpp
obj/web-emulator.o: $(ui)/emulator/emulator.cpp
obj/web-video.o: $(ui)/video/video.cpp
obj/web-audio.o: $(ui)/audio/audio.cpp
obj/web-platform.o: $(ui)/platform/platform.cpp
obj/web.o: $(ui)/web.cpp
obj/icarus.o: ../icarus/icarus.cpp
obj/icarus-resource.o: ../icarus/resource/resource.cpp

package: $(objects)
	mkdir -p ./out/package
	$(compiler) \
		$(flags) \
		-s MODULARIZE=1 \
		-s EXPORT_ES6=1 \
		-s USE_ES6_IMPORT_META=0 \
		-o ./out/package/$(name)-lib.js \
		$(objects)

	wasm-strip ./out/package/byuu-web-lib.wasm

	cp ./target-web/package.json ./out/package/
	cp ../README.md ./out/package/
	cp ../CHANGELOG.md ./out/package/
	cp ../LICENSE.txt ./out/package/

	$(PYTHON) ./target-web/build.py ./target-web/api.js > ./out/package/api.js
	$(PYTHON) ./target-web/build.py ./target-web/api.d.ts > ./out/package/api.d.ts

app: $(objects)
	mkdir -p ./out/app
	$(compiler) \
		$(flags) \
		--shell-file ./target-web/shell.html \
		-o ./out/app/$(name).html \
		$(objects)
		
	mv ./out/app/$(name).html ./out/app/index.html
	cp -R ./target-web/assets ./out/app/
	$(PYTHON) ./target-web/build.py ./target-web/service-worker.js > ./out/app/service-worker.js

all: package app

# Serve files by looking in folders in the following order
# This is required to deal with WASM map durring debugging, 
# so the required source file(s) may be accessed
serve:
	python ./target-web/serve.py \
    ${CURDIR}/.. \
    ${CURDIR}/out/app \
    ${CURDIR}/out/package \
    ${CURDIR}/target-web \
    ${CURDIR} \
    ${EMSDK}=/emsdk

clean-app:
	rm -rf ./out/app

clean-package:
	rm -rf ./out/package

clean: clean-app clean-package