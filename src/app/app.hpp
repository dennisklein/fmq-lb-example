#ifndef APP_HPP
#define APP_HPP

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <FairMQChannel.h>
#include <FairMQMessage.h>
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
// work messages. This is supposed to be a
// placeholder for the output of the processing stage
// before the `worker` stage (e.g. reading
// data from disk / receiving messages or similar).
auto generate_work() -> std::unique_ptr<work>
{
  static constexpr std::uint_fast32_t seed(42);
  static std::mt19937 gen(seed);
  static constexpr float shape(1.5);
  static constexpr float scale(0.9);
  static std::gamma_distribution<> gd(shape, scale);

  auto work(std::make_unique<work>());
  work->duration_in_ms = std::lround(gd(gen) * 100);
  // LOG(warn) << work->duration_in_ms;

  return work;
}

// Called in the `worker` to emulate a CPU-bound
// workload with a simple busy loop. Discards the work
// item at the end.
template<typename Deleter>
auto do_work(std::unique_ptr<work, Deleter> work) -> bool
{
  // LOG(warn) << work->duration_in_ms;
  using clock = std::chrono::high_resolution_clock;

  auto const start = clock::now();
  long long counter(0);
  int const check_clock_interval(500000);

  for(;;) {
    ++counter;
    if(   counter % check_clock_interval == 0
       && clock::now() - start > std::chrono::milliseconds(work->duration_in_ms)) {
      break;
    }
  }

  return true;
}

// Simple data serializer that just moves the C++ memory
// representation of the data object into the message
// content. It is zero-copy, but requires a POD data type
// and a compatible run-time environment on the msg
// receiver (e.g. on the same host)
//
// We move the (de)serialization logic here to make the actual
// device code more ...
// * ... readable by hiding all the ugly i/o code, and
// * ... maintainable by not duplicating the easy-to-screw-up code too much.
template<typename T>
auto serialize(FairMQChannel & ch, std::unique_ptr<T> data)
  -> std::unique_ptr<fair::mq::Message>
{
  return ch.NewMessage(static_cast<void*>(data.release()),
                       sizeof(T),
                       [](void* p, void*) { delete static_cast<T*>(p); },
                       nullptr);
}

template<typename T>
auto deserialize(std::unique_ptr<fair::mq::Message> msg)
{
  assert(msg->GetSize() >= sizeof(T));

  auto data_ptr(static_cast<T*>(msg->GetData()));
  auto deleter([m=std::move(msg)](T*) mutable { m.reset(nullptr); });
  auto data(std::unique_ptr<T, decltype(deleter)>(data_ptr, std::move(deleter)));

  return data;
}

} // namespace app

#endif // APP_HPP
