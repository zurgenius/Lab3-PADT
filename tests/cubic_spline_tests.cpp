#include "array_sequence.h"
#include "cubic_spline.h"
#include "linear_spline.h"

#include <gtest/gtest.h>
#include <stdexcept>

namespace {

class TestInterpolator : public Interpolator<double> {
  public:
    Sequence<FunctionSegment<double>> *
    interpolate(const Sequence<Point<double>> & /*points*/) const override {
        return new MutableArraySequence<FunctionSegment<double>>();
    }
};

FunctionSegment<double> make_segment(double left, double right, const double *coefficients,
                                     int coefficient_count) {
    return FunctionSegment<double>{left, right,
                                   DynamicArray<double>(coefficients, coefficient_count)};
}

} // namespace

TEST(InterpolatorEvaluateTest, ReturnsNoneForEmptySegments) {
    TestInterpolator interpolator;
    MutableArraySequence<FunctionSegment<double>> segments;

    const Option<double> value = interpolator.evaluate(segments, 1.0);

    EXPECT_FALSE(value.has_value());
}

TEST(InterpolatorEvaluateTest, ReturnsNoneOutsideDomain) {
    TestInterpolator interpolator;
    double coefficients[] = {1.0};
    FunctionSegment<double> segment_items[] = {make_segment(0.0, 1.0, coefficients, 1)};
    MutableArraySequence<FunctionSegment<double>> segments(segment_items, 1);

    EXPECT_FALSE(interpolator.evaluate(segments, -0.1).has_value());
    EXPECT_FALSE(interpolator.evaluate(segments, 1.1).has_value());
}

TEST(InterpolatorEvaluateTest, SelectsSegmentWithBinarySearchAndLeftBoundaryRule) {
    TestInterpolator interpolator;
    double left_coefficients[] = {10.0, 1.0};
    double right_coefficients[] = {100.0, 10.0};
    FunctionSegment<double> segment_items[] = {
        make_segment(0.0, 1.0, left_coefficients, 2),
        make_segment(1.0, 2.0, right_coefficients, 2),
    };
    MutableArraySequence<FunctionSegment<double>> segments(segment_items, 2);

    const Option<double> boundary = interpolator.evaluate(segments, 1.0);
    const Option<double> middle = interpolator.evaluate(segments, 1.5);

    ASSERT_TRUE(boundary.has_value());
    ASSERT_TRUE(middle.has_value());
    EXPECT_DOUBLE_EQ(boundary.get_value(), 11.0);
    EXPECT_DOUBLE_EQ(middle.get_value(), 105.0);
}

TEST(InterpolatorEvaluateTest, EvaluatesPolynomialOfAnyDegreeRelativeToLeftBoundary) {
    TestInterpolator interpolator;
    double coefficients[] = {1.0, 2.0, 3.0, 4.0};
    FunctionSegment<double> segment_items[] = {make_segment(2.0, 4.0, coefficients, 4)};
    MutableArraySequence<FunctionSegment<double>> segments(segment_items, 1);

    const Option<double> value = interpolator.evaluate(segments, 3.0);

    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(value.get_value(), 10.0);
}

TEST(CubicSplineInterpolatorTest, InterpolateThrowsWhenTooFewPoints) {
    Point<double> point_items[] = {{0.0, 0.0}, {1.0, 1.0}};
    MutableArraySequence<Point<double>> points(point_items, 2);
    CubicSplineInterpolator<double> interpolator;

    EXPECT_THROW(interpolator.interpolate(points), std::invalid_argument);
}

TEST(CubicSplineInterpolatorTest, InterpolateThrowsWhenXValuesAreNotIncreasing) {
    Point<double> equal_x_items[] = {{0.0, 0.0}, {1.0, 1.0}, {1.0, 2.0}};
    Point<double> descending_x_items[] = {{0.0, 0.0}, {2.0, 1.0}, {1.0, 2.0}};
    MutableArraySequence<Point<double>> equal_x_points(equal_x_items, 3);
    MutableArraySequence<Point<double>> descending_x_points(descending_x_items, 3);
    CubicSplineInterpolator<double> interpolator;

    EXPECT_THROW(interpolator.interpolate(equal_x_points), std::invalid_argument);
    EXPECT_THROW(interpolator.interpolate(descending_x_points), std::invalid_argument);
}

TEST(CubicSplineInterpolatorTest, CreatesOneSegmentBetweenEachPairOfPoints) {
    Point<double> point_items[] = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};
    MutableArraySequence<Point<double>> points(point_items, 4);
    CubicSplineInterpolator<double> interpolator;

    Sequence<FunctionSegment<double>> *segments = interpolator.interpolate(points);

    ASSERT_EQ(segments->get_count(), 3);
    EXPECT_DOUBLE_EQ(segments->get(0).left, 0.0);
    EXPECT_DOUBLE_EQ(segments->get(0).right, 1.0);
    EXPECT_DOUBLE_EQ(segments->get(1).left, 1.0);
    EXPECT_DOUBLE_EQ(segments->get(1).right, 2.0);
    EXPECT_DOUBLE_EQ(segments->get(2).left, 2.0);
    EXPECT_DOUBLE_EQ(segments->get(2).right, 3.0);
    delete segments;
}

TEST(CubicSplineInterpolatorTest, EvaluatedSplinePassesThroughSourcePoints) {
    Point<double> point_items[] = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};
    MutableArraySequence<Point<double>> points(point_items, 4);
    CubicSplineInterpolator<double> interpolator;

    Sequence<FunctionSegment<double>> *segments = interpolator.interpolate(points);

    for (int index = 0; index < points.get_count(); ++index) {
        const Option<double> value = interpolator.evaluate(*segments, points.get(index).x);
        ASSERT_TRUE(value.has_value());
        EXPECT_NEAR(value.get_value(), points.get(index).y, 1e-12);
    }
    delete segments;
}

TEST(CubicSplineInterpolatorTest, LinearFunctionProducesLinearSegments) {
    Point<double> point_items[] = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};
    MutableArraySequence<Point<double>> points(point_items, 4);
    CubicSplineInterpolator<double> interpolator;

    Sequence<FunctionSegment<double>> *segments = interpolator.interpolate(points);

    ASSERT_EQ(segments->get_count(), 3);
    for (int index = 0; index < segments->get_count(); ++index) {
        const FunctionSegment<double> &segment = segments->get(index);
        ASSERT_EQ(segment.coefficients.get_size(), 4);
        EXPECT_NEAR(segment.coefficients.get(0), 1.0 + 2.0 * segment.left, 1e-12);
        EXPECT_NEAR(segment.coefficients.get(1), 2.0, 1e-12);
        EXPECT_NEAR(segment.coefficients.get(2), 0.0, 1e-12);
        EXPECT_NEAR(segment.coefficients.get(3), 0.0, 1e-12);
    }
    delete segments;
}

TEST(CubicSplineInterpolatorTest, EvaluatesLinearFunctionBetweenNodes) {
    Point<double> point_items[] = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};
    MutableArraySequence<Point<double>> points(point_items, 4);
    CubicSplineInterpolator<double> interpolator;

    Sequence<FunctionSegment<double>> *segments = interpolator.interpolate(points);

    const Option<double> first = interpolator.evaluate(*segments, 0.5);
    const Option<double> second = interpolator.evaluate(*segments, 1.5);
    const Option<double> third = interpolator.evaluate(*segments, 2.5);

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    ASSERT_TRUE(third.has_value());
    EXPECT_NEAR(first.get_value(), 2.0, 1e-12);
    EXPECT_NEAR(second.get_value(), 4.0, 1e-12);
    EXPECT_NEAR(third.get_value(), 6.0, 1e-12);
    delete segments;
}

TEST(LinearSplineInterpolatorTest, InterpolateThrowsWhenTooFewPoints) {
    Point<double> point_items[] = {{0.0, 0.0}};
    MutableArraySequence<Point<double>> points(point_items, 1);
    LinearSplineInterpolator<double> interpolator;

    EXPECT_THROW(interpolator.interpolate(points), std::invalid_argument);
}

TEST(LinearSplineInterpolatorTest, InterpolateThrowsWhenXValuesAreNotIncreasing) {
    Point<double> equal_x_items[] = {{0.0, 0.0}, {0.0, 1.0}};
    Point<double> descending_x_items[] = {{1.0, 0.0}, {0.0, 1.0}};
    MutableArraySequence<Point<double>> equal_x_points(equal_x_items, 2);
    MutableArraySequence<Point<double>> descending_x_points(descending_x_items, 2);
    LinearSplineInterpolator<double> interpolator;

    EXPECT_THROW(interpolator.interpolate(equal_x_points), std::invalid_argument);
    EXPECT_THROW(interpolator.interpolate(descending_x_points), std::invalid_argument);
}

TEST(LinearSplineInterpolatorTest, CreatesLinearSegmentBetweenEachPairOfPoints) {
    Point<double> point_items[] = {{0.0, 1.0}, {2.0, 5.0}, {3.0, 8.0}};
    MutableArraySequence<Point<double>> points(point_items, 3);
    LinearSplineInterpolator<double> interpolator;

    Sequence<FunctionSegment<double>> *segments = interpolator.interpolate(points);

    ASSERT_EQ(segments->get_count(), 2);

    const FunctionSegment<double> &first = segments->get(0);
    EXPECT_DOUBLE_EQ(first.left, 0.0);
    EXPECT_DOUBLE_EQ(first.right, 2.0);
    ASSERT_EQ(first.coefficients.get_size(), 2);
    EXPECT_DOUBLE_EQ(first.coefficients.get(0), 1.0);
    EXPECT_DOUBLE_EQ(first.coefficients.get(1), 2.0);

    const FunctionSegment<double> &second = segments->get(1);
    EXPECT_DOUBLE_EQ(second.left, 2.0);
    EXPECT_DOUBLE_EQ(second.right, 3.0);
    ASSERT_EQ(second.coefficients.get_size(), 2);
    EXPECT_DOUBLE_EQ(second.coefficients.get(0), 5.0);
    EXPECT_DOUBLE_EQ(second.coefficients.get(1), 3.0);

    delete segments;
}

TEST(LinearSplineInterpolatorTest, EvaluatedSplinePassesThroughSourcePointsAndBetweenNodes) {
    Point<double> point_items[] = {{0.0, 1.0}, {2.0, 5.0}, {3.0, 8.0}};
    MutableArraySequence<Point<double>> points(point_items, 3);
    LinearSplineInterpolator<double> interpolator;

    Sequence<FunctionSegment<double>> *segments = interpolator.interpolate(points);

    for (int index = 0; index < points.get_count(); ++index) {
        const Option<double> value = interpolator.evaluate(*segments, points.get(index).x);
        ASSERT_TRUE(value.has_value());
        EXPECT_NEAR(value.get_value(), points.get(index).y, 1e-12);
    }

    const Option<double> first_middle = interpolator.evaluate(*segments, 1.0);
    const Option<double> second_middle = interpolator.evaluate(*segments, 2.5);

    ASSERT_TRUE(first_middle.has_value());
    ASSERT_TRUE(second_middle.has_value());
    EXPECT_NEAR(first_middle.get_value(), 3.0, 1e-12);
    EXPECT_NEAR(second_middle.get_value(), 6.5, 1e-12);

    delete segments;
}
