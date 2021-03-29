debug_host := 127.0.0.1:8000
wasm := true
debug := false
profiler := false
synchro := true
simulated_synchro := false
simd := false

.DEFAULT:
	$(MAKE) ${@} -C higan target=web \
		wasm=$(wasm) \
		simd=$(simd) \
		synchro=$(synchro) \
		simulated_synchro=$(simulated_synchro) \
		debug=$(debug) \
		profiler=$(profiler) \
		debug_host=$(debug_host)
