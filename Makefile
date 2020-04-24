debug_host := 127.0.0.1
wasm := true
debug := false
profiler := false
synchro := true

.DEFAULT:
	$(MAKE) ${@} -C higan target=web \
		wasm=$(wasm) \
		synchro=$(synchro) \
		debug=$(debug) \
		profiler=$(profiler) \
		debug_host=$(debug_host)
