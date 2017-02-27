// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2013 2016 HörTech gGmbH
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

// A ruby language binding for mhafw_lib 

#include "mhafw_lib.h"
#include "ruby.h"

static void mha_free(void * mha) {
  delete static_cast<fw_t *>(mha);
  mha = 0;
}

static VALUE mha_alloc(VALUE klass) {
  fw_t * mha = new fw_t;
  return Data_Wrap_Struct(klass, 0, &mha_free, mha);
}

static VALUE mha_exit_request(VALUE self) {
  fw_t * mha = 0;
  Data_Get_Struct(self, fw_t, mha);
  return mha->exit_request() ? Qtrue : Qfalse;
}

static VALUE mha_parse(VALUE self, VALUE request) {
  request = StringValue(request);
  if (request == Qnil)
    rb_raise(rb_eTypeError,"parse method needs string parameter");
  fw_t * mha = 0;
  Data_Get_Struct(self, fw_t, mha);
  std::string answer;
  try {
    answer = mha->parse(std::string(RSTRING_PTR(request),
                                    RSTRING_LEN(request)));
    return rb_str_new(answer.data(), answer.length());
  }
  catch (std::exception & e) {
    rb_raise(rb_eRuntimeError, "%s", e.what());
  }
}

typedef VALUE(*rb_f_t)(...);

extern "C"
void Init_mha_ruby() {
  VALUE rb_cMHA = rb_define_class("MHA", rb_cObject);
  rb_define_alloc_func(rb_cMHA, &mha_alloc);
  rb_define_method(rb_cMHA, "exit_request?", (rb_f_t)&mha_exit_request, 0);
  rb_define_method(rb_cMHA, "parse",  (rb_f_t)&mha_parse, 1);
}

// Local Variables:
// c-basic-offset: 2
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
