// Test library configuration for microsoft_gsl.cfg
//
// Usage:
// $ cppcheck --check-library --library=microsoft_gsl --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr --suppress=autoNoType test/cfg/microsoft_gsl.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// C++ Standard Library
#include <vector>

// Guideline Support Library
#include <gsl/gsl>

struct owner_type
{
  owner_type() = default;
  owner_type(const owner_type &) = delete;
  owner_type(owner_type &&) = default;

  ~owner_type() { delete ptr_; }

  auto operator=(const owner_type &) -> owner_type & = delete;
  auto operator=(owner_type &&) -> owner_type & = default;

private:
  gsl::owner<int *> ptr_{new int(42)};
};

auto pre_and_post_condition_test(int i) -> int
{
  Expects(i > 0);

  const auto result{i * 2};

  Ensures(result > 0);
  return result;
}

auto suppress_macro_test(std::span<int> s) -> int
{
  GSL_SUPPRESS("bounds.1")
  return s[0];
}

auto iterate_over_container_test(const std::vector<int> &v) -> int
{
  int sum{0};

  for (gsl::index i{0}; i < v.size(); ++i)
  {
    sum += v[i];
  }
  return sum;
}

void not_null_test(gsl::not_null<int *> p)
{
  Expects(p != nullptr);
  *p = 42;
}

void strict_not_null_test(gsl::strict_not_null<int *> p)
{
  Expects(p != nullptr);
  *p = 42;
}

auto byte_test(gsl::byte b) -> gsl::byte
{
  return b | 0x81;
}
