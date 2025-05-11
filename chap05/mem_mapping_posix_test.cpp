#include <gtest/gtest.h>
/**
 *  g++ -L/usr/local/lib  -o mem_mapping_posix_test mem_mapping_posix_test.cpp -lgtest -lgtest_main -lpthread
*/
extern "C" {
    #include "mem_mapping_posix.c"  // 直接包含 C 文件，或者包含相应的头文件（如果有）
}

// 测试套件
class MemMappingPosixTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化代码（如果需要）
    }

    void TearDown() override {
        // 清理代码（如果需要）
    }
};

// 测试用例
TEST_F(MemMappingPosixTest, OpenAndClose) {
    const char* queue_name = "/004";
    struct mq_attr attr = {0, 10, 256, 0};

    mqd_t mqd = mq_open(queue_name, O_CREAT | O_RDWR, 0666, &attr);

    //ASSERT_NE(mqd, (mqd_t)-1) << "mq_open failed";

    int result = mq_close(mqd);
    ASSERT_EQ(result, 0) << "mq_close failed";

    int unlink_result = mq_unlink(queue_name);
    ASSERT_EQ(unlink_result, 0) << "mq_unlink failed";
}

// 其他测试用例...

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}