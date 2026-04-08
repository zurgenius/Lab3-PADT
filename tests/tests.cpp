#include "array_sequence.h"
#include "list_sequence.h"
#include "utils.h"

#include <gtest/gtest.h>

TEST(DynamicArrayTest, SupportsResizeAndIndexedAccess) {
    DynamicArray<int> array(2);
    array.set(0, 7);
    array.set(1, 9);

    array.resize(4);
    array.set(2, 11);

    EXPECT_EQ(array.get_size(), 4);
    EXPECT_EQ(array[0], 7);
    EXPECT_EQ(array[2], 11);
}

TEST(DynamicArrayTest, ThrowsOnInvalidIndex) {
    DynamicArray<int> array(1);
    EXPECT_THROW(array.get(-1), std::out_of_range);
    EXPECT_THROW(array.set(2, 10), std::out_of_range);
}

TEST(LinkedListTest, PreservesOrderAcrossOperations) {
    LinkedList<int> list;
    list.append(2);
    list.prepend(1);
    list.insert_at(3, 2);

    EXPECT_EQ(list.get_length(), 3);
    EXPECT_EQ(list.get_first(), 1);
    EXPECT_EQ(list.get_last(), 3);
    EXPECT_EQ(list[1], 2);
}

TEST(LinkedListTest, CopiesIndependently) {
    int items[] = {1, 2, 3};
    LinkedList<int> original(items, 3);
    LinkedList<int> copy(original);

    original.insert_at(99, 1);

    EXPECT_EQ(copy.get_length(), 3);
    EXPECT_EQ(copy.get(1), 2);
}

TEST(LinkedListTest, ReturnsSublist) {
    int items[] = {5, 6, 7, 8};
    LinkedList<int> list(items, 4);

    LinkedList<int> *sublist = list.get_sub_list(1, 2);
    EXPECT_EQ(sublist->get_length(), 2);
    EXPECT_EQ(sublist->get_first(), 6);
    EXPECT_EQ(sublist->get_last(), 7);

    delete sublist;
}

TEST(IteratorTest, WalksThroughList) {
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

TEST(MutableArraySequenceTest, ModifiesObjectInPlace) {
    MutableArraySequence<int> sequence;
    Sequence<int> *same = sequence.append(1);
    same = same->append(2);
    same = same->prepend(0);

    EXPECT_EQ(same, &sequence);
    EXPECT_EQ(sequence.get_count(), 3);
    EXPECT_EQ(sequence[0], 0);
    EXPECT_EQ(sequence[2], 2);
}

TEST(ImmutableArraySequenceTest, ReturnsNewObject) {
    int items[] = {1, 2, 3};
    ImmutableArraySequence<int> sequence(items, 3);

    Sequence<int> *updated = sequence.append(4);

    EXPECT_EQ(sequence.get_count(), 3);
    EXPECT_EQ(updated->get_count(), 4);
    EXPECT_EQ(updated->get_last(), 4);

    delete updated;
}

TEST(MutableListSequenceTest, SupportsMapWhereReduce) {
    int items[] = {1, 2, 3, 4};
    MutableListSequence<int> sequence(items, 4);

    Sequence<int> *mapped = sequence.map(utils::square);
    Sequence<int> *filtered = sequence.where(utils::is_even);
    int reduced = sequence.reduce(utils::sum, 0);

    EXPECT_EQ(mapped->get(2), 9);
    EXPECT_EQ(filtered->get_count(), 2);
    EXPECT_EQ(filtered->get(1), 4);
    EXPECT_EQ(reduced, 10);

    delete mapped;
    delete filtered;
}

TEST(ImmutableListSequenceTest, SupportsSubsequenceAndConcat) {
    int left_items[] = {1, 2, 3, 4};
    int right_items[] = {5, 6};
    ImmutableListSequence<int> left(left_items, 4);
    ImmutableListSequence<int> right(right_items, 2);

    Sequence<int> *sub = left.get_sub_sequence(1, 2);
    Sequence<int> *joined = left.concat(&right);

    EXPECT_EQ(sub->get_count(), 2);
    EXPECT_EQ(sub->get_first(), 2);
    EXPECT_EQ(joined->get_count(), 6);
    EXPECT_EQ(joined->get_last(), 6);

    delete sub;
    delete joined;
}

TEST(SequenceTest, TryGetUsesOption) {
    MutableArraySequence<int> sequence;
    EXPECT_FALSE(sequence.try_get_first().has_value());

    sequence.append(42);
    EXPECT_TRUE(sequence.try_get_first().has_value());
    EXPECT_EQ(sequence.try_get_first().get_value(), 42);
}

TEST(SequenceTest, SliceReplacesRequestedRange) {
    int items[] = {1, 2, 3, 4, 5};
    int replacement_items[] = {8, 9};
    MutableArraySequence<int> sequence(items, 5);
    MutableArraySequence<int> replacement(replacement_items, 2);

    Sequence<int> *sliced = sequence.slice(1, 3, &replacement);

    EXPECT_EQ(sliced->get_count(), 4);
    EXPECT_EQ(sliced->get(0), 1);
    EXPECT_EQ(sliced->get(1), 8);
    EXPECT_EQ(sliced->get(2), 9);
    EXPECT_EQ(sliced->get(3), 5);

    delete sliced;
}
