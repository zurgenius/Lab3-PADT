#include "array_sequence.h"
#include "cubic_spline.h"

#include <gtest/gtest.h>
#include <stdexcept>

Point<double> add_one(const Point<double> &value) {
    Point<double> result{value.x, value.y + 1.0};
    return result;
}

bool is_positive_double(const Point<double> &value) { return value.y > 0.0; }

Point<double> sum_double(const Point<double> &left, const Point<double> &right) {
    Point<double> result{left.x, left.y + right.y};
    return result;
}

TEST(CubicSplineTest, BuildThrowsWhenTooFewPoints) {
    Point<double> points[] = {{0.0, 0.0}, {1.0, 1.0}};
    MutableCubicSpline<double> spline;

    EXPECT_THROW(spline.build(points, 2), std::invalid_argument);
}

TEST(CubicSplineTest, EvaluateMatchesNodesForLinearFunction) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 4);

    EXPECT_NEAR(spline.evaluate(0.0), 1.0, 1e-12);
    EXPECT_NEAR(spline.evaluate(1.0), 3.0, 1e-12);
    EXPECT_NEAR(spline.evaluate(2.0), 5.0, 1e-12);
    EXPECT_NEAR(spline.evaluate(3.0), 7.0, 1e-12);
}

TEST(CubicSplineTest, EvaluateMatchesLinearFunctionBetweenNodes) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 4);

    EXPECT_NEAR(spline.evaluate(0.5), 2.0, 1e-12);
    EXPECT_NEAR(spline.evaluate(1.5), 4.0, 1e-12);
    EXPECT_NEAR(spline.evaluate(2.5), 6.0, 1e-12);
}

TEST(CubicSplineTest, GetSegmentCountReturnsZeroBeforeBuild) {
    MutableCubicSpline<double> spline;

    EXPECT_EQ(spline.get_segment_count(), 0);
}

TEST(CubicSplineTest, GetSegmentReturnsPolynomialCoefficients) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 4);

    EXPECT_EQ(spline.get_segment_count(), 3);

    const SplineSegment<double> first = spline.get_segment(0);
    EXPECT_DOUBLE_EQ(first.left_x, 0.0);
    EXPECT_DOUBLE_EQ(first.right_x, 1.0);
    EXPECT_DOUBLE_EQ(first.a, 1.0);
    EXPECT_DOUBLE_EQ(first.b, 2.0);
    EXPECT_DOUBLE_EQ(first.c, 0.0);
    EXPECT_DOUBLE_EQ(first.d, 0.0);

    const SplineSegment<double> second = spline.get_segment(1);
    EXPECT_DOUBLE_EQ(second.left_x, 1.0);
    EXPECT_DOUBLE_EQ(second.right_x, 2.0);
    EXPECT_DOUBLE_EQ(second.a, 3.0);
    EXPECT_DOUBLE_EQ(second.b, 2.0);
    EXPECT_DOUBLE_EQ(second.c, 0.0);
    EXPECT_DOUBLE_EQ(second.d, 0.0);
}

TEST(CubicSplineTest, TryGetSegmentReturnsNoneForInvalidIndex) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 4);

    EXPECT_FALSE(spline.try_get_segment(-1).has_value());
    EXPECT_FALSE(spline.try_get_segment(3).has_value());
    EXPECT_THROW(spline.get_segment(3), std::out_of_range);
}

TEST(CubicSplineTest, ImmutableAppendReturnsNewSpline) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 2.0}, {2.0, 3.0}};
    ImmutableCubicSpline<double> spline;

    spline.build(points, 3);

    Sequence<Point<double>> *updated = spline.append(Point<double>{3.0, 4.0});
    EXPECT_EQ(spline.get_count(), 3);
    EXPECT_EQ(updated->get_count(), 4);
    EXPECT_DOUBLE_EQ(spline.get_last().y, 3.0);
    EXPECT_DOUBLE_EQ(updated->get_last().y, 4.0);
    delete updated;
}

TEST(CubicSplineSequenceTest, AccessorsReturnFValues) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 2.0}, {2.0, 3.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 3);

    Sequence<Point<double>> *seq = &spline;
    EXPECT_EQ(seq->get_count(), 3);
    EXPECT_DOUBLE_EQ(seq->get_first().y, 1.0);
    EXPECT_DOUBLE_EQ(seq->get_last().y, 3.0);
    EXPECT_DOUBLE_EQ(seq->get(1).y, 2.0);
    EXPECT_TRUE(seq->try_get_first().has_value());
    EXPECT_DOUBLE_EQ(seq->try_get(1).get_value().y, 2.0);
}

TEST(CubicSplineSequenceTest, AppendAddsValueToEnd) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 2.0}, {2.0, 3.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 3);
    Point<double> appended{3.0, 4.0};
    spline.append(appended);

    EXPECT_EQ(spline.get_count(), 4);
    EXPECT_DOUBLE_EQ(spline.get_last().y, 4.0);
    EXPECT_DOUBLE_EQ(spline.get(3).y, 4.0);
}

TEST(CubicSplineSequenceTest, PrependAddsValueToStart) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 2.0}, {2.0, 3.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 3);
    Point<double> prepended{-1.0, 0.0};
    spline.prepend(prepended);

    EXPECT_EQ(spline.get_count(), 4);
    EXPECT_DOUBLE_EQ(spline.get_first().y, 0.0);
    EXPECT_DOUBLE_EQ(spline.get(1).y, 1.0);
}

TEST(CubicSplineSequenceTest, InsertAtAddsValueInMiddle) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 2.0}, {2.0, 3.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 3);
    Point<double> inserted{0.5, 1.5};
    spline.insert_at(inserted, 1);

    EXPECT_EQ(spline.get_count(), 4);
    EXPECT_DOUBLE_EQ(spline.get(1).y, 1.5);
    EXPECT_DOUBLE_EQ(spline.get(2).y, 2.0);
}

TEST(CubicSplineSequenceTest, MapWhereReduceWorkOnSpline) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, -2.0}, {2.0, 3.0}, {3.0, -4.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 4);

    Sequence<Point<double>> *mapped = spline.map(add_one);
    EXPECT_DOUBLE_EQ(mapped->get(0).y, 2.0);
    EXPECT_DOUBLE_EQ(mapped->get(1).y, -1.0);
    delete mapped;

    Sequence<Point<double>> *filtered = spline.where(is_positive_double);
    EXPECT_EQ(filtered->get_count(), 2);
    EXPECT_DOUBLE_EQ(filtered->get(0).y, 1.0);
    EXPECT_DOUBLE_EQ(filtered->get(1).y, 3.0);
    delete filtered;

    Point<double> reduced = spline.reduce(sum_double, Point<double>{0.0, 0.0});
    EXPECT_DOUBLE_EQ(reduced.y, -2.0);
}

TEST(CubicSplineSequenceTest, ConcatCreatesCombinedSequence) {
    Point<double> left_points[] = {{0.0, 1.0}, {1.0, 2.0}, {2.0, 3.0}};
    Point<double> right_points[] = {{0.0, 4.0}, {1.0, 5.0}, {2.0, 6.0}};
    MutableCubicSpline<double> left;
    MutableCubicSpline<double> right;

    left.build(left_points, 3);
    right.build(right_points, 3);

    Sequence<Point<double>> *joined = left.concat(&right);
    EXPECT_EQ(joined->get_count(), 6);
    EXPECT_DOUBLE_EQ(joined->get(2).y, 3.0);
    EXPECT_DOUBLE_EQ(joined->get(3).y, 4.0);
    delete joined;
}

TEST(CubicSplineSequenceTest, SubSequenceAndSliceFollowSequenceRules) {
    Point<double> points[] = {{0.0, 1.0}, {1.0, 2.0}, {2.0, 3.0}, {3.0, 4.0}, {4.0, 5.0}};
    MutableCubicSpline<double> spline;

    spline.build(points, 5);

    Sequence<Point<double>> *subseq = spline.get_sub_sequence(1, 3);
    EXPECT_EQ(subseq->get_count(), 3);
    EXPECT_DOUBLE_EQ(subseq->get(0).y, 2.0);
    EXPECT_DOUBLE_EQ(subseq->get(2).y, 4.0);
    delete subseq;

    Point<double> replacement_items[] = {{1.5, 8.0}, {2.5, 9.0}};
    MutableArraySequence<Point<double>> replacement(replacement_items, 2);
    Sequence<Point<double>> *sliced = spline.slice(1, 2, &replacement);
    EXPECT_EQ(sliced->get_count(), 5);
    EXPECT_DOUBLE_EQ(sliced->get(0).y, 1.0);
    EXPECT_DOUBLE_EQ(sliced->get(1).y, 8.0);
    EXPECT_DOUBLE_EQ(sliced->get(2).y, 9.0);
    EXPECT_DOUBLE_EQ(sliced->get(3).y, 4.0);
    EXPECT_DOUBLE_EQ(sliced->get(4).y, 5.0);
    delete sliced;
}
