#include "array_sequence.h"
#include "interpolator.h"

#include <gtest/gtest.h>

namespace {

class TestInterpolator : public Interpolator<double> {
  public:
    Sequence<Function<double> *> *
    interpolate(const Sequence<Point<double>> & /*points*/) const override {
        return new MutableArraySequence<Function<double> *>();
    }
};

Function<double> *make_segment(double left, double right, const double *coefficients,
                               int coefficient_count) {
    DynamicArray<double> coeffs(coefficients, coefficient_count);
    return new PolynomialFunction<double>(left, right, coeffs);
}

} // namespace

TEST(InterpolatorEvaluateTest, ReturnsNoneForEmptySegments) {
    TestInterpolator interpolator;
    MutableArraySequence<Function<double> *> segments;

    const Option<double> value = interpolator.evaluate(segments, 1.0);

    EXPECT_FALSE(value.has_value());
}

TEST(InterpolatorEvaluateTest, ReturnsNoneOutsideDomain) {
    TestInterpolator interpolator;
    double coefficients[] = {1.0};
    Function<double> *segment_items[] = {make_segment(0.0, 1.0, coefficients, 1)};
    MutableArraySequence<Function<double> *> segments(segment_items, 1);

    EXPECT_FALSE(interpolator.evaluate(segments, -0.1).has_value());
    EXPECT_FALSE(interpolator.evaluate(segments, 1.1).has_value());

    delete segment_items[0];
}

TEST(InterpolatorEvaluateTest, SelectsSegmentWithBinarySearchAndLeftBoundaryRule) {
    TestInterpolator interpolator;
    double left_coefficients[] = {10.0, 1.0};
    double right_coefficients[] = {100.0, 10.0};
    Function<double> *segment_items[] = {
        make_segment(0.0, 1.0, left_coefficients, 2),
        make_segment(1.0, 2.0, right_coefficients, 2),
    };
    MutableArraySequence<Function<double> *> segments(segment_items, 2);

    const Option<double> boundary = interpolator.evaluate(segments, 1.0);
    const Option<double> middle = interpolator.evaluate(segments, 1.5);

    ASSERT_TRUE(boundary.has_value());
    ASSERT_TRUE(middle.has_value());
    EXPECT_DOUBLE_EQ(boundary.get_value(), 11.0);
    EXPECT_DOUBLE_EQ(middle.get_value(), 105.0);

    delete segment_items[0];
    delete segment_items[1];
}

TEST(InterpolatorEvaluateTest, EvaluatesPolynomialOfAnyDegreeRelativeToLeftBoundary) {
    TestInterpolator interpolator;
    double coefficients[] = {1.0, 2.0, 3.0, 4.0};
    Function<double> *segment_items[] = {make_segment(2.0, 4.0, coefficients, 4)};
    MutableArraySequence<Function<double> *> segments(segment_items, 1);

    const Option<double> value = interpolator.evaluate(segments, 3.0);

    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(value.get_value(), 10.0);

    delete segment_items[0];
}
