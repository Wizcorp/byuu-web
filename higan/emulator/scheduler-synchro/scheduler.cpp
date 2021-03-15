auto Scheduler::reset() -> void {
  _threads.reset();
}

auto Scheduler::threads() const -> uint {
  return _threads.size();
}

auto Scheduler::thread(uint uniqueID) const -> maybe<Thread&> {
  for(auto& thread : _threads) {
    if(thread->_uniqueID == uniqueID) return *thread;
  }
  return {};
}

//if threads A and B both have a clock value of 0, it is ambiguous which should run first.
//to resolve this, a uniqueID is assigned to each thread when appended to the scheduler.
//the first unused ID is selected, to avoid the uniqueID growing in an unbounded fashion.
auto Scheduler::uniqueID() const -> uint {
  uint uniqueID = 0;
  while(thread(uniqueID)) uniqueID++;
  return uniqueID;
}

//find the clock time of the furthest behind thread.
auto Scheduler::minimum() const -> uintmax {
  uintmax minimum = (uintmax)-1;
  for(auto& thread : _threads) {
    minimum = min(minimum, thread->_clock - thread->_uniqueID);
  }
  return minimum;
}

//find the clock time of the furthest ahead thread.
auto Scheduler::maximum() const -> uintmax {
  uintmax maximum = 0;
  for(auto& thread : _threads) {
    maximum = max(maximum, thread->_clock - thread->_uniqueID);
  }
  return maximum;
}

auto Scheduler::append(Thread& thread) -> bool {
  if(_threads.find(&thread)) return false;
  thread._uniqueID = uniqueID();
  thread._clock = maximum() + thread._uniqueID;
  _threads.append(&thread);
  return true;
}

auto Scheduler::remove(Thread& thread) -> void {
  _threads.removeByValue(&thread);
}

auto Scheduler::run() -> void {
  _primary->main();
}

//power cycle and soft reset events: assigns the primary thread and resets all thread clocks.
auto Scheduler::power(Thread& thread) -> void {
  _primary = &thread;

  for(auto& thread : _threads) {
    thread->_clock = thread->_uniqueID;
  }
}

auto Scheduler::enter(Mode mode) -> Event {
  if(mode == Mode::Run) {
    _mode = mode;
    
    while(_primary->event ==  Event::None) {
        _primary->main();
    }
    
    return exit(_primary->event);
  }

  // todo: this is probably incorrect since it relies on calling
  // exit, which will try to switch coros
  if(mode == Mode::Synchronize) {
    // run all threads to safe points, starting with the primary thread.
    _mode = Mode::SynchronizePrimary;
    
    _primary->main();
    _primary->event = Event::None;

    for(auto& thread : _threads) {
      if(thread != _primary) {
        _mode = Mode::SynchronizeAuxiliary;
        thread->main();
      }
    }
    return Event::Synchronize;
  }

  return Event::None;
}

auto Scheduler::exit(Event event) -> Event {
  //subtract the minimum time from all threads to prevent clock overflow.
  auto reduce = minimum();
  for(auto& thread : _threads) {
    thread->_clock -= reduce;
  }

  //return to the thread that entered the scheduler originally.
  _event = event;
  _primary->event = Event::None;
  platform->event(event);

  return event;
}

//used to prevent auxiliary threads from blocking during synchronization.
//for instance, a secondary CPU waiting on an interrupt from the primary CPU.
//as other threads are not run during synchronization, this would otherwise cause a deadlock.
auto Scheduler::synchronizing() const -> bool {
  return _mode == Mode::SynchronizeAuxiliary;
}

//marks a safe point (typically the beginning of the entry point) of a thread.
//the scheduler may exit at these points for the purpose of synchronization.
auto Scheduler::synchronize() -> void {
  if(_mode == Mode::SynchronizePrimary) _primary->event = Event::Synchronize;
}

auto Scheduler::getSynchronize() -> bool {
  return _synchronize;
}

auto Scheduler::setSynchronize(bool synchronize) -> void {
  _synchronize = synchronize;
}
