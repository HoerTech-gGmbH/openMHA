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

#include "types.h"

#include "mha_plugin.hh"
#include "mha_os.h"

#include <memory>
#include <string>

/** Namespace where all classes of the matlab wrapper plugin live*/
namespace matlab_wrapper {

/** Thin wrapper around the emxArray containing the user defined configuration variables.
 * This wrapper holds a copy of the user configuration which is used by the process callback.
 * On write access to a variable, the user configuration is changed and a new copy is created and
 * exchanged in a real time safe manner.
 */
class matlab_wrapper_rt_cfg_t {
public:
  /** Ctor of the real time configuration class. Creates a local copy of the user config
   * @param user_config_ Pointer to the array containing the user config
   */
  explicit matlab_wrapper_rt_cfg_t(emxArray_user_config_t* user_config_);
  /** Dtor. Calls the appropriate c-style destructors of the emx_ types */
  ~matlab_wrapper_rt_cfg_t();
  /** User configuration to be used in the process callback */
  emxArray_user_config_t* user_config;
};

class matlab_wrapper_t;
/** Utility class connecting a user_config_t instance to its corresponding configuration parser. Provides the callback.
 * Every time the a user defined configuration parser is changed, this class makes sure that the corresponding 'matlab-side'
 * struct is also changed and a new real time config is created and pushed.
 */
class callback{
public:
  /** Ctor
   * @param parent_ Ptr to the parent parser. Used to signal finished change. 
   * @param user_config_ Ptr to user_config_t struct holding the 'matlab side' of the configuration variable
   * @param var_ Ptr to the configuration parser corresponding to user_config_
   */
  callback(matlab_wrapper_t* parent_, user_config_t* user_config_, MHAParser::mfloat_t* var_);
  /** Write access callback. To be called on every writeaccess of *var. Synchronises *var and *user_config. */
  void on_writeaccess();
private:
  /** Ptr to user_config_t */
  user_config_t* user_config;
  /** Ptr to parser */
  MHAParser::mfloat_t* var;
  /** Ptr to parent plugin */
  matlab_wrapper_t* parent;
};

/** Matlab wraper plugin interface class */
class matlab_wrapper_t : public MHAPlugin::plugin_t<matlab_wrapper_rt_cfg_t>
{
  friend callback;
  /** Wrapper class around the matlab-generated library */
  class wrapped_plugin_t {
  public:
    wrapped_plugin_t()=delete;
    /** Ctor. Opens matlab-generated library and resolves functions
     * @param name_ file name of shared library
     */
    explicit wrapped_plugin_t(const char* name_);
    /** Dtor. */
    ~wrapped_plugin_t();
    /** Process callback. Convert mha-type mha_wave_t to matlab array and
     * calls wrapped process callback
     * @param s input/output signal
     * @param user_config_ Ptr to user configuration array
     */
    mha_wave_t* process(mha_wave_t* s,emxArray_user_config_t* user_config_);
    /** Prepare callback. Calls wrapped prepare function if necessary and determines output signal dimensions */
    void prepare(mhaconfig_t& config);
    /** Release callback. Cleans up io arrays and calls wrapped release if necessary. */
    void release();
    /** Ptr to user config array */
    emxArray_user_config_t* user_config=nullptr;
    private:
    /** Handle to matlab generated shared library */
    dynamiclib_t library_handle;
    /** Handle to matlab generated cleanup function */
    void (*fcn_terminate)()=nullptr;
    /** Handle to matlab generated init function */
    void (*fcn_init)(emxArray_user_config_t*)=nullptr;
    /** Handle to matlab generated process function */
    void (*fcn_process)(const emxArray_real_T *, const signal_dimensions_t *,
                        emxArray_user_config_t*, emxArray_real_T *)=nullptr;
    /** Handle to matlab generated prepare function */
    void (*fcn_prepare)(signal_dimensions_t *,
                        emxArray_user_config_t *)=nullptr;
    void (*fcn_release)()=nullptr;
    /** Signal dimensions. Matlab-equivalent to mha_config_t */
    signal_dimensions_t signal_dimensions{0,'U',0,0,0,0};
    /** Ptr to emxArray holding the input signal */
    emxArray_real_T* wave_in=nullptr;
    /** Ptr to emxArray holding the output signal */
    emxArray_real_T* wave_out=nullptr;
    /** MHA waveform holding the output signal */
    std::unique_ptr<MHASignal::waveform_t> mha_wave_out;
  };

public:
  /** Ctor of the plugin interface class
   * @param iac Handle to ac space
   */
  matlab_wrapper_t(const algo_comm_t& iac,const std::string&,const std::string&);
  /** Process callback for wave to wave processing */
  mha_wave_t* process(mha_wave_t*);
  /** Process callback for spec to spec processing currently unused */
  mha_spec_t* process(mha_spec_t*);
  /** Prepare callback
   * @param signal_info struct holding the input/output signal dimensions
   */
  void prepare(mhaconfig_t& signal_info);
  /** Release callback */
  void release();
private:
  /** Configuration variable holding the file name of the matlab generated library */
  MHAParser::string_t library_name{"Name of matlab generated library",""};
  /** Patchbay for the interface plugins */
  MHAEvents::patchbay_t<matlab_wrapper_t> patchbay;
  /** Patchbay for the custom callbacks. Can use normal patchbay bc/ of the interface of patchbay_t */
  MHAEvents::patchbay_t<callback> cb_patchbay;
  /** Unique ptr holding the instance of the plugin wrapper class */
  std::unique_ptr<wrapped_plugin_t> plug;
  /** Create new library wrapper and load library. To be called write access to library_name. */
  void load_lib();
  /** Create new real time safe user config from user config. Called by the custom callbacks.*/
  void update();
  /** Vector holding the user defined configuration variables */
  std::vector<MHAParser::mfloat_t> vars;
  /** Vector holding the callbacks for the user defined variables' write access */
  std::vector<callback> callbacks;
};
}
