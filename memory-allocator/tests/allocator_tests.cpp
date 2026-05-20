#include <gtest/gtest.h>
#include"alloc.h"

// Test alloc functions works.
TEST(AllocatorTest, BasicAllocTest) {
  void* x = alloc(sizeof(int));
  EXPECT_NE(x,nullptr);

  void* y = alloc(0);
  EXPECT_EQ(y, nullptr);
}

TEST(AllocatorTest, BasicDestroyTest) {
  void* x = alloc(sizeof(int));
  EXPECT_NE(x, nullptr); // memory allocated

  EXPECT_EQ(destroy(&x), true); // memory freed successfully.
  EXPECT_EQ(x, nullptr); // address set to NULL.

  EXPECT_EQ(destroy(&x), false); // Free NULL returns false
}

TEST(AllocatorTest, BasicFullAllocTest) {
  void* x = alloc(1024*1024*1024 - 4); // Allocate all available memory.
  EXPECT_NE(x, nullptr); // memory allocated

  void* y = alloc(4);
  EXPECT_EQ(y, nullptr); // We should not be able to allocate memory as there is no mem available.
}

TEST(AllocatorTest, BasicReallocTest) {
  void* x = alloc(sizeof(int));
  EXPECT_NE(x, nullptr); // memory allocated
  void* xAddress = x;

  void* y = alloc(sizeof(int));
  EXPECT_NE(y, nullptr);

  EXPECT_EQ(destroy(&x), true); // memory freed successfully.
  EXPECT_EQ(x, nullptr); // address set to NULL.

  void* z = alloc(sizeof(int));
  EXPECT_NE(z, nullptr);
  EXPECT_EQ(z, xAddress); // z should retake x's place.

  // Clear all allocations

  EXPECT_EQ(destroy(&y), true); // memory freed successfully.
  EXPECT_EQ(y, nullptr); // address set to NULL.

  EXPECT_EQ(destroy(&z), true); // memory freed successfully.
  EXPECT_EQ(z, nullptr); // address set to NULL.
}