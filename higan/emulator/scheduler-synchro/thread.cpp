Thread::~Thread() {
  destroy();
}

auto Thread::active() const -> bool { return _active; }
auto Thread::handle() const -> thread_handle_t* { return _handle; }
auto Thread::frequency() const -> uintmax { return _frequency; }
auto Thread::scalar() const -> uintmax { return _scalar; }
auto Thread::clock() const -> uintmax { return _clock; }

auto Thread::setHandle(thread_handle_t *handle) -> void {
  _handle = handle;
}

auto Thread::setFrequency(double frequency) -> void {
  _frequency = frequency + 0.5;
  _scalar = Second / _frequency;
}

auto Thread::setScalar(uintmax scalar) -> void {
  _scalar = scalar;
}

auto Thread::setClock(uintmax clock) -> void {
  _clock = clock;
}

auto Thread::create(double frequency, function<void ()> entryPoint) -> void {
  if(!_handle) {
    _handle = (thread_handle_t *) malloc(sizeof(thread_handle_t));
    _handle->thread = this;
  }

  setFrequency(frequency);
  setClock(0);
  scheduler.append(*this);
}

auto Thread::destroy() -> void {
  scheduler.remove(*this);
  if(_handle) free(_handle);
  _handle = nullptr;
}

auto Thread::step(uint clocks) -> void {
  _clock += _scalar * clocks;
}

//ensure all threads are caught up to the current thread before proceeding.
auto Thread::synchronize() -> void {
  //note: this will call Thread::synchronize(*this) at some point, but this is safe:
  //the comparison will always fail as the current thread can never be behind itself.
  for(auto thread : scheduler._threads) synchronize(*thread);
}

//ensure the specified thread(s) are caught up the current thread before proceeding.
template<typename... P>
auto Thread::synchronize(Thread& thread, P&&... p) -> void {
  // yield back
  if (_handle->parent != nullptr && _handle->parent == thread._handle) {
    if constexpr(sizeof...(p) > 0) synchronize(forward<P>(p)...);
    return;
  }

  //switching to another thread does not guarantee it will catch up before switching back.
  thread_handle_t *_parent = thread._handle->parent;
  thread._handle->parent = _handle;
  while(thread.clock() < clock()) {
    //disable synchronization for auxiliary threads during scheduler synchronization.
    //synchronization can begin inside of this while loop.
    if(scheduler.synchronizing()) break;
    thread._active = true;
    thread.main();
    thread._active = false;
  }
  thread._handle->parent = _parent;

  //convenience: allow synchronizing multiple threads with one function call.
  if constexpr(sizeof...(p) > 0) synchronize(forward<P>(p)...);
}

auto Thread::serialize(serializer& s) -> void {
  s.integer(_frequency);
  s.integer(_scalar);
  s.integer(_clock);

  if(!scheduler._synchronize) {
    static uint8_t stack[Thread::Size];
    bool resume = true; // todo

    if(s.mode() == serializer::Size) {
      s.array(stack, Thread::Size);
      s.boolean(resume);
    }

    if(s.mode() == serializer::Load) {
      s.array(stack, Thread::Size);
      s.boolean(resume);
      memory::copy(_handle, stack, Thread::Size);
    }

    if(s.mode() == serializer::Save) {
      memory::copy(stack, _handle, Thread::Size);
      s.array(stack, Thread::Size);
      s.boolean(resume);
    }
  }
}
