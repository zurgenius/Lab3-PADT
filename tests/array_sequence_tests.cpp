#include "array_sequence.h"
#include "utils.h"

#include <gtest/gtest.h>

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
