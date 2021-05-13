#ifndef APP_HPP
#define APP_HPP

#include <chrono>
#include <cstdint>
#include <cmath>
#include <fairlogger/Logger.h>
#include <memory>
#include <random>

// Emulate the application domain
namespace app {

// Message sent from `sampler` to `worker`.
// Contains the duration in milliseconds the
// worker shall simulate a CPU-bound workload.
struct work
{
  long duration_in_ms{500};
};

// Called in the `sampler` to generate new
// work messages. This is supposed to
// emulate the output of the processing stage
// before the `worker` stage (e.g. reading
// data from disk / receiving messages or similar).
auto generate_work() -> std::unique_ptr<work>
{
  static constexpr std::uint_fast32_t seed(42);
  static std::mt19937 gen(seed);
  static constexpr float shape(1.5);
  static constexpr float scale(0.9);
  static std::gamma_distribution<> gd(shape, scale);

  // allocate on heap because we want to emulate much
  // larger data message
  auto work(std::make_unique<work>());
  work->duration_in_ms = std::lround(gd(gen) * 100);
  // LOG(warn) << work->duration_in_ms;

  return work;
}

// Called in the `worker` to emulate a CPU-bound
// workload with a simple busy loop. Discards the work
// message at the end.
auto do_work(std::unique_ptr<work> work) -> void
{
  // LOG(warn) << work->duration_in_ms;
  using clock = std::chrono::high_resolution_clock;

  auto const start = clock::now();
  long long counter(1);
  static constexpr int check_clock_interval(500000);

  for(;;) {
    ++counter;
    if(   counter % check_clock_interval == 0
       && clock::now() - start > std::chrono::milliseconds(work->duration_in_ms)) {
      break;
    }
  }
}

} // namespace app

#endif // APP_HPP
