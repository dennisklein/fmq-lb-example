#include <app.hpp>
#include <cassert>
#include <FairMQChannel.h>
#include <fairmq/Device.h>
#include <fairmq/DeviceRunner.h>
#include <memory>

using namespace fair::mq;

namespace
{

// `worker` requests work at `sampler` with empty messages
struct worker : Device
{
  auto Init() -> void override {
    this->AddChannel("work", FairMQChannel("req", "connect", "tcp://localhost:5555"));
  }

  auto ConditionalRun() -> bool override {
    using namespace app;

    auto & ch(this->fChannels.at("work").at(0)); // channel to receive and send on

    auto work_request_msg(ch.NewMessage());
    if (ch.Send(work_request_msg) == 0) {
      auto work_msg(ch.NewMessage());
      if (ch.Receive(work_msg) > 0) {
        return do_work(deserialize<work>(std::move(work_msg)));
      }
    }

    // for some future FairMQ version we plan to support:
    //
    // auto & ch(fChannels.at("work").at(0)); // channel to receive and send on
    //
    // if (ch.Send(ch.NewMessage()) == 0) {
    //   return do_work(deserialize<work>(ch.Receive()));
    // }

    return false;
  }
};

} // namespace

auto main(int argc, char* argv[]) -> int
{
  DeviceRunner runner(argc, argv);
  runner.AddHook<hooks::InstantiateDevice>([](DeviceRunner& r){
    r.fDevice = std::make_unique<worker>();
  });
  return runner.RunWithExceptionHandlers();
}
