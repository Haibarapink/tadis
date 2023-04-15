#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// pure_assert is a macro that is used to create an instance of the Assertion
// class. The Assertion class is used to check a boolean condition and print an
// error message if // // the condition is false.
// @usage : pure_assert(false) << "condition is false" << false;
#define pure_assert(condition) pure_test::Assertion(condition) << " [" << __FILE__ << ":" << __LINE__ << "] [message: "
#define pure_report(condition)                                                                     \
  pure_test::Reporter(condition) << " [" << __FILE__ << ":" << __LINE__ << " [" << __func__ << "]" \
                                 << "[message: "
namespace pure_test {

// Time and print format be like "usr 0.0xs sys 0.0xs", and set start and end ,
// print the end - start
class Timer {
public:
  Timer()
  {
    start_ = std::chrono::system_clock::now();
  }
  ~Timer()
  {}

  void start()
  {
    start_ = std::chrono::system_clock::now();
  }

  std::string end()
  {
    end_ = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_ - start_;
    return std::to_string(elapsed_seconds.count());
  }

private:
  std::chrono::time_point<std::chrono::system_clock> start_;
  std::chrono::time_point<std::chrono::system_clock> end_;
};

using Func = std::function<void()>;

class Reporter {
public:
  Reporter(bool cnd) : cnd_(cnd)
  {}

  ~Reporter()
  {
    if (cnd_) {
      // std::cout << "\033[32m[PASS] "
      //           << "\033[0m" << std::endl;
      pass_count_++;
    } else {
      std::cout << "\033[31m[FAIL]" << ss_.str() << "]\033[0m" << std::endl;
      fail_count_++;
    }
  }

  template <typename T>
  Reporter &operator<<(const T &value)
  {
    if (!cnd_) {
      ss_ << value;
    }
    return *this;
  }

  static void print_summary()
  {
    std::cout << "Summary: " << pass_count_ << " passed, " << fail_count_ << " failed." << std::endl;
  }

  static size_t pass_count_;
  static size_t fail_count_;

public:
  std::stringstream ss_;
  bool cnd_;
};

/// @brief A class for checking boolean conditions and printing error messages
/// if the condition is false.
class Assertion {
public:
  Assertion(bool condition) : condition_(condition)
  {}

  template <typename T>
  Assertion &operator<<(const T &value)
  {
    if (!condition_) {
      message_ << value;
    }
    return *this;
  }

  ~Assertion()
  {
    if (!condition_) {
      std::cerr << "[assert fail]" << message_.str() << "]" << std::endl;
      std::abort();
    }
  }

private:
  bool condition_;
  std::stringstream message_;
};

class TestCase {
public:
  std::string name;
  Func func;
};

// Test suite class
class TestSuite {
public:
  // Add a test case to the suite
  void add_test_case(const std::string &name, Func test_case)
  {
    test_cases_.push_back({name, std::move(test_case)});
  }

  // Run all test cases in the suite
  void run()
  {
    std::cout << "Running " << test_cases_.size() << " test cases...\n";
    for (const auto &test_case : test_cases_) {
      std::cout << "Running test case: " << test_case.name << "...\n";
      auto fail = Reporter::fail_count_;
      Timer t;
      test_case.func();
      auto time_cost = t.end();
      if (fail == Reporter::fail_count_) {
        std::cout << "\033[32m[PASS] "
                  << "\033[0m" << std::endl;
        std::cout << "Time cost: " << time_cost << "s" << std::endl;
      }
    }

    if (Reporter::fail_count_ == 0)
      std::cout << "All test cases passed!\n";
    else
      Reporter::print_summary();
  }

private:
  std::vector<TestCase> test_cases_;
};
}  // namespace pure_test

#define PURE_TEST_INIT()                       \
  size_t pure_test::Reporter::fail_count_ = 0; \
  size_t pure_test::Reporter::pass_count_ = 0;
// Define test macros
#define PURE_TEST_EQ(a, b) pure_assert((a) == (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_NE(a, b) pure_assert((a) != (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_GT(a, b) pure_assert((a) > (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_GE(a, b) pure_assert((a) >= (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_LT(a, b) pure_assert((a) < (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_LE(a, b) pure_assert((a) <= (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_TRUE(a) pure_assert((a)) << "Expected: true, actual: false"
#define PURE_TEST_FALSE(a) pure_assert(!(a)) << "Expected: false, actual: true"
// report the test result
#define PURE_TEST_REPORT(a) pure_report((a)) << "Expected: true, actual: false"
#define PURE_TEST_EQ_REPORT(a, b) pure_report((a) == (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_NE_REPORT(a, b) pure_report((a) != (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_GT_REPORT(a, b) pure_report((a) > (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_GE_REPORT(a, b) pure_report((a) >= (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_LT_REPORT(a, b) pure_report((a) < (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_LE_REPORT(a, b) pure_report((a) <= (b)) << "Expected: " << (a) << ", actual: " << (b)
#define PURE_TEST_TRUE_REPORT(a) pure_report((a)) << "Expected: true, actual: false"
#define PURE_TEST_FALSE_REPORT(a) pure_report(!(a)) << "Expected: false, actual: true"

#define PURE_TEST_PREPARE() pure_test::TestSuite test_suite;
#define PURE_TEST_CASE(name) test_suite.add_test_case(#name, name);
#define PURE_TEST_RUN() test_suite.run();