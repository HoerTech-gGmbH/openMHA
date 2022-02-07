// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2011 2013 2015 2016 2017 2018 HörTech gGmbH
// Copyright © 2019 2020 HörTech gGmbH
// Copyright © 2021 2022 Hörzentrum Oldenburg gGmbH
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

#ifndef MHA_ALGO_COMM_HH
#define MHA_ALGO_COMM_HH

/**
  \ingroup mhatoolbox
  \ingroup algocomm
  \file   mha_algo_comm.hh
  \brief  Header file for Algorithm Communication

  Functions and classes for Algorithm Communication (AC) support.
*/


#include "mha.hh"
#include "mha_signal.hh"
#include "mha_toolbox.h"
#include <vector>
#include <string>
#include <map>

#define ALGO_COMM_ID_STR "MFVK3jL5rmeus1XtggEI971aXCR/GU7RRehKz4kQtrg="

extern algo_comm_t algo_comm_default;

namespace MHA_AC {

    /** 
        \ingroup algocomm

        \brief Convert an AC variable into a spectrum
        
        This function reads an AC variable and tries to convert it into a
        valid spectrum. The spectrum variable is only valid during
        the current call of the plugin's process() method and should not be
        stored for later reuse.
    
        The stride of the AC variable is used as the number of spectral bins
        per channel.  The complex values of the spectrum are not copied, the
        \c buf pointer of the returned spectrum points to the original memory
        of the AC variable.
        \param ac AC handle
        \param name Name of the variable
        \return Spectrum structure
    */
    mha_spec_t get_var_spectrum(algo_comm_t ac,const std::string& name);

    /** 
        \ingroup algocomm

        \brief Convert an AC variable into a waveform
        
        This function reads an AC variable and tries to convert it into a
        valid block of waveform signal. The waveform variable only valid during
        the current call of the plugin's process() method and should not be
        stored for later reuse.
    
        The stride of the AC variable is used as the number of audio channels.
        The single-precision floating-point sample values are not copied, the
        \c buf pointer of the returned waveform points to the original memory
        of the AC variable.
        \param ac AC handle
        \param name Name of the variable
        \return waveform structure
    */
    mha_wave_t get_var_waveform(algo_comm_t ac,const std::string& name);

    /**
       \ingroup algocomm
       
       \brief Return value of an integer scalar AC variable
       
       \param ac AC handle
       \param name Name of the variable
       \return Variable value
     */
    int get_var_int(algo_comm_t ac,const std::string& name);

    /**
       \ingroup algocomm

       \brief Return value of an floating point scalar AC variable

       \param ac AC handle
       \param name Name of the variable
       \return Variable value
     */
    float get_var_float(algo_comm_t ac,const std::string& name);
    
    /**
       \ingroup algocomm

       \brief Return value of an floating point vector AC variable as standard vector of floats.

        Because this function allocates memory for the return value, it
        should not be called during signal processing, but only from the plugin
        prepare() method.
       \param ac AC handle
       \param name Name of the variable
       \return Variable value
     */
    std::vector<float> get_var_vfloat(algo_comm_t ac,const std::string& name);
    
    /**
       \ingroup algocomm
       Convenience class for inserting a spectrum into the AC space.

       In MHA, spectra are stored non-interleaved: First all bins of the first
       channel are stored, then all bins of the second channel, etc.
       
       The stride of the AC variable is set to the number of stored bins, which
       is equal to floor(fftlen/2)+1 in MHA (negative frequency bins are not
       stored).
    */
    class spectrum_t : public MHASignal::spectrum_t {
    public:
        /** \brief Initialize memory and metadata of the AC variable.
            All spectral bins are initially set to 0.
            \param ac AC handle
            \param name Name of variable in AC space
            \param bins Number of FFT bins per channel in the spectrum_t class
            \param channels Number of audio channels in the spectrum_t class
            \param insert_now If true, then the constructor inserts the new
                              variable into the AC space, and the
                              destructor will remove the variable from AC space
                              when it executes. */
        spectrum_t(algo_comm_t ac,
                   const std::string & name,
                   unsigned int bins,
                   unsigned int channels,
                   bool insert_now);
        /** Destroy the AC variable: deallocate its memory.
         * If the constructor parameter insert_now was true, then the destruc-
         * tor removes the AC variable from AC space when it executes. */
        ~spectrum_t();
        /** Insert or re-insert AC variable into AC space. Plugins should call
         * this method from their prepare() and process() functions. */
        void insert();
        /** Remove the AC variable by reference from the AC variable space.
         * Plugins may call this method only from their prepare(), release()
         * methods or their plugin destructor.  It is not necessary to remove
         * the AC variable from AC space at all if either another AC variable
         * with the same name has replaced this variable before this variable
         * is destroyed, or if no plugin will access this variable between its
         * destruction and either its replacement or the MHA exit. */
        void remove();
    protected:
        /** AC variable space. */
        const algo_comm_t ac;
        /** Name of this AC variable in the AC variable space. */
        const std::string name;
        /** flag whether to remove from AC variable space in destructor. */
        const bool remove_during_destructor;
    };

    /**
       \ingroup algocomm
       Convenience class for inserting a waveform
       (a block of time-domain audio signal) into the AC space.

       In MHA, waveforms are stored interleaved: The first sample of the first
       is followed by the first samples of all other channels before the second
       sample of the first sample is stored, etc.
       
       The stride of the AC variable is set to the number of audio channels.
    */
    class waveform_t : public MHASignal::waveform_t {
    public:
        /** \brief Initialize memory and metadata of the AC variable.
            All audio samples are initially set to 0.
            \param ac AC handle
            \param name Name of variable in AC space
            \param frames Number of samples per channel in the waveform_t class
            \param channels Number of audio channels in the waveform_t class
            \param insert_now If true, then the constructor inserts the new
                              variable into the AC space, and the
                              destructor will remove the variable from AC space
                              when it executes. */
        waveform_t(algo_comm_t ac,
                   const std::string & name,
                   unsigned int frames,
                   unsigned int channels,
                   bool insert_now);
        /** Destroy the AC variable: deallocate its memory.
         * If the constructor parameter insert_now was true, then the destruc-
         * tor removes the AC variable from AC space when it executes. */
        ~waveform_t();
        /** Insert or re-insert AC variable into AC space. Plugins should call
         * this method from their prepare() and process() functions. */
        void insert();
        /** Remove the AC variable by reference from the AC variable space.
         * Plugins may call this method only from their prepare(), release()
         * methods or their plugin destructor.  It is not necessary to remove
         * the AC variable from AC space at all if either another AC variable
         * with the same name has replaced this variable before this variable
         * is destroyed, or if no plugin will access this variable between its
         * destruction and either its replacement or the MHA exit. */
        void remove();
    protected:
        /** AC variable space. */
        const algo_comm_t ac;
        /** Name of this AC variable in the AC variable space. */
        const std::string name;
        /** flag whether to remove from AC variable space in destructor. */
        const bool remove_during_destructor;
    };

    class stat_t : public MHASignal::stat_t {
    public:
        stat_t(algo_comm_t ac,const std::string& name,
               const unsigned int& frames, const unsigned int& channels,
               bool insert_now);
        void update();
        void insert();
    private:
        MHA_AC::waveform_t mean;
        MHA_AC::waveform_t std;
    };

    class ac2matrix_helper_t
    {
    public:
        ac2matrix_helper_t(algo_comm_t,const std::string&);
        void getvar();
        algo_comm_t ac;
        std::string name;
        std::string username;
        MHASignal::uint_vector_t size;
        bool is_complex;
    protected:
        comm_var_t acvar;
    };

    /**
       \ingroup algocomm

       \brief Copy AC variable to a matrix

       This class constructs a matrix of same size as an AC variable
       and can copy the AC variable to itself. The update() function
       is real-time safe.
    */
    class ac2matrix_t : private MHA_AC::ac2matrix_helper_t, public MHASignal::matrix_t
    {
    public:
        /**
           \brief Constructor
           
           \param ac AC handle
           \param name Name of AC variable to be copied
        */
        ac2matrix_t(algo_comm_t ac,const std::string& name);
        /**
           \brief Update contents of the matrix from the AC space.

           This function is real-time safe. The copy operation
           performance is of the order of the number of elements in
           the matrix.
        */
        void update();
        /**
           \brief Return name of AC variable/matrix.
         */
        const std::string& getname() const {return name;};
        /**
           \brief Return user specified name of AC variable/matrix.
         */
        const std::string& getusername() const {return username;};
        /**
           \brief Insert matrix into an AC space (other than source AC space)
           \param ac AC space handle to insert data
           \note The AC variable data buffer points to the data of the matrix. Modifications of the AC variable directly modify the data of the matrix; after deletion of the matrix, the data buffer is invalid.
         */
        void insert(algo_comm_t ac);
    };

    /**
       \ingroup algocomm
       \brief Copy all or a subset of all numeric AC variables into an array of matrixes.
    */
    class acspace2matrix_t 
    {
    public:
        /**
           \brief Constructor.

           Scan all given AC variables and allocate corresponding matrixes.

           \param ac AC handle.
           \param names Names of AC variables, or empty for all.
        */
        acspace2matrix_t(algo_comm_t ac,const std::vector<std::string>& names);

        /**
           \brief Constructor with initialization from an instance.

           \param src Instance to be copied.
        */
        acspace2matrix_t(const MHA_AC::acspace2matrix_t& src);
        ~acspace2matrix_t();
        /**
           \brief Copy all contents (deep copy).

           \param src Array of matrixes to be copied.
        */
        MHA_AC::acspace2matrix_t& operator=(const MHA_AC::acspace2matrix_t& src);
        /**
           \brief Access operator.

           \param k index into array; should not exceed size()-1.
           \retval Reference to matrix.
        */
        MHA_AC::ac2matrix_t& operator[](unsigned int k) {return *(data[k]);};

        /**
           \brief Constant access operator.

           \param k index into array; should not exceed size()-1.
           \retval Constant reference to matrix.
        */
        const MHA_AC::ac2matrix_t& operator[](unsigned int k) const {return *(data[k]);};
        /**
           \brief Update function.

           This function updates all matrixes from their corresponding
           AC variables. It can be called from the MHA Framework
           prepare function or in the processing callback.
        */
        void update() {for(unsigned int k=0;k<len;k++) data[k]->update(); frameno++;};
        /** 
            \brief Number of matrixes in AC space
        */
        unsigned int size() const {return len;};
        /**
           \brief Actual frame number.
        */
        unsigned int frame() const {return frameno;};
        /**
           \brief Insert AC space copy into an AC space (other than source AC space)
           \param ac AC space handle to insert data
         */
        void insert(algo_comm_t ac);
    private:
        unsigned int len;
        MHA_AC::ac2matrix_t** data;
        unsigned int frameno;
    };
}

namespace MHAKernel {

    /** Storage class for the AC variable space.  Uses an std::map for
     * associating AC variable names with AC variable metadata (\c comm_var_t).
     * Acts as a delegator for the std::map storage.  Allows operations that
     * may require memory allocations/deallocations only when
     * is_prepared == false. */
    class comm_var_map_t {
        /** The std::map used for organizing the AC space */
        std::map<std::string, comm_var_t> map;

        /// A list containing the names of all AC variables.
        std::vector<std::string> entries;

        /* In order to avoid complicated size types, assert that the map's
         * size_type is the same as size_t. */
        static_assert(std::is_same<std::map<std::string,comm_var_t>::size_type,
                                   size_t>::value);

        /** Update the member variable \ref entries because an AC variable has
         * been inserted or removed. Only permitted if is_prepared == false. */
        void update_entries();
    public:
        /** is_prepared stores whether the provider of the AC space has entered
         * MHA state "prepared" or not.  Operations on \c map that require
         * memory allocations or deallocations are only allowed when not
         * prepared.  Needs to be set by the containing \n algo_comm_class_t AC
         * space instance. */
        bool is_prepared = {false};

        /** Query the map if some AC variable name is present in the AC space.
         * @param name Name of AC variable to check.
         * @return true if the variable is present in the AC space.
         * @return false if no variable with this name exists in the AC space.
         */
        bool has_key(const std::string & name) const
        {return map.find(name) != map.end();}

        /** Create or replace variable.  Creating is only permitted if
         * is_prepared == false.
         * @param name Name of the AC variable to create or update.
         *             May not be empty.  Must not contain space character.
         * @param var  Metadata of the AC variable.
         * @throw MHA_Error if asked to create in prepared state.
         * @throw MHA_Error if name is emtpy or contains space. */
        void insert(const std::string & name, const comm_var_t & var);

        /** Remove variable. Only permitted if is_prepared == false.
         * @param name Name of the AC variable to remove.
         * @throw MHA_Error if called while prepared. */
        void erase_by_name(const std::string & name);

        /** Find variables that point to the given address. Erase all that
         * are found. It is not an error if no variable points there.  Only
         * permitted if is_prepared == false. 
         * @ptr Pointer to memory where the variables data is stored.
         * @throw MHA_Error if called while prepared. */
        void erase_by_pointer(void * ptr);

        /** Get the comm_var_t of an existing variable.
         * @param name The name of the AC variable.
         * @throw MHA_Error if no such variable exists in the AC space. */
        const comm_var_t & retrieve(const std::string & name) const;

        /** @return A list of names of all AC variables in this AC space. */
        const std::vector<std::string> & get_entries() const;

        /** @return number of stored AC variables */
        size_t size() const {return map.size();}
    };

    /** AC variable space. */
    class algo_comm_class_t {
    public:

        /// AC variable space constructor
        algo_comm_class_t();

        /// AC variable space destructor
        virtual
        ~algo_comm_class_t();

        /// Generates the client handle for users for this AC space.
        virtual algo_comm_t get_c_handle();

        /** Interacts with AC space storage to create or replace an AC variable
         * 
         * When the AC space is already prepared, only replacing existing
         * variables is permitted, not creating new ones.  An AC space becomes
         * prepared only after the plugin's prepare() method has finished
         * executing, and becomes unprepared again before the plugin's
         * release() method starts executing.  During signal processing, which
         * starts after all plugins have executed their prepare() methods and
         * terminates before any plugin executes its release() method, the
         * AC space stays in prepared state.
         * 
         * Plugins calling this method must ensure that it is called directly
         * or indirectly from every single invocation of their prepare() and
         * their process() methods for each AC variable that they choose to
         * publish.  During prepare(), plugins must decide which AC variables
         * to publish and stick to this decision until the next invocation of
         * release().
         * 
         * @param name Name of the AC variable to create or to replace.
         *             May not be empty.  Must not contain space character.
         * @param cv Descriptor of AC variable.  The \c data pointer of this
         *           struct must remain valid until at least the next
         *           invocation of the calling plugin's process() or release()
         *           method, the other fields must correctly describe the data.
         * @throw MHA_Error If the AC space is already prepared and no AC
         *                  variable with name \c name exists yet.
         * @throw MHA_Error if name is emtpy or contains space. */
        virtual
        void insert_var(const std::string & name, comm_var_t cv);

        /** Convenience method for inserting or replacing a scalar integer AC
         * variable into AC space.  Creates suitable comm_var_t and forwards to
         * insert_var(), therefore see also the documentation of insert_var().
         * When the AC space is already prepared, only replacing existing
         * variables is permitted, not creating new ones.
         * @param name Name of the AC variable to create or to replace.
         *             May not be empty.  Must not contain space character.
         * @param ptr Pointer to an int variable owned by the calling plugin.
         *            The pointer must remain valid until at least the next
         *            invocation of the calling plugin's process() or release()
         *            method.
         * @throw MHA_Error If the AC space is already prepared and no AC
         *                  variable with name \ref name exists yet.
         * @throw MHA_Error if name is emtpy or contains space. */
        virtual void insert_var_int(const std::string & name, int * ptr);

        /** Convenience function for inserting or replacing a vector of floats
         * as an AC variable into the AC space.  Creates suitable comm_var_t
         * and forwards to insert_var(), therefore see also the documentation
         * of insert_var().
         * When the AC space is prepared, only replacing existing variables
         * is permitted, not creating new ones.
         * @param name Name of the AC variable to create or to replace.
         *             May not be empty.  Must not contain space character.
         * @param vec Reference to a float vector owned by the calling plugin.
         *            The internal storage of this vector must remain valid
         *            until at least the next invocation of the calling
         *            plugin's process() or release() method.
         *            No methods that could cause iterator invalidation may be
         *            called on this vector until at least then.
         * @throw MHA_Error If the AC space is already prepared and no AC
         *                  variable with name \c name exists yet.
         * @throw MHA_Error if name is emtpy or contains space.
         * @throw MHA_Error if vec contains more elements than can be
         *                  represented by comm_var_t::num_entries. */
        virtual void
        insert_var_vfloat(const std::string & name, std::vector<float>& vec);

        /** Convenience method for inserting or replacing a scalar float AC
         * variable into AC space.  Creates suitable comm_var_t and forwards to
         * insert_var(), therefore see also the documentation of insert_var().
         * When the AC space is prepared, only replacing existing variables
         * is permitted, not creating new ones.
         * @param name Name of the AC variable to create or to replace.
         *             May not be empty.  Must not contain space character.
         * @param ptr Pointer to a float variable owned by the calling plugin.
         *            The pointer must remain valid until at least the next
         *            invocation of the calling plugin's process() or release()
         *            method.
         * @throw MHA_Error If the AC space is already prepared and no AC
         *                  variable with name \c name exists yet.
         * @throw MHA_Error if name is emtpy or contains space. */
        virtual void insert_var_float(const std::string & name, float* ptr);

        /** Convenience method for inserting or replacing a scalar double AC
         * variable into AC space.  Creates suitable comm_var_t and forwards to
         * insert_var(), therefore see also the documentation of insert_var().
         * When the AC space is prepared, only replacing existing variables
         * is permitted, not creating new ones.
         * @param name Name of the AC variable to create or to replace.
         *             May not be empty.  Must not contain space character.
         * @param ptr Pointer to a double variable owned by the calling plugin.
         *            The pointer must remain valid until at least the next
         *            invocation of the calling plugin's process() or release()
         *            method.
         * @throw MHA_Error If the AC space is already prepared and no AC
         *                  variable with name \c name exists yet.
         * @throw MHA_Error if name is emtpy or contains space. */
        virtual void insert_var_double(const std::string & name, double* ptr);

        /** Remove an AC variable from AC space by name.  Only
         * permitted when AC space is not prepared.  Trying to remove a
         * non-existing AC variable from AC space is not by itself an error.
         * Calling this method while the AC space is prepared is an error,
         * because it is not permitted to remove AC variables during signal
         * processing, only to update them.
         * @param name Name of the AC variable to remove.
         * @throw MHA_Error if called while prepared, and then regardless of
         *        whether an AC variable with name \c name exists or not. */
        virtual void remove_var(const std::string & name);

        /** Remove all AC variables from AC space that point to the given
         * memory address.  Only permitted when AC space is not prepared.
         * While not prepared, it is not an error if no AC variables or if
         * multiple AC variables actually point to addr.  All matching
         * variables are removed.
         * @param addr Memory address where the data of the AC variable(s) to
         *             remove is or was stored.
         * @throw MHA_Error if called while prepared, regardless whether any
         *                  AC variables currently point to addr or not. */
        virtual void remove_ref(void* addr);

        /** Interacts with AC space storage to check if an AC variable with
         * the given name exists.
         * @param name Name of the AC variable to check. */
        virtual
        bool is_var(const std::string & name) const;


        /** Interacts with AC space storage to retrieve the metadata for an
         * AC variable with the given name.
         * @param name Name of the AC variable to retrieve. 
         * @return a struct describing the AC variable's data type, memory
         *         location and size.
         * @throw MHA_Error if no AC variable with the given name exists. */
        virtual
        comm_var_t get_var(const std::string & name) const;

        /** Convenience method for retrieving a scalar integer AC
         * variable from AC space.  Checks data type and size. 
         * @param name Name of the AC variable to read.
         * @return Value of the AC varible.
         * @throw MHA_Error if no AC variable with the given name exists.
         * @throw MHA_Error if AC variable \c name is not an integer or not a
         *                  scalar. */
        virtual int get_var_int(const std::string & name) const;

        /** Convenience method for retrieving a scalar float AC
         * variable from AC space.  Checks data type and size. 
         * @param name Name of the AC variable to read.
         * @return Value of the AC varible.
         * @throw MHA_Error if no AC variable with the given name exists.
         * @throw MHA_Error if AC variable \c name is not a float or not a
         *                  scalar. */
        virtual float get_var_float(const std::string & name) const;

         /** Convenience method for retrieving a scalar double AC
         * variable from AC space.  Checks data type and size. 
         * @param name Name of the AC variable to read.
         * @return Value of the AC varible.
         * @throw MHA_Error if no AC variable with the given name exists.
         * @throw MHA_Error if AC variable \c name is not a double or not a
         *                  scalar. */
        virtual double get_var_double(const std::string & name) const;

        /** @return a list of the names of all existing AC variables. */
        virtual
        const std::vector<std::string> & get_entries() const;

        /** Interacts with AC space storage to return the number of AC
         * variables currently stored in the AC space.  Always permitted. */
        virtual
        size_t size() const;

        /** The provider of this AC space must set the AC space to prepared at
         * the end of its own prepare() operation and to not prepared at the
         * beginning of its own release() operation. */
        virtual void set_prepared(bool prepared);

    private:
        algo_comm_t ac;
        comm_var_map_t vars;
    };
}

namespace MHA_AC {
    /**
       \ingroup algocomm
       Template for convenience classes for inserting a numeric scalar
       into the AC space.
    */
    template <typename numeric_t, unsigned int MHA_AC_TYPECODE> 
    class scalar_t {
    public:
        /** \brief Initialize memory and metadata of the AC variable.
            \param ac AC handle
            \param name Name of variable in AC space
            \param val Initial value
            \param insert_now If true, then the constructor inserts the new
                              variable into the AC space, and the
                              destructor will remove the variable from AC space
                              when it executes. */
        scalar_t(algo_comm_t ac,
                 const std::string & name,
                 numeric_t val = 0,
                 bool insert_now = true)
            : data(val),
              ac(ac),
              name(name),
              remove_during_destructor(insert_now)
        {
            if (insert_now)
                insert();
        }
        /** Destroy the AC variable: deallocate its memory.
         * If the constructor parameter insert_now was true, then the destruc-
         * tor removes the AC variable from AC space when it executes. */
        ~scalar_t()
        {
            if (remove_during_destructor) {
                try {
                    remove();
                }
                catch (...) {
                    // ignore all exceptions because we are in destructor
                }
            }
        }
        numeric_t data; //!< Numeric value of this AC variable.
        /** Insert or re-insert AC variable into AC space. Plugins should call
         * this method from their prepare() and process() functions. */
        void insert()
        {
            comm_var_t acv;
            memset(&acv,0,sizeof(acv));
            acv.data_type = MHA_AC_TYPECODE;
            acv.num_entries = 1;
            acv.stride = 1;
            acv.data = &data;
            ac.handle->insert_var(name, acv);
        }
        /** Remove the AC variable by reference from the AC variable space.
         * Plugins may call this method only from their prepare(), release()
         * methods or their plugin destructor.  It is not necessary to remove
         * the AC variable from AC space at all if either another AC variable
         * with the same name has replaced this variable before this variable
         * is destroyed, or if no plugin will access this variable between its
         * destruction and either its replacement or the MHA exit. */
        void remove()
        {
            ac.handle->remove_ref(&data);
        }
    private:
        /** AC variable space. */
        const algo_comm_t ac;
        /** Name of this AC variable in the AC variable space. */
        const std::string name;
        /** flag whether to remove from AC variable space in destructor. */
        const bool remove_during_destructor;
    };
    /// Convenience class for inserting an integer variable into the AC space.
    typedef scalar_t<int, MHA_AC_INT> int_t;
    /// Convenience class for inserting a single-precision floating-point
    /// variable into the AC space.
    typedef scalar_t<float, MHA_AC_FLOAT> float_t;
    /// Convenience class for inserting a double-precision floating-point
    /// variable into the AC space.
    typedef scalar_t<double, MHA_AC_DOUBLE> double_t;
}

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
