## FairMQ standalone example for a load-balancing topology

Useful patterns in message passing applications are solutions to variations of the
classic producer-consumer problem. This example demonstrates how to build a FairMQ
topology with a single producer of CPU-bound *work* called `sampler` and multiple parallel
consumers of this *work* called `worker`s. The goal is to keep all `worker`s as busy
as possible.

### Topology

```
                            ┌────────┐
                    ┌──► REQ│ worker │
                    │       └────────┘
                    │
                    │       ┌────────┐
                    ├──► REQ│ worker │
 ┌─────────┐        │       └────────┘
 │ sampler │REP ◄───┤
 └─────────┘        │       ┌────────┐
                    ├──► REQ│ worker │
                    │       └────────┘
                    │
                    │
                    └──► REQ   ...
```

Each `worker` connects with a **REQ** channel to the **REP** channel of the `sampler`. Whenever
a `worker` has no work it sends an empty msg to out on its **REQ** channel. On reception of
such an empty work request msg the `sampler` generates an appropriate work reply msg.

### Installation

```
git clone https://github.com/dennisklein/fmq-lb-example
cmake -S fmq-lb-example -B fmq-lb-example-build -DCMAKE_INSTALL_PREFIX=fmq-lb-example-install
cmake --build fmq-lb-example-build --target install -j4
```

### WORK-IN-PROGRESS

More docs and an extensive example for a controller built with the APIs provided in the
FairMQ SDK and DDS is ongoing work.
