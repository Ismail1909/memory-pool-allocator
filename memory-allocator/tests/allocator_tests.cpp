#include <gtest/gtest.h>
#include"alloc.h"

// Test alloc functions works.
TEST(AllocatorTest, BasicAllocTest) {
  void* x = alloc(sizeof(int));
  EXPECT_NE(x,nullptr);
}

TEST(AllocatorTest, BasicDeleteTest) {
  void* x = alloc(32);
  EXPECT_NE(x, nullptr); // memory allocated

  EXPECT_EQ(destroy(&x), true); // memory freed successfully.
  EXPECT_EQ(x, nullptr); // address set to NULL.
}