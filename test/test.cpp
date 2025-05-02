#include <gtest/gtest.h>

// 一个简单的测试用例
TEST(SampleTest, EqualityTest) {
    EXPECT_EQ(1 + 1, 2);
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}