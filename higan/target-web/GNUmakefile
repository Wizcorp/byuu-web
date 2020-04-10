compiler = emcc

name := byuu-web
debug_host := 127.0.0.1
wasm := true
debug := true
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

all: $(objects)
	$(compiler) \
		$(flags) \
		--shell-file ./target-web/shell.html \
		--source-map-base http://$(debug_host):8000/ \
		-o ./out/$(name).html \
		$(objects)
		

	mv ./out/$(name).html ./out/index.html
