# algorithmOutOfBounds

**Message**: The algorithm 'std::copy' accesses 5 elements through the iterator 'v1.begin()' but only 3 elements are available.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

Many STL algorithms take an iterator that denotes the beginning of a second range (typically an output range) and
assume that this range is large enough. If it is not, the algorithm writes or reads past the end of the container,
which is undefined behavior.

This checker uses the ValueFlow analysis to compare the number of elements an algorithm accesses with the number of
elements that are actually available through the iterator, and warns when the access is out of bounds. Three groups
of algorithms are checked:

- Algorithms that access exactly `last1 - first1` elements through the other iterator: `std::copy`, `std::move`,
  `std::swap_ranges`, `std::transform`, `std::replace_copy`, `std::replace_copy_if`, `std::reverse_copy`,
  `std::equal`, `std::mismatch`, `std::is_permutation`, `std::partial_sum`, `std::adjacent_difference` and
  `std::inner_product`.
- Algorithms that access at most `last1 - first1` elements, depending on the values in the input range:
  `std::copy_if`, `std::remove_copy`, `std::remove_copy_if` and `std::unique_copy`. Since the actual number of
  accessed elements is not known, these are only reported as inconclusive warnings (with `--inconclusive`).
- Count-based algorithms that access as many elements as the count argument says: `std::copy_n`, `std::fill_n` and
  `std::generate_n`.

The severity is `error` when the out of bounds access always happens. When the analysis depends on an earlier
condition in the code, the severity is `warning` and the message has the form "Either the condition 'v.size()==3'
is redundant or the algorithm ... ".

The checker does not warn when:

- The second range is given with both a begin and an end iterator (for example the two-range overloads of
  `std::equal`, `std::mismatch` and `std::is_permutation`), since such overloads do not access the second range out
  of bounds.
- An iterator adaptor such as `std::back_inserter` or `std::inserter` is used, since those grow the container as
  needed.
- The proof would rely on "possible" (non-known) values on both the accessed and the available side.

## How to fix

Make sure the destination range is large enough before calling the algorithm, or use an iterator adaptor such as
`std::back_inserter` that grows the container as needed.

Before:
```cpp
void f(const std::vector<int>& v0) {
    std::vector<int> v1(3);
    // If v0 has more than 3 elements, this writes past the end of v1
    std::copy(v0.begin(), v0.end(), v1.begin());
}
```

After:
```cpp
void f(const std::vector<int>& v0) {
    std::vector<int> v1(v0.size());
    std::copy(v0.begin(), v0.end(), v1.begin());
}
```

Or let the container grow:
```cpp
void f(const std::vector<int>& v0) {
    std::vector<int> v1;
    std::copy(v0.begin(), v0.end(), std::back_inserter(v1));
}
```
