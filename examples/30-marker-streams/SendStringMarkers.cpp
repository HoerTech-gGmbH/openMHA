#include <iostream>
#include <lsl_cpp.h>
#include <random>
#include <thread>


/**
 * This example program offers a 1-channel stream which contains strings.
 * The stream has the "Marker" content type and irregular rate.
 * The name of the stream can be chosen as a startup parameter.
 */

int main(int argc, char *argv[]) {
  try {
    const char *name = argc > 1 ? argv[1] : "MyEventStream";
    // make a new stream_info
    lsl::stream_info info(name, "Markers", 1, lsl::IRREGULAR_RATE, lsl::cf_string, "id23443");

    // make a new outlet
    lsl::stream_outlet outlet(info);

    // send random marker strings
    std::cout << "Now sending markers... " << std::endl;
    std::vector<std::string> markertypes{
      "Test", "Blah", "Marker", "XYZ", "Testtest", "Test-1-2-3"};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> rnd(0, markertypes.size() - 1);
    std::uniform_int_distribution<int> delayrnd(0, 1000);
    while (true) {
      // wait for a 20ms
      std::this_thread::sleep_for(std::chrono::milliseconds(delayrnd(gen)));
      // and choose the marker to send
      std::string mrk = markertypes[rnd(gen)];
      std::cout << "now sending: " << mrk << std::endl;

      // now send it (note the &)
      outlet.push_sample(&mrk);
    }
  } catch (std::exception &e) { std::cerr << "Got an exception: " << e.what() << std::endl; }
  std::cout << "Press any key to exit. " << std::endl;
  std::cin.get();
  return 0;
}
