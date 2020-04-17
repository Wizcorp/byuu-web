compiler = emcc

name := byuu-web
debug_host := 127.0.0.1
wasm := true
debug := false
synchro := true

ifeq ($(wasm), true)
  flags += -s WASM=1 
  ifeq ($(debug), true)
	flags += -g4 --profiling --cpuprofiler 
	# flags += --memoryprofiler
  endif
else
  flags += -s WASM=0 -s AGGRESSIVE_VARIABLE_ELIMINATION=1
endif

ifeq ($(synchro),true)
  flags += -DSCHEDULER_SYNCHRO
endif

# -Werror 
flags += \
	-fPIC \
	-DICARUS_LIBRARY \
	-ffast-math \
	-ffinite-math-only \
	-ffp-model=fast \
	-fomit-frame-pointer \
	-lopenal \
	--bind \
	-s ENVIRONMENT=web \
	-s ASYNCIFY=1 \
	-s DISABLE_EXCEPTION_CATCHING=1 \
	-s USE_SDL=2 \
	-s TOTAL_MEMORY=200MB \
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
		--source-map-base http://$(debug_host):8000/ \
		-o ./out/package/$(name)-lib.js \
		$(objects)

	cp ./target-web/package.json ./out/package/
	cp ./target-web/api.js ./out/package/
	cp ./target-web/api.d.ts ./out/package/
	cp ../README.md ./out/package/
	cp ../CHANGELOG.md ./out/package/
	cp ../LICENSE.txt ./out/package/

app: $(objects)
	mkdir -p ./out/app
	$(compiler) \
		$(flags) \
		--shell-file ./target-web/shell.html \
		--source-map-base http://$(debug_host):8000/ \
		-o ./out/app/$(name).html \
		$(objects)
		

	mv ./out/app/$(name).html ./out/app/index.html

all: package app

# Serve files by looking in folders in the following order
# This is required to deal with WASM map durring debugging, 
# so the required source file(s) may be accessed
serve:
	python ./target-web/serve.py \
    ${CURDIR}/.. \
    ${CURDIR}/out/app \
    ${CURDIR}/target-web \
    ${CURDIR} \
    ${EMSDK}=/emsdk

publish:
	$(MAKE) clean-package
	$(MAKE) package
ifneq ($(version), true)
	cd ./out/app && npm version $(version)
else
	@echo "Version bump info not specified, skipping" 
endif
	cd ./out/app && npm publish

clean-app:
	rm -rf ./out/app

clean-package:
	rm -rf ./out/package

clean: clean-app clean-package