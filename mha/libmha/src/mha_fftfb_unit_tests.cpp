// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 HörTech gGmbH
//
// openMHA is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// openMHA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License, version 3 for more details.
//
// You should have received a copy of the GNU Affero General Public License, 
// version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

#include <gtest/gtest.h>
#include "mha_fftfb.hh"

TEST(fspacing_t, error_message_on_edge_frequency_0_in_log_mode)
{
  // Regression test: Having an edge frequency of 0Hz in logarithmic
  // edge frequency mode has previously caused an error message
  // "(mha_fftfb) Fatal error in implementation!" without further
  // explanation.  This test checks that the error message has improved.
  
  // A MHA parser for adding the filterbank parameter variables to
  MHAParser::parser_t parser;

  // Instantiate the object to manage the filterbank paramter variables
  MHAOvlFilter::fftfb_vars_t parameters(parser);

  // set the variables that caused the problem
  //  parser.parse("fftlen=8192");
  parser.parse("ftype=edge");
  parser.parse("fscale=log");
  parser.parse("f=[0 165.8 353.6 612.4 866.0 1095.4 1341.6 1596.9 1889.4"
               " 2336.7 2839.0 3386.7 4125.5 5075.4 6305.6 7993.7 12000]");

  std::string err = "";
  try {
    // compute the filterbank from the parameters
    unsigned nfft=8192; unsigned srate = 44100;
    MHAOvlFilter::fftfb_t filterbank(parameters,nfft,srate);
  } catch(MHA_Error & e) {
    err = e.get_msg();
  }
  // We expect an explanation in the error message that contains all of the
  // words "logarithmic", "0", and "edge", but does not contain any of the
  // words "Fatal", "error", or "implementation"
  mha_debug("%s\n", err.c_str());
  auto contains = [&](auto word){return err.find(word) != std::string::npos;};
  EXPECT_TRUE(contains("logarithmic"));
  EXPECT_TRUE(contains("0"));
  EXPECT_TRUE(contains("edge"));
  EXPECT_FALSE(contains("Fatal"));
  EXPECT_FALSE(contains("error"));
  EXPECT_FALSE(contains("implementation"));
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
