#include "dynamic_array.h"

#include <gtest/gtest.h>
#include <stdexcept>

TEST(DynamicArrayTest, DefaultConstructorCreatesEmptyArray) {
    DynamicArray<int> array;

    EXPECT_EQ(array.get_size(), 0);
}

TEST(DynamicArrayTest, SizedConstructorCreatesRequestedSize) {
    DynamicArray<int> array(3);

    EXPECT_EQ(array.get_size(), 3);
}

TEST(DynamicArrayTest, ItemsConstructorCopiesItems) {
    int items[] = {4, 5, 6};

    DynamicArray<int> array(items, 3);

    EXPECT_EQ(array.get(0), 4);
    EXPECT_EQ(array.get(1), 5);
    EXPECT_EQ(array.get(2), 6);
}

TEST(DynamicArrayTest, CopyConstructorCopiesValues) {
    int items[] = {7, 8};
    DynamicArray<int> original(items, 2);

    DynamicArray<int> copy(original);

    EXPECT_EQ(copy.get(0), 7);
    EXPECT_EQ(copy.get(1), 8);
}

TEST(DynamicArrayTest, GetReturnsStoredElement) {
    int items[] = {10, 20, 30};
    DynamicArray<int> array(items, 3);

    EXPECT_EQ(array.get(1), 20);
}

TEST(DynamicArrayTest, OperatorIndexReturnsStoredElement) {
    int items[] = {1, 3, 5};
    DynamicArray<int> array(items, 3);

    EXPECT_EQ(array[2], 5);
}

TEST(DynamicArrayTest, GetSizeReturnsCurrentSize) {
    DynamicArray<int> array(5);

    EXPECT_EQ(array.get_size(), 5);
}

TEST(DynamicArrayTest, SetChangesElement) {
    DynamicArray<int> array(2);

    array.set(1, 42);

    EXPECT_EQ(array.get(1), 42);
}

TEST(DynamicArrayTest, ResizeGrowsArray) {
    int items[] = {2, 4};
    DynamicArray<int> array(items, 2);

    array.resize(4);

    EXPECT_EQ(array.get_size(), 4);
    EXPECT_EQ(array.get(0), 2);
    EXPECT_EQ(array.get(1), 4);
}

TEST(DynamicArrayTest, ResizeShrinksArray) {
    int items[] = {2, 4, 6};
    DynamicArray<int> array(items, 3);

    array.resize(2);

    EXPECT_EQ(array.get_size(), 2);
    EXPECT_EQ(array.get(0), 2);
    EXPECT_EQ(array.get(1), 4);
}

TEST(DynamicArrayTest, EnumeratorWalksThroughArray) {
    int items[] = {1, 2, 3};
    DynamicArray<int> array(items, 3);

    int total = 0;
    IEnumerator<int> *iterator = array.get_enumerator();
    while (iterator->move_next()) {
        total += iterator->get_current();
    }
    delete iterator;

    EXPECT_EQ(total, 6);
}

TEST(DynamicArrayTest, GetThrowsOnInvalidIndex) {
    DynamicArray<int> array(1);

    EXPECT_THROW(array.get(-1), std::out_of_range);
}

TEST(DynamicArrayTest, SetThrowsOnInvalidIndex) {
    DynamicArray<int> array(1);

    EXPECT_THROW(array.set(2, 10), std::out_of_range);
}
