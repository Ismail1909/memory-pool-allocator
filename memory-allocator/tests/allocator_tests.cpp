#include <gtest/gtest.h>
#include"alloc.h"

// Test alloc functions works.
TEST(AllocatorTest, BasicTest) {
  void* x = alloc(sizeof(int));
  EXPECT_NE(x,nullptr);
}