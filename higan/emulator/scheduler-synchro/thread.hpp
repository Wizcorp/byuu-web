
struct Thread;

struct Scheduler;

struct thread_handle_t {
  Thread* thread;
  thread_handle_t* parent = nullptr;
};

struct Thread {
  enum : uintmax { Second = (uintmax)-1 >> 1 };
  enum : uintmax { Size = 8_KiB * sizeof(void*) };

  higan::Event event = higan::Event::None;
  std::string name;

  Thread() = default;
  Thread(const Thread&) = delete;
  auto operator=(const Thread&) = delete;
  inline virtual ~Thread();

  virtual auto main() -> void = 0;
  inline explicit operator bool() const { return _handle; }
  inline auto active() const -> bool;
  inline auto handle() const -> thread_handle_t*;
  inline auto frequency() const -> uintmax;
  inline auto scalar() const -> uintmax;
  inline auto clock() const -> uintmax;

  inline auto setHandle(thread_handle_t *handle) -> void;
  inline auto setFrequency(double frequency) -> void;
  inline auto setScalar(uintmax scalar) -> void;
  inline auto setClock(uintmax clock) -> void;

  inline auto create(double frequency, function<void ()> entryPoint) -> void;
  inline auto destroy() -> void;

  inline auto step(uint clocks) -> void;
  inline auto synchronize() -> void;
  template<typename... P> inline auto synchronize(Thread&, P&&...) -> void;

  inline auto serialize(serializer& s) -> void;

protected:
  thread_handle_t *_handle = nullptr;
  uint _uniqueID = 0;
  uintmax _frequency = 0;
  uintmax _scalar = 0;
  uintmax _clock = 0;

  friend class Scheduler;
};
