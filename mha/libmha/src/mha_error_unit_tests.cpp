#include "mha_error.hh"
#include <gtest/test.h>

TEST(mha_error, helper_function_digits_computes_correct_number_of_decimal_digits)
{
  using mha_error_helpers::digits;
  EXPECT_EQ(1U, digits(0U));
  EXPECT_EQ(1U, digits(1U));
  EXPECT_EQ(1U, digits(9U));
  EXPECT_EQ(2U, digits(10U));
  EXPECT_EQ(2U, digits(99U));
  EXPECT_EQ(3U, digits(100U));
  EXPECT_TRUE(false);
}
