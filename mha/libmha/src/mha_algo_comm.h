// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2011 2013 2015 2016 2017 HörTech gGmbH
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
  \ingroup mhatoolbox
  \ingroup algocomm
  \file   mha_algo_comm.h
  \brief  Header file for Algorithm Communication

*/


#ifndef MHA_ALGO_COMM_H
#define MHA_ALGO_COMM_H

#include "mha.h"
#include "mha_signal.hh"
#include <vector>
#include <string>

#ifdef __cplusplus

/**
   \ingroup algocomm

   \brief Functions and classes for Algorithm Communication (AC) support 

*/
namespace MHA_AC {

    /** 
        \ingroup algocomm

        \brief Convert an AC variable into a spectrum
        
        This function reads an AC variable and tries to convert it into a
        valid spectrum. The Spectrum variable is granted to be valid only
        for one call of the processing function.
    
        \param ac AC handle
        \param name Name of the variable
        \return Spectrum structure
    */
    mha_spec_t get_var_spectrum(algo_comm_t ac,const std::string& name);

    /** 
        \ingroup algocomm

        \brief Convert an AC variable into a waveform
        
        This function reads an AC variable and tries to convert it into a
        valid waveform. The waveform variable is granted to be valid only
        for one call of the processing function.
    
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

       \param ac AC handle
       \param name Name of the variable
       \return Variable value
     */
    std::vector<float> get_var_vfloat(algo_comm_t ac,const std::string& name);
    
    /**
       \ingroup algocomm
       \brief Insert a MHASignal::spectrum_t class into the AC space

       The variable is automatically removed on destruction.

    */
    class spectrum_t : public MHASignal::spectrum_t {
    public:
        /** \brief Create the AC variable.
            \param ac AC handle
            \param name Name of variable in AC space
            \param bins Number of FFT bins in the waveform_t class
            \param channels Number of audio channels in the waveform_t class
            \param insert_now Insert implicitely in the constructor (true) or explicitely in the insert() function (false)
         */
        spectrum_t(algo_comm_t ac,
                   std::string name,
                   unsigned int bins,
                   unsigned int channels,
                   bool insert_now);
        ~spectrum_t();
        void insert();//!< Insert AC variable into AC space
    protected:
        algo_comm_t ac;
        std::string name;
    };

    /**
       \ingroup algocomm
       \brief Insert a MHASignal::waveform_t class into the AC space

       The variable is automatically removed on destruction.

    */
    class waveform_t : public MHASignal::waveform_t {
    public:
        /** \brief Create the AC variable.
            \param ac AC handle
            \param name Name of variable in AC space
            \param frames Number of frames in the waveform_t class
            \param channels Number of audio channels in the waveform_t class
            \param insert_now Insert implicitely in the constructor (true) or explicitely in the insert() function (false)
         */
        waveform_t(algo_comm_t ac,
                   std::string name,
                   unsigned int frames,
                   unsigned int channels,
                   bool insert_now);
        ~waveform_t();
        void insert();//!< Insert AC variable into AC space
    protected:
        algo_comm_t ac;
        std::string name;
    };

    /**
       \ingroup algocomm
       \brief Insert a integer variable into the AC space

       The variable is automatically removed on destruction.

    */
    class int_t {
    public:
        int_t(algo_comm_t,std::string,int=0);
        ~int_t();
        int data; //!< Integer value variable
    private:
        algo_comm_t ac;
    };

    /**
       \ingroup algocomm
       \brief Insert a float point variable into the AC space

       The variable is automatically removed on destruction.

    */
    class float_t {
    public:
        float_t(algo_comm_t,std::string,float=0); //!< Constructor
        ~float_t();
        float data; //!< Floating point value variable
    private:
        algo_comm_t ac;
    };

    /**
       \ingroup algocomm
       \brief Insert a double precision floating point variable into the AC space

       The variable is automatically removed on destruction.

    */
    class double_t {
    public:
        double_t(algo_comm_t,std::string,double=0);
        ~double_t();
        double data; //!< Floating point value variable
    private:
        algo_comm_t ac;
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

#endif

#endif

/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * c-basic-offset: 4
 * End:
 */
