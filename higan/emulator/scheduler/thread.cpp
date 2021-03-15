#if defined(SCHEDULER_SYNCHRO) && !defined(SIMULATED_SYNCHRO) 
#include "../scheduler-synchro/thread.cpp"
#else
auto Thread::EntryPoints() -> vector<EntryPoint>& {
  static vector<EntryPoint> entryPoints;
  return entryPoints;
}

auto Thread::Enter() -> void {
  for(uint64_t index : range(EntryPoints().size())) {
    if(co_active() == EntryPoints()[index].handle) {
      auto entryPoint = EntryPoints()[index].entryPoint;
      EntryPoints().remove(index);
      while(true) {
        scheduler.synchronize();
        entryPoint();
      }
    }
  }
  struct ThreadNotFound{};
  throw ThreadNotFound{};
}

Thread::~Thread() {
  destroy();
}

auto Thread::active() const -> bool { return co_active() == _handle; }
auto Thread::handle() const -> cothread_t { return _handle; }
auto Thread::frequency() const -> uintmax { return _frequency; }
auto Thread::scalar() const -> uintmax { return _scalar; }
auto Thread::clock() const -> uintmax { return _clock; }

auto Thread::setHandle(cothread_t handle) -> void {
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
    _handle = co_create(Thread::Size, &Thread::Enter);
  } else {
    co_derive(_handle, Thread::Size, &Thread::Enter);
  }
  EntryPoints().append({_handle, entryPoint});
  setFrequency(frequency);
  setClock(0);
  scheduler.append(*this);
}

auto Thread::destroy() -> void {
  scheduler.remove(*this);
  if(_handle) co_delete(_handle);
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
  //switching to another thread does not guarantee it will catch up before switching back.
  while(thread.clock() < clock()) {
    //disable synchronization for auxiliary threads during scheduler synchronization.
    //synchronization can begin inside of this while loop.
    if(scheduler.synchronizing()) break;
    co_switch(thread.handle());
  }
  //convenience: allow synchronizing multiple threads with one function call.
  if constexpr(sizeof...(p) > 0) synchronize(forward<P>(p)...);
}

auto Thread::serialize(serializer& s) -> void {
  s.integer(_frequency);
  s.integer(_scalar);
  s.integer(_clock);

  if(!scheduler._synchronize) {
    static uint8_t stack[Thread::Size];
    bool resume = co_active() == _handle;

    if(s.mode() == serializer::Size) {
      s.array(stack, Thread::Size);
      s.boolean(resume);
    }

    if(s.mode() == serializer::Load) {
      s.array(stack, Thread::Size);
      s.boolean(resume);
      memory::copy(_handle, stack, Thread::Size);
      if(resume) scheduler._resume = _handle;
    }

    if(s.mode() == serializer::Save) {
      memory::copy(stack, _handle, Thread::Size);
      s.array(stack, Thread::Size);
      s.boolean(resume);
    }
  }
}
#endif