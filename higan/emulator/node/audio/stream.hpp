struct Stream : Object {
  DeclareClass(Stream, "Stream")
  using Object::Object;

  inline auto channels() const -> uint { return _channels.size(); }
  inline auto frequency() const -> float { return _frequency; }
  inline auto resamplerFrequency() const -> float { return _resamplerFrequency; }

  auto setChannels(uint channels) -> void;
  auto setFrequency(float frequency) -> void;
  auto setResamplerFrequency(float resamplerFrequency) -> void;

  auto resetFilters() -> void;
  auto addLowPassFilter(float cutoffFrequency, uint order, uint passes = 1) -> void;
  auto addHighPassFilter(float cutoffFrequency, uint order, uint passes = 1) -> void;
  auto addLowShelfFilter(float cutoffFrequency, uint order, float gain, float slope) -> void;
  auto addHighShelfFilter(float cutoffFrequency, uint order, float gain, float slope) -> void;

  auto pending() const -> bool;
  auto read(float samples[]) -> uint;
  auto write(const float samples[]) -> void;

  template<typename... P>
  inline auto sample(P&&... p) -> void {
    if(runAhead()) return;
    float samples[sizeof...(p)] = {forward<P>(p)...};
    write(samples);
  }

protected:
  struct Filter {
    enum class Mode : uint { OnePole, Biquad } mode;
    enum class Type : uint { None, LowPass, HighPass, LowShelf, HighShelf } type;
    enum class Order : uint { None, First, Second } order;
    DSP::IIR::OnePole onePole;
    DSP::IIR::Biquad biquad;
  };
  struct Channel {
    vector<Filter> filters;
    vector<DSP::IIR::Biquad> nyquist;
    DSP::Resampler::Cubic resampler;
  };
  vector<Channel> _channels;
  float _frequency = 48000.0;
  float _resamplerFrequency = 48000.0;
};
