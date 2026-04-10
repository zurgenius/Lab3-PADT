#include "array_sequence.h"
#include "list_sequence.h"
#include "utils.h"

#include <gtest/gtest.h>

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

TEST(ArraySequenceTest, DefaultConstructorCreatesEmptySequence) {
    MutableArraySequence<int> sequence;

    EXPECT_EQ(sequence.get_count(), 0);
}

TEST(ArraySequenceTest, ItemsConstructorCopiesItems) {
    int items[] = {1, 2, 3};

    MutableArraySequence<int> sequence(items, 3);

    EXPECT_EQ(sequence.get_count(), 3);
    EXPECT_EQ(sequence.get_first(), 1);
    EXPECT_EQ(sequence.get_last(), 3);
}

TEST(ArraySequenceTest, CopyConstructorCopiesState) {
    int items[] = {2, 4, 6};
    MutableArraySequence<int> original(items, 3);

    MutableArraySequence<int> copy(original);
    original.append(8);

    EXPECT_EQ(copy.get_count(), 3);
    EXPECT_EQ(copy.get_last(), 6);
}

TEST(ArraySequenceTest, GetFirstReturnsFirstItem) {
    int items[] = {9, 8};
    MutableArraySequence<int> sequence(items, 2);

    EXPECT_EQ(sequence.get_first(), 9);
}

TEST(ArraySequenceTest, GetLastReturnsLastItem) {
    int items[] = {9, 8};
    MutableArraySequence<int> sequence(items, 2);

    EXPECT_EQ(sequence.get_last(), 8);
}

TEST(ArraySequenceTest, GetReturnsValueByIndex) {
    int items[] = {9, 8, 7};
    MutableArraySequence<int> sequence(items, 3);

    EXPECT_EQ(sequence.get(1), 8);
}

TEST(ArraySequenceTest, TryGetReturnsNoneForInvalidIndex) {
    MutableArraySequence<int> sequence;

    EXPECT_FALSE(sequence.try_get(0).has_value());
}

TEST(ArraySequenceTest, GetCountReturnsLogicalSize) {
    int items[] = {1, 2, 3, 4};
    MutableArraySequence<int> sequence(items, 4);

    EXPECT_EQ(sequence.get_count(), 4);
}

TEST(ArraySequenceTest, GetSubSequenceReturnsRequestedRange) {
    int items[] = {1, 2, 3, 4};
    MutableArraySequence<int> sequence(items, 4);

    Sequence<int> *subsequence = sequence.get_sub_sequence(1, 2);

    EXPECT_EQ(subsequence->get_count(), 2);
    EXPECT_EQ(subsequence->get_first(), 2);
    EXPECT_EQ(subsequence->get_last(), 3);

    delete subsequence;
}

TEST(ArraySequenceTest, AppendMutatesMutableSequence) {
    MutableArraySequence<int> sequence;

    Sequence<int> *result = sequence.append(5);

    EXPECT_EQ(result, &sequence);
    EXPECT_EQ(sequence.get_last(), 5);
}

TEST(ArraySequenceTest, AppendHandlesLargeNumberOfOperations) {
    MutableArraySequence<int> sequence;

    for (int value = 0; value < 100000; value++) {
        sequence.append(value);
    }

    EXPECT_EQ(sequence.get_count(), 100000);
    EXPECT_EQ(sequence.get_first(), 0);
    EXPECT_EQ(sequence.get_last(), 99999);
}

TEST(ArraySequenceTest, PrependMutatesMutableSequence) {
    int items[] = {2, 3};
    MutableArraySequence<int> sequence(items, 2);

    sequence.prepend(1);

    EXPECT_EQ(sequence.get_first(), 1);
}

TEST(ArraySequenceTest, InsertAtMutatesMutableSequence) {
    int items[] = {1, 3};
    MutableArraySequence<int> sequence(items, 2);

    sequence.insert_at(2, 1);

    EXPECT_EQ(sequence.get(1), 2);
}

TEST(ArraySequenceTest, ConcatReturnsJoinedSequence) {
    int left_items[] = {1, 2};
    int right_items[] = {3, 4};
    MutableArraySequence<int> left(left_items, 2);
    MutableArraySequence<int> right(right_items, 2);

    Sequence<int> *joined = left.concat(&right);

    EXPECT_EQ(joined->get_count(), 4);
    EXPECT_EQ(joined->get_last(), 4);

    delete joined;
}

TEST(ArraySequenceTest, MapAppliesFunction) {
    int items[] = {1, 2, 3};
    MutableArraySequence<int> sequence(items, 3);

    Sequence<int> *mapped = sequence.map(utils::square);

    EXPECT_EQ(mapped->get(0), 1);
    EXPECT_EQ(mapped->get(1), 4);
    EXPECT_EQ(mapped->get(2), 9);

    delete mapped;
}

TEST(ArraySequenceTest, WhereFiltersValues) {
    int items[] = {-1, 0, 4};
    MutableArraySequence<int> sequence(items, 3);

    Sequence<int> *filtered = sequence.where(utils::is_positive);

    EXPECT_EQ(filtered->get_count(), 1);
    EXPECT_EQ(filtered->get_first(), 4);

    delete filtered;
}

TEST(ArraySequenceTest, ReduceCombinesValues) {
    int items[] = {1, 2, 3, 4};
    MutableArraySequence<int> sequence(items, 4);

    EXPECT_EQ(sequence.reduce(utils::sum, 0), 10);
}

TEST(ArraySequenceTest, SliceReplacesRequestedRange) {
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

TEST(ArraySequenceTest, ImmutableAppendReturnsNewSequence) {
    int items[] = {1, 2, 3};
    ImmutableArraySequence<int> sequence(items, 3);

    Sequence<int> *updated = sequence.append(4);

    EXPECT_EQ(sequence.get_count(), 3);
    EXPECT_EQ(updated->get_count(), 4);
    EXPECT_EQ(updated->get_last(), 4);

    delete updated;
}

TEST(ArraySequenceTest, EnumeratorWalksThroughSequence) {
    int items[] = {1, 2, 3};
    MutableArraySequence<int> sequence(items, 3);

    int total = 0;
    IEnumerator<int> *iterator = sequence.get_enumerator();
    while (iterator->move_next()) {
        total += iterator->get_current();
    }
    delete iterator;

    EXPECT_EQ(total, 6);
}

TEST(ArraySequenceTest, EnumeratorUsesLogicalCountAfterGrowth) {
    MutableArraySequence<int> sequence;
    sequence.append(1);
    sequence.append(2);
    sequence.append(3);

    int total = 0;
    IEnumerator<int> *iterator = sequence.get_enumerator();
    while (iterator->move_next()) {
        total += iterator->get_current();
    }
    delete iterator;

    EXPECT_EQ(sequence.get_count(), 3);
    EXPECT_EQ(total, 6);
}

TEST(ListSequenceTest, DefaultConstructorCreatesEmptySequence) {
    MutableListSequence<int> sequence;

    EXPECT_EQ(sequence.get_count(), 0);
}

TEST(ListSequenceTest, ItemsConstructorCopiesItems) {
    int items[] = {1, 2, 3};

    MutableListSequence<int> sequence(items, 3);

    EXPECT_EQ(sequence.get_count(), 3);
    EXPECT_EQ(sequence.get_first(), 1);
    EXPECT_EQ(sequence.get_last(), 3);
}

TEST(ListSequenceTest, CopyConstructorCopiesState) {
    int items[] = {2, 4, 6};
    MutableListSequence<int> original(items, 3);

    MutableListSequence<int> copy(original);
    original.append(8);

    EXPECT_EQ(copy.get_count(), 3);
    EXPECT_EQ(copy.get_last(), 6);
}

TEST(ListSequenceTest, GetFirstReturnsFirstItem) {
    int items[] = {9, 8};
    MutableListSequence<int> sequence(items, 2);

    EXPECT_EQ(sequence.get_first(), 9);
}

TEST(ListSequenceTest, GetLastReturnsLastItem) {
    int items[] = {9, 8};
    MutableListSequence<int> sequence(items, 2);

    EXPECT_EQ(sequence.get_last(), 8);
}

TEST(ListSequenceTest, GetReturnsValueByIndex) {
    int items[] = {9, 8, 7};
    MutableListSequence<int> sequence(items, 3);

    EXPECT_EQ(sequence.get(1), 8);
}

TEST(ListSequenceTest, TryGetReturnsNoneForInvalidIndex) {
    MutableListSequence<int> sequence;

    EXPECT_FALSE(sequence.try_get(0).has_value());
}

TEST(ListSequenceTest, GetCountReturnsLength) {
    int items[] = {1, 2, 3, 4};
    MutableListSequence<int> sequence(items, 4);

    EXPECT_EQ(sequence.get_count(), 4);
}

TEST(ListSequenceTest, GetSubSequenceReturnsRequestedRange) {
    int items[] = {1, 2, 3, 4};
    MutableListSequence<int> sequence(items, 4);

    Sequence<int> *subsequence = sequence.get_sub_sequence(1, 2);

    EXPECT_EQ(subsequence->get_count(), 2);
    EXPECT_EQ(subsequence->get_first(), 2);
    EXPECT_EQ(subsequence->get_last(), 3);

    delete subsequence;
}

TEST(ListSequenceTest, AppendMutatesMutableSequence) {
    MutableListSequence<int> sequence;

    Sequence<int> *result = sequence.append(5);

    EXPECT_EQ(result, &sequence);
    EXPECT_EQ(sequence.get_last(), 5);
}

TEST(ListSequenceTest, PrependMutatesMutableSequence) {
    int items[] = {2, 3};
    MutableListSequence<int> sequence(items, 2);

    sequence.prepend(1);

    EXPECT_EQ(sequence.get_first(), 1);
}

TEST(ListSequenceTest, InsertAtMutatesMutableSequence) {
    int items[] = {1, 3};
    MutableListSequence<int> sequence(items, 2);

    sequence.insert_at(2, 1);

    EXPECT_EQ(sequence.get(1), 2);
}

TEST(ListSequenceTest, ConcatReturnsJoinedSequence) {
    int left_items[] = {1, 2};
    int right_items[] = {3, 4};
    MutableListSequence<int> left(left_items, 2);
    MutableListSequence<int> right(right_items, 2);

    Sequence<int> *joined = left.concat(&right);

    EXPECT_EQ(joined->get_count(), 4);
    EXPECT_EQ(joined->get_last(), 4);

    delete joined;
}

TEST(ListSequenceTest, MapAppliesFunction) {
    int items[] = {1, 2, 3};
    MutableListSequence<int> sequence(items, 3);

    Sequence<int> *mapped = sequence.map(utils::square);

    EXPECT_EQ(mapped->get(0), 1);
    EXPECT_EQ(mapped->get(1), 4);
    EXPECT_EQ(mapped->get(2), 9);

    delete mapped;
}

TEST(ListSequenceTest, WhereFiltersValues) {
    int items[] = {-1, 0, 4};
    MutableListSequence<int> sequence(items, 3);

    Sequence<int> *filtered = sequence.where(utils::is_positive);

    EXPECT_EQ(filtered->get_count(), 1);
    EXPECT_EQ(filtered->get_first(), 4);

    delete filtered;
}

TEST(ListSequenceTest, ReduceCombinesValues) {
    int items[] = {1, 2, 3, 4};
    MutableListSequence<int> sequence(items, 4);

    EXPECT_EQ(sequence.reduce(utils::sum, 0), 10);
}

TEST(ListSequenceTest, SliceReplacesRequestedRange) {
    int items[] = {1, 2, 3, 4, 5};
    int replacement_items[] = {8, 9};
    MutableListSequence<int> sequence(items, 5);
    MutableArraySequence<int> replacement(replacement_items, 2);

    Sequence<int> *sliced = sequence.slice(1, 3, &replacement);

    EXPECT_EQ(sliced->get_count(), 4);
    EXPECT_EQ(sliced->get(0), 1);
    EXPECT_EQ(sliced->get(1), 8);
    EXPECT_EQ(sliced->get(2), 9);
    EXPECT_EQ(sliced->get(3), 5);

    delete sliced;
}

TEST(ListSequenceTest, ImmutableAppendReturnsNewSequence) {
    int items[] = {1, 2, 3};
    ImmutableListSequence<int> sequence(items, 3);

    Sequence<int> *updated = sequence.append(4);

    EXPECT_EQ(sequence.get_count(), 3);
    EXPECT_EQ(updated->get_count(), 4);
    EXPECT_EQ(updated->get_last(), 4);

    delete updated;
}

TEST(ListSequenceTest, EnumeratorWalksThroughSequence) {
    int items[] = {1, 2, 3};
    MutableListSequence<int> sequence(items, 3);

    int total = 0;
    IEnumerator<int> *iterator = sequence.get_enumerator();
    while (iterator->move_next()) {
        total += iterator->get_current();
    }
    delete iterator;

    EXPECT_EQ(total, 6);
}
