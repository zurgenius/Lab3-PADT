#include "linked_list.h"

#include <gtest/gtest.h>
#include <stdexcept>

TEST(LinkedListTest, DefaultConstructorCreatesEmptyList) {
    LinkedList<int> list;

    EXPECT_EQ(list.get_length(), 0);
}

TEST(LinkedListTest, ItemsConstructorCopiesItems) {
    int items[] = {3, 4, 5};

    LinkedList<int> list(items, 3);

    EXPECT_EQ(list.get_first(), 3);
    EXPECT_EQ(list.get_last(), 5);
}

TEST(LinkedListTest, CopyConstructorCopiesValues) {
    int items[] = {1, 2, 3};
    LinkedList<int> original(items, 3);

    LinkedList<int> copy(original);

    original.insert_at(99, 1);

    EXPECT_EQ(copy.get_length(), 3);
    EXPECT_EQ(copy.get_first(), 1);
    EXPECT_EQ(copy.get_last(), 3);
}

TEST(LinkedListTest, AssignmentOperatorCopiesValues) {
    int source_items[] = {8, 9, 10};
    int target_items[] = {1};
    LinkedList<int> source(source_items, 3);
    LinkedList<int> target(target_items, 1);

    target = source;
    source.prepend(7);

    EXPECT_EQ(target.get_length(), 3);
    EXPECT_EQ(target.get_first(), 8);
    EXPECT_EQ(target.get_last(), 10);
}

TEST(LinkedListTest, GetFirstReturnsHead) {
    int items[] = {11, 12};
    LinkedList<int> list(items, 2);

    EXPECT_EQ(list.get_first(), 11);
}

TEST(LinkedListTest, GetLastReturnsTail) {
    int items[] = {11, 12};
    LinkedList<int> list(items, 2);

    EXPECT_EQ(list.get_last(), 12);
}

TEST(LinkedListTest, GetLengthReturnsLength) {
    int items[] = {6, 7, 8, 9};
    LinkedList<int> list(items, 4);

    EXPECT_EQ(list.get_length(), 4);
}

TEST(LinkedListTest, GetSubListReturnsRequestedRange) {
    int items[] = {5, 6, 7, 8};
    LinkedList<int> list(items, 4);

    LinkedList<int> *sublist = list.get_sub_list(1, 2);

    EXPECT_EQ(sublist->get_length(), 2);
    EXPECT_EQ(sublist->get_first(), 6);
    EXPECT_EQ(sublist->get_last(), 7);

    delete sublist;
}

TEST(LinkedListTest, AppendAddsElementToTail) {
    LinkedList<int> list;

    list.append(4);

    EXPECT_EQ(list.get_last(), 4);
}

TEST(LinkedListTest, PrependAddsElementToHead) {
    LinkedList<int> list;

    list.prepend(4);

    EXPECT_EQ(list.get_first(), 4);
}

TEST(LinkedListTest, InsertAtAddsElementInMiddle) {
    int items[] = {1, 3};
    LinkedList<int> list(items, 2);

    list.insert_at(2, 1);

    IEnumerator<int> *iterator = list.get_enumerator();
    ASSERT_TRUE(iterator->move_next());
    EXPECT_EQ(iterator->get_current(), 1);
    ASSERT_TRUE(iterator->move_next());
    EXPECT_EQ(iterator->get_current(), 2);
    ASSERT_TRUE(iterator->move_next());
    EXPECT_EQ(iterator->get_current(), 3);
    delete iterator;
}

TEST(LinkedListTest, EnumeratorWalksThroughList) {
    int items[] = {3, 4, 5};
    LinkedList<int> list(items, 3);

    int total = 0;
    IEnumerator<int> *iterator = list.get_enumerator();
    while (iterator->move_next()) {
        total += iterator->get_current();
    }
    delete iterator;

    EXPECT_EQ(total, 12);
}

TEST(LinkedListTest, GetFirstThrowsOnEmptyList) {
    LinkedList<int> list;

    EXPECT_THROW(list.get_first(), std::out_of_range);
}

TEST(LinkedListTest, GetLastThrowsOnEmptyList) {
    LinkedList<int> list;

    EXPECT_THROW(list.get_last(), std::out_of_range);
}
