// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2023 2024 Hörzentrum Oldenburg gGmbH
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

#include <torch/script.h> // One-stop header.
#include "mha_plugin.hh"
#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <cmath>

class bmfwf_t : public MHAPlugin::plugin_t<int>
{
  MHAParser::string_t model_file;
  /** Keep Track of the prepare/release calls */
  MHAParser::int_mon_t prepared;
  MHAParser::float_t scaling_factor;
  MHAParser::float_t unscaling_ratio;
  MHAParser::float_t mix_back;
  MHAParser::float_t calib_factor;

public:
  bmfwf_t(MHA_AC::algo_comm_t &iac, const std::string &configured_name);

  /** Plugin preparation.
   * @param signal_info
   *   Structure containing a description of the form of the signal
   *   (domain, number of channels, frames per block, sampling rate.
   */
  void prepare(mhaconfig_t &signal_info);
  void release(void);

  /** Signal processing performed by the plugin.
   * @param signal
   *   Pointer to the input signal structure.
   * @return
   *   Returns a pointer to the input signal structure,
   *   with a the signal modified by this plugin.
   *   (In-place processing)
   */
  mha_spec_t *process(mha_spec_t *signal);

private:
  void load_model(); // Declare the load_model method
  int64_t frame_index;
  float_t unscaling_factor;
  float_t calib_factor_lin;
  torch::Tensor noisy_frame;
  torch::Tensor noisy_frame_real;
  torch::Tensor noisy_frame_imag;
  torch::Tensor output_tensor;
  torch::jit::script::Module model;
  MHASignal::spectrum_t *out;
};

bmfwf_t::bmfwf_t(MHA_AC::algo_comm_t &iac, const std::string &)
    : MHAPlugin::plugin_t<int>("This plugin implements a deep neural network"
                               " based multi-frame Wiener filter",
                               iac),
      model_file("Path to the model file exported using torchscript", "model.pt"),
      prepared("State of this plugin: 0 = unprepared, 1 = prepared"),
      scaling_factor("The scaling factor that is applied to the input of the"
                     " model to account for different scaling of MHA and"
                     " PyTorch STFT.",
                     "1.0"),
      unscaling_ratio("The ratio between the time domain volume before and after processing.",
                      "1.0"),
      mix_back("The fraction of the noisy signal to be mixed back into the output.",
               "0.02"),
      calib_factor("The factor in dB to be applied to the output frame"
                   " (within the Torchscript model)",
                   "0.3")
{
  insert_item("model_file", &model_file);
  prepared.data = 0;
  insert_item("prepared", &prepared);
  insert_item("scaling_factor", &scaling_factor);
  insert_item("unscaling_ratio", &unscaling_ratio);
  insert_item("mix_back", &mix_back);
  insert_item("calib_factor", &calib_factor);
}

void bmfwf_t::prepare(mhaconfig_t &signal_info)
{
  if (signal_info.domain != MHA_SPECTRUM)
    throw MHA_Error(__FILE__, __LINE__,
                    "This plugin can only process spectrum signals.");
  if (signal_info.fftlen != 128)
    throw MHA_Error(__FILE__, __LINE__,
                    "This plugin requires an FFT length of 128, got %u.",
                    signal_info.fftlen);
  if (signal_info.channels != 4)
    throw MHA_Error(__FILE__, __LINE__,
                    "This plugin requires four input channels, got %u.",
                    signal_info.channels);
  // 2 output channels
  signal_info.channels = 2;

  c10::InferenceMode guard;
  load_model();
  unscaling_factor = unscaling_ratio.data / scaling_factor.data;
  calib_factor_lin = std::pow(10, calib_factor.data / 20.);
  frame_index = 0;
  noisy_frame = torch::zeros({1, 4, 65}, torch::kComplexFloat);
  noisy_frame_real = torch::zeros({1, 4, 65}, torch::kFloat32);
  noisy_frame_imag = torch::zeros({1, 4, 65}, torch::kFloat32);
  output_tensor = torch::zeros({1, 2, 65, 2}, torch::kFloat32);
  out = new MHASignal::spectrum_t(65, 2);
  prepared.data = 1;
}

void bmfwf_t::load_model()
{
  try
  {
    c10::InferenceMode guard;
    model = torch::jit::load(model_file.data);
  }
  catch (const std::exception &e)
  {
    throw MHA_Error(__FILE__, __LINE__, "Failed to load the model: %s", e.what());
  }
}

void bmfwf_t::release(void)
{
  frame_index = 0;
  delete out;
  out = 0;
  prepared.data = 0;
}

mha_spec_t *bmfwf_t::process(mha_spec_t *signal)
{
  c10::InferenceMode guard;
  // num_frames corresponds to number of frequency bins
  out->num_frames = signal->num_frames;
  out->num_channels = 2;

  const unsigned int num_channels = signal->num_channels;
  const unsigned int num_bins = signal->num_frames;
  const unsigned int total_elements = num_channels * num_bins;

  std::vector<float> real_data(total_elements);
  std::vector<float> imag_data(total_elements);

  for (unsigned int i = 0; i < total_elements; i++)
  {
    real_data[i] = signal->buf[i].re * scaling_factor.data;
    imag_data[i] = signal->buf[i].im * scaling_factor.data;
  }
  noisy_frame = torch::complex(torch::from_blob(real_data.data(), {1, num_channels, num_bins}),
                               torch::from_blob(imag_data.data(), {1, num_channels, num_bins}));

  output_tensor = model.forward({frame_index++, noisy_frame, mix_back.data, calib_factor_lin}).toTensor();

  if (output_tensor.sizes() == std::vector<int64_t>({1, 2, 65, 2}))
  {
    for (unsigned int channel = 0; channel < 2; channel++)
    {
      auto output_tensor_channel = output_tensor[0][channel];
      for (unsigned int bin = 0; bin < signal->num_frames; bin++)
      {
        auto output_tensor_channel_bin = output_tensor_channel[bin];
        auto &out_buf = out->buf[channel * signal->num_frames + bin];
        out_buf.re = output_tensor_channel_bin[0].item<float>() * unscaling_factor;
        out_buf.im = output_tensor_channel_bin[1].item<float>() * unscaling_factor;
      }
    }
  }

  else
  {
    std::cerr << "Error: output tensor is not a FloatTensor of size (1, 2, 65, 2)! "
              << "Actual size: (" << output_tensor.sizes() << "), "
              << "Actual data type: " << output_tensor.dtype().name() << "\n";
  }

  return out;
}

MHAPLUGIN_CALLBACKS(bmfwf, bmfwf_t, spec, spec)
MHAPLUGIN_DOCUMENTATION(bmfwf, "DNN-based spatial filter",
  "Implements a deep neural network (DNN)-based"
  " multi-frame Wiener filter (MFWF).\n"
  "\n"
  "The provided DNN model"
  " (examples/34-DNN-bases-speech-enhancement/model\\_bmfwf.pt)"
  " is trained to extract a single target speaker from a frontal cone"
  " within $ \\pm 10^{\\circ} $.\n"
  "\n"
  "The DNN architecture is based on Wang et al., “Neural Speech Enhancement"
  " with Very Low Algorithmic Latency and Complexity via Integrated full-"
  " and sub-band Modeling”, in Proc. IEEE International Conference on"
  " Acoustics, Speech and Signal Processing (ICASSP) 2023."
  " It was modified to decrease computational complexity.\n"
  "\n"
  "The MFWF implementation is similar to Wang et al., “TF-GridNet: Integrating"
  " Full- and Sub-Band Modeling for Speech Separation”, in"
  " IEEE/ACM Transactions on Audio, Speech, and Language Processing, vol. 31,"
  " 2023, Eq. (14). It was modified for online processing using recursive"
  " smoothing.\n"
  "\n"
  "This plugin processes short time fourier transform signal in four audio"
  " channels with audio sampling rate 16kHz, FFT length 128, Hanning window"
  " length 64 and hop size 32 samples."
  " Limiting the number of threads can be beneficial for runtime performance,"
  " e.g., using OMP\\_NUM\\_THREADS=1."
)
