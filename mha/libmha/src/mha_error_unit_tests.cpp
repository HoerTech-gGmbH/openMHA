// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 2018 2019 2020 HörTech gGmbH
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

#include "mha_error.hh"
#include <gtest/gtest.h>
#include <cstdint>

TEST(MHA_Error, helper_function_digits_computes_correct_number_of_decimal_digits)
{
  using mha_error_helpers::digits;
  EXPECT_EQ(1U, digits(0U));
  EXPECT_EQ(1U, digits(1U));
  EXPECT_EQ(1U, digits(9U));
  EXPECT_EQ(2U, digits(10U));
  EXPECT_EQ(2U, digits(99U));
  EXPECT_EQ(3U, digits(100U));
  EXPECT_EQ(10U, digits(4294967295U));
}

TEST(MHA_Error, MHA_Error_is_a_subclass_of_std_exception) {
  MHA_Error error_instance("",0," ");
  std::exception * std_exception_pointer = &error_instance;
  EXPECT_NE(nullptr, std_exception_pointer);
}

TEST(MHA_Error, snprintf_required_length_computes_correct_length) {
  using mha_error_helpers::snprintf_required_length;
  EXPECT_EQ(0U, snprintf_required_length(""));
  EXPECT_EQ(1U, snprintf_required_length("1"));
  EXPECT_EQ(1U, snprintf_required_length("%s", "1"));
  EXPECT_EQ(1U, snprintf_required_length("%d", 1));
  EXPECT_EQ(2U, snprintf_required_length("%d", 10));
  EXPECT_EQ(6U, snprintf_required_length("%d\n%.1f\n", 1,0.5));
}

TEST(MHA_Error, format_string_conversions) {
    // convert char with %c
    char c = 'x';
    EXPECT_STREQ("() x", MHA_Error("",0,"%c",c).get_msg());

    // convert a c string with %s
    char array[] = "hello";
    EXPECT_STREQ("() hello", MHA_Error("",0,"%s",array).get_msg());
    
    // convert a short unsigned integer with %hu
    unsigned short us = 65535;
    EXPECT_STREQ("() 65535", MHA_Error("",0,"%hu",us).get_msg());

    // convert a short signed integer with %hd
    short s = -1;
    EXPECT_STREQ("() -1", MHA_Error("",0,"%hd",s).get_msg());

    // convert an unsigned int with %u
    unsigned ui = 0;
    EXPECT_STREQ("() 0", MHA_Error("",0,"%u",ui).get_msg());

    // convert a signed int with %d and %i
    int i = -1;
    EXPECT_STREQ("() -1", MHA_Error("",0,"%d",i).get_msg());
    EXPECT_STREQ("() -1", MHA_Error("",0,"%i",i).get_msg());

    // convert a size_t and a std::vector<int>::size_type with %zu
    // Make our assumption that size_t equals std::vector<int>::size_type
    // explicit with a static assert
    static_assert(std::is_same<size_t, std::vector<int>::size_type>::value, "Type size_t and standard library size_type\n"
                                                                             "are not equivalent");
    const size_t size = 10;
    const std::vector<int>::size_type size_vector = 10;
    EXPECT_STREQ("() 10", MHA_Error("",0,"%zu",size).get_msg());
    EXPECT_STREQ("() 10", MHA_Error("",0,"%zu",size_vector).get_msg());

    // convert pointers with %p. Pointer values are unpredictable.
    char * hello = array;
    char * ello = hello+1;
    EXPECT_STREQ("ello", ello);
    MHA_Error hello_error("",0,"%p",hello);
    const unsigned strlen_hello_error = strlen(hello_error.get_msg());
    char last_hex_digit_hello = hello_error.get_msg()[strlen_hello_error-1];
    switch (last_hex_digit_hello) {
        // pointer to "ello" has address of pointer to "hello" plus one, in hex
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case 'a': case 'b': case 'c': case 'd': case 'e':
        EXPECT_EQ(char(last_hex_digit_hello+1),
                  MHA_Error("",0,"%p",ello).get_msg()[strlen_hello_error-1]);
        break;
    case '9':
        EXPECT_EQ('a',
                  MHA_Error("",0,"%p",ello).get_msg()[strlen_hello_error-1]);
        break;
    default: // case 'f':
        EXPECT_EQ('0',
                  MHA_Error("",0,"%p",ello).get_msg()[strlen_hello_error-1]);
        break;
    }
    
    // convert a float and a double with %g for values <<1, >>1, and ~1
    //Values <<1
    float f = 1.1e-20f;
    double d = 1.1e-40;
    EXPECT_STREQ("() 1.1e-20", MHA_Error("",0,"%g",f).get_msg());
    EXPECT_STREQ("() 1.1e-40", MHA_Error("",0,"%g",d).get_msg());

    //Values >>1
    f = 1.1e20f;
    d = 1.1e40;
    EXPECT_STREQ("() 1.1e+20", MHA_Error("",0,"%g",f).get_msg());
    EXPECT_STREQ("() 1.1e+40", MHA_Error("",0,"%g",d).get_msg());

    //Values ~1
    f = 99.9e-2f;
    d = 99.9e-2;
    EXPECT_STREQ("() 0.999", MHA_Error("",0,"%g",f).get_msg());
    EXPECT_STREQ("() 0.999", MHA_Error("",0,"%g",d).get_msg());

    // convert a float and a double with %f
    f = 1.0f;
    d = 1.0;
    EXPECT_STREQ("() 1.000000", MHA_Error("",0,"%f",f).get_msg());
    EXPECT_STREQ("() 1.000000", MHA_Error("",0,"%f",d).get_msg());
}

/*
 * Local Variables:
 * compile-command: "make -C .. unit-tests"
 * coding: utf-8-unix
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
