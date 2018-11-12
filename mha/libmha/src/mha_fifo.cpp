// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2008 2009 2010 2011 2013 2014 2016 2017 2018 HörTech gGmbH
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

#include <cassert>
#include <algorithm>
#include "mha.h"
#include "mha_error.hh"
#include "mha_fifo.h"

template <class T>
void mha_fifo_lw_t<T>::write(const T * data, unsigned count)
{
    mha_fifo_thread_guard_t fifo_guard(sync);
    if (error[1]) throw *error[1];
    while (count > 0) {
        if (error[1]) throw *error[1];
        unsigned available_space = this->get_available_space();
        unsigned can_write = std::min(available_space, count);
        if (can_write > 0) {
            // write as much as possible now
            this->mha_fifo_t<T>::write(data, can_write);
            data += can_write;
            count -= can_write;
            sync->increment();
        }
        else {
            // cannot write anything but need to, wait for more space
            if (error[1]) throw *error[1];
            sync->wait_for_decrease();
        }
    }
}

template <class T>
void mha_fifo_lw_t<T>::read(T * buf, unsigned count)
{
    mha_fifo_thread_guard_t fifo_guard(sync);
    unsigned remaining = count;
    if (error[0]) throw *error[0];
    while (remaining > 0) {
        if (error[0]) throw *error[0];
        unsigned available_data = this->get_fill_count();
        unsigned can_read = std::min(available_data, remaining);
        if (can_read > 0) {
            // read as much as possible now
            this->mha_fifo_t<T>::read(buf, can_read);
            buf += can_read;
            remaining -= can_read;
            sync->decrement();
        }
        else {
            // cannot read anything but need to, wait for more data
            if (error[0]) throw *error[0];
            sync->wait_for_increase();
        }
    }
}

template <class T>
mha_fifo_lw_t<T>::mha_fifo_lw_t(unsigned max_fill_count)
    : mha_fifo_t<T>(max_fill_count),
      sync(new mha_fifo_thread_platform_implementation_t())
{
    error[0] = error[1] = 0;
}

template <class T>
mha_fifo_lw_t<T>::~mha_fifo_lw_t()
{
    delete sync;
    sync = 0;
}

template <class T>
void mha_fifo_lw_t<T>::set_error(unsigned index, MHA_Error * error)
{
    mha_fifo_thread_guard_t fifo_guard(sync);
    index &= 1;
    this->error[index] = error;
    if (error) {
        if (index == 0) {
            sync->increment();
        }
        else if (index == 1) {
            sync->decrement();
        }
    }
}

template <class FIFO>
mha_dblbuf_t<FIFO>::mha_dblbuf_t(unsigned outer_size_, unsigned inner_size_,
                                 unsigned delay_,
                                 unsigned input_channels_,
                                 unsigned output_channels_,
                                 const value_type &delay_data)
    : outer_size(outer_size_),
      inner_size(inner_size_),
      delay(delay_),
      fifo_size(delay + std::max(inner_size, outer_size)),
      input_channels(input_channels_),
      output_channels(output_channels_),
      input_fifo(fifo_size * input_channels),
      output_fifo(fifo_size * output_channels),
      inner_error(0),
      outer_error(0)
{
    if (input_channels == 0 || output_channels == 0)
        throw MHA_ErrorMsg("mha doublebuffer does not support 0 channels");
    for (unsigned k = 0; k < delay * input_channels; ++k)
        input_fifo.write(&delay_data, 1);
}

template <class FIFO>
mha_dblbuf_t<FIFO>::~mha_dblbuf_t()
{
    delete inner_error; 
    inner_error = 0;
    delete outer_error;
    outer_error = 0;
}

template <class FIFO>
void mha_dblbuf_t<FIFO>::process(const value_type * input_signal,
                                 value_type * output_signal,
                                 unsigned count)
{
    if (count > outer_size)
        throw MHA_Error(__FILE__, __LINE__,
                        "mha_dblbuf_t::process called with greater buffer (%u)"
                        " than specified during initialization (%u)",
                        count,
                        outer_size);
    input_fifo.write(input_signal, count * input_channels);
    output_fifo.read(output_signal, count * output_channels);
}

template <class FIFO>
void mha_dblbuf_t<FIFO>::input(value_type * input_signal)
{
    input_fifo.read(input_signal, inner_size * input_channels);
}

template <class FIFO>
void mha_dblbuf_t<FIFO>::output(const value_type * output_signal)
{
    output_fifo.write(output_signal, inner_size * output_channels);
}

template <class FIFO>
void mha_dblbuf_t<FIFO>::provoke_inner_error(const MHA_Error & error)
{
    delete inner_error;
    inner_error = 0;
    inner_error = new MHA_Error(error);
    input_fifo.set_error(0, inner_error);
    output_fifo.set_error(1, inner_error);
}
    
template <class FIFO>
void mha_dblbuf_t<FIFO>::provoke_outer_error(const MHA_Error & error)
{
    delete outer_error;
    outer_error = 0;
    outer_error = new MHA_Error(error);
    input_fifo.set_error(1, outer_error);
    output_fifo.set_error(0, outer_error);
}

template class mha_fifo_t<mha_real_t>;
template class mha_dblbuf_t<mha_fifo_lw_t<mha_real_t> >;
template class mha_fifo_lw_t<mha_real_t>;

/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * indent-tabs-mode: nil
 * End:
 */
