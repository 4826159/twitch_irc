#include "gtest/gtest.h"

extern "C" {
    #include "chat_queue.h"
}

#include <string>
#include <cstring>

TEST(QTest, BasicCreate)
{
    struct chat_q *queue = NULL;

    EXPECT_EQ(NULL, queue);

    queue = init_queue();

    ASSERT_NE((struct chat_q *)NULL, queue);

    EXPECT_EQ(0, queue->start);
    EXPECT_EQ(0, queue->end);
    EXPECT_EQ(0, queue->length);
    ASSERT_TRUE(NULL != queue->messages);

    for (int i = 0; i < MESSAGE_TOTAL; i++)
        EXPECT_TRUE(NULL == queue->messages[i]);

    delete_queue(queue);
}

TEST(QTest, Sweep)
{
    struct chat_q *queue;

    queue = init_queue();

    for (int i = 0; i < MESSAGE_TOTAL; i++) {
        enqueue_line(queue, NULL);
        ASSERT_FALSE(queue->start == queue->end);
        dequeue_line(queue, NULL);
        EXPECT_EQ(0, queue->length);
        ASSERT_TRUE(queue->start == queue->end);
    }

    ASSERT_EQ(0, queue->end);
    ASSERT_EQ(0, queue->start);

    delete_queue(queue);
}

TEST(QTest, Fill)
{
    struct chat_q *queue;

    queue = init_queue();

    for (int i = 0; i < MESSAGE_TOTAL; i++) {
        ASSERT_EQ(i, queue->length);
        ASSERT_EQ(i, queue->end);
        ASSERT_EQ(0, queue->start);
        std::string number = "Put in " + std::to_string(i);
        char *c_number = new char[number.length()];
        std::strcpy(c_number, number.c_str());
        enqueue_line(queue, c_number);
    }

    ASSERT_EQ(MESSAGE_TOTAL, queue->length);
    ASSERT_EQ(0, queue->end);
    ASSERT_EQ(0, queue->start);

    for (int i = 0; i < MESSAGE_TOTAL; i++) {
        ASSERT_EQ(MESSAGE_TOTAL - i, queue->length);
        std::string number = "Put in " + std::to_string(i);
        char *c_number;
        dequeue_line(queue, &c_number);
        ASSERT_STREQ(number.c_str(), c_number);
    }

    EXPECT_TRUE(queue->start == queue->end);

    delete_queue(queue);
}

// Runs in 16ms on my Pi3B
TEST(QTest, ManyFill)
{
    struct chat_q *queue;

    queue = init_queue();

    for (int test = 0; test < 1000; test++) {

        for (int i = 0; i < MESSAGE_TOTAL; i++)
            enqueue_line(queue, NULL);

        ASSERT_EQ(MESSAGE_TOTAL, queue->length);
        ASSERT_EQ(0, queue->end);
        ASSERT_EQ(0, queue->start);

        for (int i = 0; i < MESSAGE_TOTAL; i++)
            dequeue_line(queue, NULL);

        EXPECT_TRUE(queue->start == queue->end);
    }

    delete_queue(queue);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
