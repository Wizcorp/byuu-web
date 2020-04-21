debug_host := 127.0.0.1
wasm := true
debug := false
profile := false
synchro := true

.DEFAULT:
	$(MAKE) ${@} -C higan target=web \
		wasm=$(wasm) \
		synchro=$(synchro) \
		debug=$(debug) \
		profile=$(profile) \
		debug_host=$(debug_host)
