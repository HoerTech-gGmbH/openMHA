// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 HörTech gGmbH
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

/**
* \mainpage Overview
* The H&ouml;rTech Open Master Hearing Aid (\mha), is a development and evaluation 
* software platform that is able to execute hearing aid signal processing in real-time
* on standard computing hardware with a low delay between sound input and output.
* \section str Structure
* The openMHA can be split into four major components :
* 	- \ref mhascript "The openMHA command line application (MHA)"
* 	- \ref plugif "Signal processing plugins"
* 	- Audio input-output (IO) plugins (see io_file_t, MHAIOJack, io_parser_t, io_tcp_parser_t)
* 	- \ref mhatoolbox "The openMHA toolbox library"
* 	.
* 
* \image html structure_openmha.png
* \image latex structure_openmha.pdf "openMHA structure" width=0.4\textwidth
*
* \ref mhascript "The openMHA command line application (MHA)" acts as a plugin host.
* It can load signal processing plugins as well as audio input-output
* (IO) plugins.
* Additionally, it provides the command line configuration interface and
* a TCP/IP based configuration interface.
* Several IO plugins exist:
* For real-time signal processing, commonly the \mha MHAIOJack plugin (see plugins' manual) is
* used, which provides an interface to the Jack Audio Connection Kit (JACK).
* Other IO plugins provide audio file access or TCP/IP-based processing.
*
* \ref plugif "\mha plugins" provide the audio signal processing capabilities and
* audio signal handling.
* Typically, one openMHA plugin implements one specific algorithm.
* The complete virtual hearing aid signal processing can be achieved by
* a combination of several openMHA plugins. 
*
* \section pltf Platform Services and Conventions
*
* The openMHA platform offers some services and conventions to
* algorithms implemented in plugins, that make it especially well suited
* to develop hearing aid algorithms, while still supporting
* general-purpose signal processing.
*
* \subsection asd Audio Signal Domains
* 
* As in most other plugin hosts, the audio signal in the openMHA is
* processed in audio chunks.
* However, plugins are not restricted to propagate audio signal as
* blocks of audio samples in the time domain
* another option is to propagate the audio signal in the short
* time Fourier transform (STFT) domain, i.e. as spectra of blocks of
* audio signal, so that not every plugin has to perform its own STFT
* analysis and synthesis.
* Since STFT analysis and re-synthesis of acceptable audio quality always
* introduces an algorithmic delay, sharing STFT data is a necessity for
* a hearing aid signal processing platform, because the overall delay of
* the complete processing has to be as short as possible.
*
* Similar to some other platforms, the openMHA allows also arbitrary data to be
* exchanged between plugins through a mechanism called \ref algocomm
* "algorithm communication variables"
* or short
* "AC vars".
* This mechanism is commonly used to share data such as filter
* coefficients or filter states.
*
* \subsection rtscc Real-Time Safe Complex Configuration Changes
* 
* Hearing aid algorithms in the openMHA can export configuration
* settings that may be changed by the user at run time.
*
* To ensure real-time safe signal processing, the audio processing will
* normally be done in a signal processing thread with real-time
* priority, while user interaction with configuration parameters would
* be performed in a configuration thread with normal priority, so that
* the audio processing does not get interrupted by configuration tasks.
* Two types of problems may occur when the user is changing parameters
* in such a setup:
* - The change of a simple parameter exposed to the user may cause
*   an involved recalculation of internal runtime parameters that the
*   algorithm actually uses in processing.
*   The duration required to perform this recalculation may be a
*   significant portion of (or take even longer than) the time available
*   to process one block of audio signal.
*   In hearing aid usage, it is not acceptable to halt audio
*   processing for the duration that the recalculation may require.
* - If the user needs to change multiple parameters to reach a
*   desired configuration state of an algorithm from the original
*   configuration state, then it may not be acceptable that processing
*   is performed while some of the parameters have already been changed
*   while others still retain their original values. It is also not
*   acceptable to interrupt signal processing until all pending
*   configuration changes have been performed.
* .
*
* The openMHA provides a mechanism in its toolbox library to enable
* real-time safe configuration changes in openMHA plugins:
* 
* Basically, existing runtime configurations are used in the processing
* thread until the work of creating an updated runtime configuration has
* been completed in the configuration thread.
* 
* In hearing aids, it is more acceptable to continue to use an outdated
* configuration for a few more milliseconds than blocking all processing.
* 
* The openMHA toolbox library provides an easy-to-use mechanism to
* integrate real-time safe runtime configuration updates into every
* plugin.
*
* \subsection bridge Plugins can Themselves Host Other Plugins
*
* An openMHA plugin can itself act as a plugin host.
* This allows to combine analysis and re-synthesis methods in a single
* plugin.
* We call plugins that can themselves load other plugins ``bridge
* plugins'' in the openMHA.
*
* When such a bridge plugin is then called by the
* openMHA to process one block of signal, it will first perform its
* analysis, then invoke (as a function call) the signal processing in
* the loaded plugin to process the block of signal in the analysis
* domain, wait to receive a processed block of signal in the analysis
* domain back from the loaded plugin when the signal processing function
* call to that plugin returns, then perform the re-synthesis transform,
* and finally return the block of processed signal in the original
* domain back to the caller of the bridge plugin. 
*
* \subsection clb Central Calibration
* 
* The purpose of hearing aid signal processing is to enhance the sound
* for hearing impaired listeners.
* Hearing impairment generally means that people suffering from it have
* increased hearing thresholds, i.e. soft sounds that are audible for
* normal hearing listeners may be imperceptible for hearing impaired
* listeners.
* To provide accurate signal enhancement for hearing impaired people,
* hearing aid signal processing algorithms have to be able to determine
* the absolute physical sound pressure level corresponding to a digital
* signal given to any openMHA plugin for processing.
* Inside the openMHA, we achieve this with the following convention:
* The single-precision floating point time-domain sound signal samples, that are processed
* inside the openMHA plugins in blocks of short durations, have the physical
* pressure unit Pascal (\f$1 \mathrm{Pa} = 1 \mathrm{N} / \mathrm{m}^2\f$).
* With this convention in place, all plugins can determine the
* absolute physical sound pressure level from the sound samples that
* they process.
* A derived convention is employed in the spectral domain for STFT signals.
* Due to the dependency of the calibration on the hardware used, it is the responsibility 
* of the user of the openMHA to perform calibration
* measurements and adapt the openMHA settings to make sure that this
* calibration convention is met.
* We provide the plugin \c transducers which can be configured to perform the
* necessary signal adjustments.
*
*
*/

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
