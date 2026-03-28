#include "algorithms.h"
#include "bit_sequence.h"
#include "sequence.h"

#include <gtest/gtest.h>

namespace {

int square(const int &value) { return value * value; }

bool is_even(const int &value) { return value % 2 == 0; }

int sum(const int &left, const int &right) { return left + right; }

bool is_zero(const int &value) { return value == 0; }

bool is_zero_bit(const Bit &bit) { return !bit.get(); }

} // namespace

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
  EnumeratorWrapper<int> it(list.get_enumerator());
  while (it.move_next()) {
    total += it.get_current();
  }

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

  Sequence<int> *mapped = sequence.map(square);
  Sequence<int> *filtered = sequence.where(is_even);
  int reduced = sequence.reduce(sum, 0);

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

TEST(BitTest, ImplementsBitwiseOperators) {
  Bit one(1);
  Bit zero(0);

  EXPECT_EQ((one & zero).get(), false);
  EXPECT_EQ((one | zero).get(), true);
  EXPECT_EQ((one ^ zero).get(), true);
  EXPECT_EQ((~one).get(), false);
}

TEST(BitSequenceTest, SupportsSequenceInterface) {
  Bit items[] = {Bit(1), Bit(0), Bit(1)};
  BitSequence sequence(items, 3);

  EXPECT_EQ(sequence.get_count(), 3);
  EXPECT_EQ(sequence[0], Bit(1));
  EXPECT_EQ(sequence.get_last(), Bit(1));
}

TEST(BitSequenceTest, SupportsBitwiseOperations) {
  Bit first_items[] = {Bit(1), Bit(0), Bit(1)};
  Bit second_items[] = {Bit(1), Bit(1), Bit(0)};
  BitSequence first(first_items, 3);
  BitSequence second(second_items, 3);

  BitSequence *and_result = first & second;
  BitSequence *xor_result = first ^ second;
  BitSequence *not_result = ~first;

  EXPECT_EQ(and_result->get(0), Bit(1));
  EXPECT_EQ(and_result->get(1), Bit(0));
  EXPECT_EQ(xor_result->get(1), Bit(1));
  EXPECT_EQ(not_result->get(0), Bit(0));

  delete and_result;
  delete xor_result;
  delete not_result;
}

TEST(BitSequenceTest, AppliesMaskToSequence) {
  Bit mask_items[] = {Bit(1), Bit(0), Bit(1), Bit(0)};
  int values[] = {10, 20, 30, 40};
  BitSequence mask(mask_items, 4);
  MutableArraySequence<int> sequence(values, 4);

  Sequence<int> *filtered = mask.apply_mask(&sequence);
  EXPECT_EQ(filtered->get_count(), 2);
  EXPECT_EQ(filtered->get(0), 10);
  EXPECT_EQ(filtered->get(1), 30);

  delete filtered;
}

TEST(BitSequenceTest, RejectsMaskWithWrongLength) {
  Bit mask_items[] = {Bit(1), Bit(0)};
  int values[] = {1, 2, 3};
  BitSequence mask(mask_items, 2);
  MutableArraySequence<int> sequence(values, 3);

  EXPECT_THROW(mask.apply_mask(&sequence), std::invalid_argument);
}

TEST(AlgorithmsTest, SplitProducesChunksBetweenDelimiters) {
  int items[] = {1, 2, 0, 3, 0, 4};
  MutableArraySequence<int> sequence(items, 6);

  Sequence<Sequence<int> *> *groups = split(&sequence, is_zero);
  EXPECT_EQ(groups->get_count(), 3);
  EXPECT_EQ(groups->get(0)->get_count(), 2);
  EXPECT_EQ(groups->get(1)->get_count(), 1);
  EXPECT_EQ(groups->get(2)->get(0), 4);

  for (int index = 0; index < groups->get_count(); index++) {
    delete groups->get(index);
  }
  delete groups;
}

TEST(AlgorithmsTest, SplitWorksForBitSequenceToo) {
  Bit bits[] = {Bit(1), Bit(0), Bit(1), Bit(1)};
  BitSequence sequence(bits, 4);

  Sequence<Sequence<Bit> *> *groups = split(&sequence, is_zero_bit);
  EXPECT_EQ(groups->get_count(), 2);
  EXPECT_EQ(groups->get(0)->get_count(), 1);
  EXPECT_EQ(groups->get(1)->get_count(), 2);

  for (int index = 0; index < groups->get_count(); index++) {
    delete groups->get(index);
  }
  delete groups;
}

TEST(AlgorithmsTest, ComputesMinMaxAvg) {
  int items[] = {4, 1, 7, 2};
  MutableListSequence<int> sequence(items, 4);

  const Stats stats = min_max_avg(&sequence);
  EXPECT_EQ(stats.min, 1);
  EXPECT_EQ(stats.max, 7);
  EXPECT_EQ(stats.avg, 3.5);
}

TEST(AlgorithmsTest, RejectsEmptySequenceForStats) {
  MutableArraySequence<int> sequence;
  EXPECT_THROW(min_max_avg(&sequence), std::invalid_argument);
}
