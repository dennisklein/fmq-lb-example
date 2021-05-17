#include <app.hpp>
#include <cassert>
#include <FairMQChannel.h>
#include <fairmq/Device.h>
#include <fairmq/DeviceRunner.h>
#include <memory>

using namespace fair::mq;

namespace
{

// `sampler` hands out work messages to connected `worker` devices.
struct sampler : Device
{
  auto Init() -> void override {
    this->AddChannel("work", FairMQChannel("rep", "bind", "tcp://localhost:5555"));
  }

  auto ConditionalRun() -> bool override {
    using namespace app;

    auto & ch(this->fChannels.at("work").at(0)); // channel to receive and send on

    auto work_request_msg(ch.NewMessage());
    if (ch.Receive(work_request_msg) == 0) {
      auto work_msg(serialize(ch, generate_work()));
      return ch.Send(work_msg) >= 0;
    }

    // for some future FairMQ version we plan to support:
    //
    // if (ch.Receive()->GetSize() == 0) {
    //   return ch.Send(serialize(ch, generate_work())) >= sizeof(work);
    // }

    return false;
  }
};

} // namespace

auto main(int argc, char* argv[]) -> int
{
  DeviceRunner runner(argc, argv);
  runner.AddHook<hooks::InstantiateDevice>([](DeviceRunner& r){
    r.fDevice = std::make_unique<sampler>();
  });
  return runner.RunWithExceptionHandlers();
}
