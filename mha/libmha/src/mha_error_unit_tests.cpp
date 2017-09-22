#include "mha_error.hh"
#include <gtest/gtest.h>

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
  MHA_Error error_instance("",0,"");
  std::exception * std_exception_pointer = &error_instance;
  EXPECT_NE(nullptr, std_exception_pointer);
}
