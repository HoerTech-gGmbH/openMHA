#include "lsl_cpp.h"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

/**
 * This is a minimal lsl example that demonstrates how a multi-channel
 * stream can be used to push data to the network via stream outlet.
 * It creates a stream outlet, waits until there are one or more consumers
 * and then sends random data to the stream.
 */

// hard-coded number of samples ('channels' in lsl parlance) to be send per push_sample
const int nchannels = 480;

int main(int argc, char* argv[]) {

  // create a new stream_info and open an outlet with it - Take name from cmd line or use
  // "SimpleStream" as default
  lsl::stream_info info(argc > 1 ? argv[1] : "SimpleStream", "Audio", nchannels);
  lsl::stream_outlet outlet(info);

  // Wait until there is a consumer before continuing - timeout is 999s
  outlet.wait_for_consumers(999);

  // allocate buffer for data
  float sample[nchannels];
  // send data as long as there are consumers
  while(outlet.have_consumers()) {
    // Fill the buffer with random data
    for (int c=0;c<nchannels;c++) sample[c] = (rand()%100)/100.0f-0.5;
    // send it
    outlet.push_sample(sample);
    // sleep for 10 ms to limit rate to 100 Hz to get a similar frame rate as the receiving end
    // the real rate will be slightly below, as no compensation is done for simplicity reasons,
    // we can expect occasional dropouts in this example.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return 0;
}
