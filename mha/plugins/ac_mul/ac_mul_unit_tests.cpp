// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2021 HörTech gGmbH
// Copyright © 2021 Hörzentrum Oldenburg gGmbH
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
#include "ac_mul.hh"
#include "mha_algo_comm.hh"

TEST(enum_arg_type_t, test_values_differ_from_each_other) {
    ASSERT_NE(ARG_RR, ARG_RC);
    ASSERT_NE(ARG_RR, ARG_CR);
    ASSERT_NE(ARG_RR, ARG_CC);
    ASSERT_NE(ARG_RC, ARG_CR);
    ASSERT_NE(ARG_RC, ARG_CC);
    ASSERT_NE(ARG_CR, ARG_CC);
}

TEST(enum_val_type_t, test_values_differ_from_each_other) {
    ASSERT_NE(VAL_REAL, VAL_COMPLEX);
}

class test_ac_mul_t : public ::testing::Test {
public:
  // AC variable space
  MHAKernel::algo_comm_class_t acspace = {};
  // C handle to AC variable space
  algo_comm_t ac = {acspace.get_c_handle()};

  // example signal dimensions for the prepare method
  mhaconfig_t signal_properties {
    .channels = 1U,
    .domain = MHA_WAVEFORM,
    .fragsize = 10U,
    .wndlen = 10U,
    .fftlen = 18U,
    .srate = 44100.0f
  };
  // audio containers for calling process()
  MHASignal::waveform_t timesignal = {10,1};
  MHASignal::spectrum_t specsignal = {10,1};

  // name of the result ac variable
  const std::string resultname = {"resultname"};

  // Plugin instance
  ac_mul_t ac_mul = {ac, resultname};

  // some AC variables to use in tests
  MHA_AC::waveform_t a = {ac, "a", 32, 2, true};
  MHA_AC::waveform_t b = {ac, "b", 32, 2, true};
  MHA_AC::spectrum_t C = {ac, "C", 129, 2, true};
  MHA_AC::spectrum_t D = {ac, "D", 129, 2, true};
  // correct dimensions to multiply with a for mixed-mode multiplications
  MHA_AC::spectrum_t A = {ac, "A", 32, 2, true};

  // fills real ac matrices like this: .value(frame,ch) = frame + ch + offset
  void fill(MHA_AC::waveform_t & acvar, mha_real_t offset) {
    for (size_t ch = 0; ch < acvar.num_channels; ++ch)
      for (size_t frame = 0; frame < acvar.num_frames; ++frame)
        acvar.value(frame,ch) = frame + ch + offset;
  }
  // Checks if the result ac matrix contains products of elements generated
  // with fill
  void check_rr(mha_real_t offset1, mha_real_t offset2)
  {
    mha_wave_t acvar = MHA_AC::get_var_waveform(ac, resultname);
    for (size_t ch = 0; ch < acvar.num_channels; ++ch)
      for (size_t frame = 0; frame < acvar.num_frames; ++frame) {
        mha_real_t r1 = frame + ch + offset1; // first factor for channel,frame
        mha_real_t r2 = frame + ch + offset2; // second factor
        ASSERT_FLOAT_EQ(r1 * r2, value(acvar,frame,ch)) // check each element
        << "Unexpected product value at frame=" << frame << ",ch="
        << ch << ": Expected " << r1 << "*" << r2 << "=" << (r1*r2)
        << ". Actual " << value(acvar,frame,ch) << ".";
      }
  }
  // fills complex ac matrices like this:
  // .value(bin,ch) = bin + ch + offset - i
  void fill(MHA_AC::spectrum_t & acvar, mha_real_t offset) {
    for (size_t ch = 0; ch < acvar.num_channels; ++ch)
      for (size_t bin = 0; bin < acvar.num_frames; ++bin)
        set(acvar.value(bin,ch), bin + ch + offset, -1);
  }
  // Checks if an ac matrix contains products of elements generated with fill
  void check_cc(mha_real_t offset1, mha_real_t offset2)
  {
    mha_spec_t acvar = MHA_AC::get_var_spectrum(ac, resultname);
    for (size_t ch = 0; ch < acvar.num_channels; ++ch)
      for (size_t bin = 0; bin < acvar.num_frames; ++bin) {
        mha_complex_t c1 = {bin + ch + offset1, -1}; // first factor
        mha_complex_t c2 = {bin + ch + offset2, -1}; // second factor
        ASSERT_FLOAT_EQ((c1 * c2).re, value(acvar,bin,ch).re)
        << "Unexpected product value real part at bin=" << bin << ",ch="
        << ch << ": Expected " << (c1*c2).re << ". Actual "
        << value(acvar,bin,ch).re << ".";
        ASSERT_FLOAT_EQ((c1 * c2).im, value(acvar,bin,ch).im)
        << "Unexpected product value imaginary part at bin=" << bin << ",ch="
        << ch << ": Expected " << (c1*c2).im << ". Actual "
        << value(acvar,bin,ch).im << ".";
      }
  }
  void check_cr(mha_real_t offset1, mha_real_t offset2)
  {
    mha_spec_t acvar = MHA_AC::get_var_spectrum(ac, resultname);
    for (size_t ch = 0; ch < acvar.num_channels; ++ch)
      for (size_t bin = 0; bin < acvar.num_frames; ++bin) {
        mha_complex_t c1 = {bin + ch + offset1, -1}; // complex first factor
        mha_real_t    r2 = bin + ch + offset2;       // real second factor
        ASSERT_FLOAT_EQ(c1.re * r2, value(acvar,bin,ch).re)
        << "Unexpected product value real part at bin=" << bin << ",ch="
        << ch << ": Expected " << (c1.re*r2) << ". Actual "
        << value(acvar,bin,ch).re << ".";
        ASSERT_FLOAT_EQ(c1.im * r2, value(acvar,bin,ch).im)
        << "Unexpected product value imaginary part at bin=" << bin << ",ch="
        << ch << ": Expected " << (c1.im*r2) << ". Actual "
        << value(acvar,bin,ch).im << ".";
      }
  }
  void check_rc(mha_real_t offset1, mha_real_t offset2) {
      check_cr(offset2, offset1);
  }
};

TEST_F(test_ac_mul_t, test_state_after_constructor) {
    ASSERT_EQ("a * b", ac_mul.parse("?val"));
}

TEST_F(test_ac_mul_t, prepare_does_not_change_signal_dimensions) {
    mhaconfig_t copied_properties = signal_properties;
    ac_mul.prepare_(signal_properties);
    acspace.set_prepared(true);
    ASSERT_EQ(copied_properties.channels, signal_properties.channels);
    ASSERT_EQ(copied_properties.domain,   signal_properties.domain);
    ASSERT_EQ(copied_properties.fragsize, signal_properties.fragsize);
    ASSERT_EQ(copied_properties.wndlen,   signal_properties.wndlen);
    ASSERT_EQ(copied_properties.fftlen,   signal_properties.fftlen);
    ASSERT_EQ(copied_properties.srate,    signal_properties.srate);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, prepare_with_nonexisting_first_ac_variable_fails) {
    ac_mul.parse("= nonexisting_acvariable * a");
    ASSERT_THROW(ac_mul.prepare_(signal_properties), MHA_Error);
}

TEST_F(test_ac_mul_t, prepare_with_nonexisting_second_ac_variable_fails) {
    ac_mul.parse("= a * nonexisting_acvariable");
    ASSERT_THROW(ac_mul.prepare_(signal_properties), MHA_Error);
}

TEST_F(test_ac_mul_t, prepare_with_mismatching_ac_sizes_fails) {
    ac_mul.parse("= a * C");
    ASSERT_THROW(ac_mul.prepare_(signal_properties), MHA_Error);
}

TEST_F(test_ac_mul_t, prepare_with_invalid_expression_fails) {
    ac_mul.parse("= a"); // no multiplication!
    ASSERT_THROW(ac_mul.prepare_(signal_properties), MHA_Error);
}

TEST_F(test_ac_mul_t, prepare_real_real_succeeds) {
    ASSERT_NO_THROW(ac_mul.prepare_(signal_properties));
    acspace.set_prepared(true);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, prepare_real_squaring_succeeds) {
    ac_mul.parse("= a * a");
    ASSERT_NO_THROW(ac_mul.prepare_(signal_properties));
    acspace.set_prepared(true);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, prepare_real_complex_succeeds) {
    ac_mul.parse("= a * A");
    ASSERT_NO_THROW(ac_mul.prepare_(signal_properties));
    acspace.set_prepared(true);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, prepare_complex_real_succeeds) {
    ac_mul.parse("= A * a");
    ASSERT_NO_THROW(ac_mul.prepare_(signal_properties));
    acspace.set_prepared(true);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, prepare_complex_complex_succeeds) {
    ac_mul.parse("= C * D");
    ASSERT_NO_THROW(ac_mul.prepare_(signal_properties));
    acspace.set_prepared(true);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, prepare_complex_squaring_succeeds) {
    ac_mul.parse("= C * C");
    ASSERT_NO_THROW(ac_mul.prepare_(signal_properties));
    acspace.set_prepared(true);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, prepare_multiply_nonfloat_fails) {
    ac_mul.parse("= integer1 * integer2");
    MHA_AC::int_t integer1(ac, "integer1", 1);
    MHA_AC::int_t integer2(ac, "integer2", 2);
    ASSERT_THROW(ac_mul.prepare_(signal_properties), MHA_Error);
    acspace.set_prepared(true);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, process_real_real) {
    ac_mul.prepare_(signal_properties);
    acspace.set_prepared(true);
    fill(a,0);
    fill(b,-5);
    ac_mul.process(&timesignal);
    // check result
    check_rr(0,-5);
    // process() should fail when dimension of first parameter changes
    MHA_AC::waveform_t new_a(ac,"a",1,2,true);
    ASSERT_THROW(ac_mul.process(&timesignal), MHA_Error);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, process_real_squaring) {
    ac_mul.parse("= a * a");
    signal_properties.domain = MHA_SPECTRUM; // have one test using spec2spec
    ac_mul.prepare_(signal_properties);
    acspace.set_prepared(true);
    fill(a,-2);
    ac_mul.process(&specsignal);
    // check result
    check_rr(-2,-2);
    // process() should fail when dimensions of both parameters change
    MHA_AC::waveform_t new_a(ac,"a",10,2,true);
    ASSERT_THROW(ac_mul.process(&timesignal), MHA_Error);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, process_real_complex) {
    ac_mul.parse("= a * A");
    ac_mul.prepare_(signal_properties);
    acspace.set_prepared(true);
    fill(a,-2);
    fill(A,-4);
    ac_mul.process(&timesignal);
    // check result
    check_rc(-2,-4);
    // process() should fail when dimension of second parameter changes
    MHA_AC::spectrum_t new_A(ac,"A",129,1,true);
    ASSERT_THROW(ac_mul.process(&timesignal), MHA_Error);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, process_complex_real) {
    ac_mul.parse("= A * a");
    fill(A,2);
    fill(a,-0.5f);
    ac_mul.prepare_(signal_properties);
    acspace.set_prepared(true);
    ac_mul.process(&timesignal);
    // check result
    check_cr(2,-0.5f);
    // process() should fail when dimension of second parameter changes
    MHA_AC::waveform_t new_a(ac,"a",1,1,true);
    ASSERT_THROW(ac_mul.process(&timesignal), MHA_Error);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, process_complex_complex) {
    ac_mul.parse("= C * D");
    ac_mul.prepare_(signal_properties);
    acspace.set_prepared(true);
    fill(C, 0);
    fill(D, -5);
    ac_mul.process(&timesignal);
    // check result
    check_cc(0,-5);
    // process() should fail when domain of second parameter changes
    MHA_AC::waveform_t new_D(ac,"D",129,2,true);
    ASSERT_THROW(ac_mul.process(&timesignal), MHA_Error);
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, process_complex_squaring) {
    ac_mul.parse("= C * C");
    ac_mul.prepare_(signal_properties);
    acspace.set_prepared(true);
    fill(C, 0);
    ac_mul.process(&timesignal);
    // check result
    check_cc(0,0);
    // process() should fail when dimensions of both parameters change
    MHA_AC::spectrum_t new_C(ac,"C",129,1,true);
    ASSERT_THROW(ac_mul.process(&timesignal), MHA_Error);
    // process() should pick up new ACvar if dimension & type are correct
    MHA_AC::spectrum_t newest_C(ac,"C",129,2,true);
    fill(newest_C, -1);
    ASSERT_NO_THROW(ac_mul.process(&timesignal));
    check_cc(-1,-1); // check result
    acspace.set_prepared(false);
    ac_mul.release_();
}

TEST_F(test_ac_mul_t, process_reinserts_result_acvariable) {
    ac_mul.prepare_(signal_properties);
    acspace.set_prepared(true);
    fill(a,0);
    fill(b,-5);
    ac_mul.process(&timesignal);
    // check result
    check_rr(0,-5);

    // replace AC variable with different variable (mock a downstream plugin)
    MHA_AC::int_t fakeresultacvariable(ac,resultname,0);
    // check_result will now fail when trying to resolve resultname to matrix
    ASSERT_THROW(check_rr(0,-5), MHA_Error);
    // but the next process() invocation should fix it
    ac_mul.process(&timesignal);
    // check result
    check_rr(0,-5); // not using ASSERT_NO_THROW because check_rr ASSERTs itself
    acspace.set_prepared(false);
    ac_mul.release_();
}
