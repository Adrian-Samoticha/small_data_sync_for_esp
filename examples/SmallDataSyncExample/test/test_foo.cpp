#include <unity.h>

#include "foo.h"

void test_foo() {
  SmallDataSync data;

  TEST_ASSERT_EQUAL(1, 1);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();

  RUN_TEST(test_foo);

  return UNITY_END();
}