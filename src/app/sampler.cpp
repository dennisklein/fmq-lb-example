#include <app.hpp>
#include <FairMQChannel.h>
#include <fairmq/Device.h>
#include <fairmq/DeviceRunner.h>
#include <memory>

using namespace fair::mq;

namespace
{

// `sampler` hands out work messages to connected
// `worker` devices.
struct sampler : Device
{
  auto Init() -> void override {
    AddChannel("work", FairMQChannel("rep", "bind", "tcp://localhost:5555"));
  }

  auto InitTask() -> void override {
    OnData("work", [&](MessagePtr& /*msg*/, int) {
      auto msg(NewSimpleMessageFor("work", 0, *app::generate_work())); // TODO use proper serialization
      return Send(msg, "work", 0) >= 0;
    });
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
