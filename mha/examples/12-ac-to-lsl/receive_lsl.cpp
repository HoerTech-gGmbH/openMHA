#include "lsl_cpp.h"
#include <iostream>
#include <unistd.h>
#include <memory>
/**
 * This is a minimal example that demonstrates how a multi-channel
 * stream can be resolved into an inlet, and how the
 * raw sample data & time stamps are pulled from the inlet.
 */

int main(int argc, char* argv[]) {
  using namespace lsl;

  std::string stream_name;

  //first argument is stream name
  if(argc==2){
    stream_name=argv[1];
  }

  // resolve the stream of interest
  // and create an inlet to get data from the first result
  std::vector<stream_info> results = resolve_streams();
  int ntries=1;
  while(!results.size()){
    if(++ntries>5){
      std::cout<<"Found no streams after 5 tries, exiting."<<std::endl;
      return 1;
    }
    std::cout<<"Found no streams, retrying in 10 seconds...\n";
    usleep(1e7);
    results = resolve_streams();
  }
  //look for requested stream, choose first stream if none given
  int stream_nb=0;
  std::cout<<"Found "<<results.size()<<" stream(s): ";
  for(unsigned ii=0;ii<results.size();ii++){
    std::cout<<results[ii].name()<<" ";
    if(results[ii].name()==stream_name)
      stream_nb=ii;
  }
  std::cout<<'\n';
  stream_inlet inlet(results[stream_nb]);
  // receive data & time stamps forever
  int sz=inlet.info().channel_count();
  auto sample=std::vector<float>(sz);
  while (true){
    float ts = inlet.pull_sample(sample.data(),sz);
    for(int ii=0;ii<sz;ii++)
      std::cout<<sample[ii]<<" ";
    std::cout<<std::endl;
  }
  return 0;
}
