#ifndef MINI_GTEST_GTEST_H
#define MINI_GTEST_GTEST_H

#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace testing {

struct TestInfo {
    std::string suite_name;
    std::string test_name;
    std::function<void()> func;
};

class TestFailure : public std::exception {
public:
    explicit TestFailure(std::string message) : message_(std::move(message)) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

inline std::vector<TestInfo>& registry() {
    static std::vector<TestInfo> tests;
    return tests;
}

inline int& current_failures() {
    static int failures = 0;
    return failures;
}

inline void add_failure(const char* file, int line, const std::string& message) {
    std::cerr << file << ":" << line << ": Failure\n" << message << std::endl;
    current_failures()++;
}

template <class Left, class Right>
inline void expect_eq(const Left& left, const Right& right, const char* left_expr,
                      const char* right_expr, const char* file, int line) {
    if (!(left == right)) {
        std::ostringstream oss;
        oss << "Expected equality of these values:\n  " << left_expr << "\n  "
            << right_expr;
        add_failure(file, line, oss.str());
    }
}

inline void expect_true(bool value, const char* expr, const char* file, int line) {
    if (!value) {
        std::ostringstream oss;
        oss << "Expected true: " << expr;
        add_failure(file, line, oss.str());
    }
}

inline void expect_false(bool value, const char* expr, const char* file, int line) {
    if (value) {
        std::ostringstream oss;
        oss << "Expected false: " << expr;
        add_failure(file, line, oss.str());
    }
}

template <class Exception, class Func>
inline void expect_throw(Func&& func, const char* expr, const char* exception_name,
                         const char* file, int line) {
    bool thrown = false;
    try {
        func();
    } catch (const Exception&) {
        thrown = true;
    } catch (...) {
        std::ostringstream oss;
        oss << "Expected " << exception_name
            << " but a different exception was thrown by " << expr;
        add_failure(file, line, oss.str());
        return;
    }

    if (!thrown) {
        std::ostringstream oss;
        oss << "Expected " << exception_name << " to be thrown by " << expr;
        add_failure(file, line, oss.str());
    }
}

class TestRegistrar {
public:
    TestRegistrar(const char* suite_name, const char* test_name,
                  std::function<void()> func) {
        registry().push_back({suite_name, test_name, std::move(func)});
    }
};

inline void InitGoogleTest(int*, char**) {}

inline int RUN_ALL_TESTS() {
    int failed_tests = 0;
    std::cout << "[==========] Running " << registry().size() << " tests." << std::endl;

    for (const TestInfo& test : registry()) {
        std::cout << "[ RUN      ] " << test.suite_name << "." << test.test_name
                  << std::endl;
        current_failures() = 0;

        try {
            test.func();
        } catch (const std::exception& e) {
            add_failure(__FILE__, __LINE__, e.what());
        } catch (...) {
            add_failure(__FILE__, __LINE__, "Unknown exception");
        }

        if (current_failures() == 0) {
            std::cout << "[       OK ] " << test.suite_name << "." << test.test_name
                      << std::endl;
        } else {
            std::cout << "[  FAILED  ] " << test.suite_name << "." << test.test_name
                      << std::endl;
            failed_tests++;
        }
    }

    std::cout << "[==========] " << registry().size() << " tests ran." << std::endl;
    if (failed_tests == 0) {
        std::cout << "[  PASSED  ] All tests passed." << std::endl;
    } else {
        std::cout << "[  FAILED  ] " << failed_tests << " tests." << std::endl;
    }
    return failed_tests == 0 ? 0 : 1;
}

}  // namespace testing

#define TEST(SuiteName, TestName)                                              \
    void SuiteName##_##TestName##_Test();                                      \
    static ::testing::TestRegistrar SuiteName##_##TestName##_registrar(        \
        #SuiteName, #TestName, &SuiteName##_##TestName##_Test);                \
    void SuiteName##_##TestName##_Test()

#define EXPECT_EQ(val1, val2)                                                  \
    ::testing::expect_eq((val1), (val2), #val1, #val2, __FILE__, __LINE__)

#define EXPECT_TRUE(expr)                                                      \
    ::testing::expect_true(static_cast<bool>(expr), #expr, __FILE__, __LINE__)

#define EXPECT_FALSE(expr)                                                     \
    ::testing::expect_false(static_cast<bool>(expr), #expr, __FILE__, __LINE__)

#define EXPECT_THROW(stmt, exception_type)                                     \
    ::testing::expect_throw<exception_type>(                                   \
        [&]() { static_cast<void>(stmt); }, #stmt, #exception_type, __FILE__,  \
        __LINE__)

#endif
