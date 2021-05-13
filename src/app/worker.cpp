#include <app.hpp>
#include <cstring>
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
    AddChannel("work", FairMQChannel("req", "connect", "tcp://localhost:5555"));
  }

  auto ConditionalRun() -> bool override {
    auto request_work_msg(NewMessage());
    auto work_msg(NewMessage());
    if (Send(request_work_msg, "work") >= 0) {
      if (Receive(work_msg, "work") >= 0) {
        auto work(std::make_unique<app::work>());
        std::memcpy(static_cast<void*>(work.get()), work_msg->GetData(), sizeof(app::work)); // TODO use proper deserialization
        app::do_work(std::move(work));
        return true;
      }
    }
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
