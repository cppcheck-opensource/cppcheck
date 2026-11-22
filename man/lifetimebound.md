---
title: The lifetimebound attribute
subtitle: Version 2.23 dev
author: Cppcheck team
lang: en
documentclass: report
---

# Introduction

This document describes how Cppcheck uses the `lifetimebound` attribute, what the attribute leaves unsaid, and how Cppcheck fills in the missing information.

# The attribute

The attribute says that the result of a function refers to an argument, so the result should not be used after the argument is gone. Cppcheck needs this when it cannot see the body of the function:

    const int* get(const int& x [[clang::lifetimebound]]);

    const int* f() {
        int x = 0;
        return get(x);
    }

Without the attribute Cppcheck does not know that the returned pointer has anything to do with `x`. With it we get:

    test.cpp:5:15: error: Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]

## Spelling

These spellings are recognized:

    [[clang::lifetimebound]]
    [[msvc::lifetimebound]]
    __attribute__((lifetimebound))
    __attribute__((__lifetimebound__))

The attribute can be written on the declaration, on the definition, or on both.

## Where it can be placed

On a parameter, either after the parameter or before its type:

    const int& h(const int& x [[clang::lifetimebound]]);
    const int& h([[clang::lifetimebound]] const int& x);

After the declarator of a member function. It then applies to the object that the function is called on. On a parameter of a constructor, the constructed object refers to the argument. This example uses both:

    #include <vector>

    struct A {
        const int& get() const [[clang::lifetimebound]];
        int x;
    };

    const int& f1() {
        A a{};
        return a.get();
    }

    struct View {
        View(const std::vector<int>& v [[clang::lifetimebound]]);
        const int* p;
    };

    View f2() {
        std::vector<int> v = {1};
        return View(v);
    }

In `f1` the returned reference is bound to `a`, and in `f2` the returned `View` is bound to `v`:

    test.cpp:10:17: error: Reference to local variable returned. [returnReference]
    test.cpp:20:16: error: Returning object that points to local variable 'v' that will be invalid when returning. [returnDanglingLifetime]

## Reference and value parameters

When the parameter is a reference, the result refers to the argument itself. When the parameter is passed by value, such as a pointer, an iterator or a `std::string_view`, the result refers to whatever the argument refers to:

    const int* next(const int* p [[clang::lifetimebound]]);

    const int* f() {
        int x = 0;
        return next(&x);
    }

Here the result is bound to `x` and not to the temporary pointer `&x`:

    test.cpp:5:16: error: Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]

## Checks that use it

The result of an annotated function is treated like any other pointer or reference with a known lifetime. So it is used by the checks for returning or storing something that refers to a local variable(`returnDanglingLifetime`, `returnReference`, `danglingLifetime`), for using it after the variable is out of scope(`invalidLifetime`), for references to temporaries(`danglingTempReference`), and for using it after a container has been modified(`invalidContainer`, `invalidContainerReference`).

# The ambiguity

The attribute says that the result refers to the argument. It does not say which part of the argument. Let's look at two functions that return a struct. One struct stores a pointer to the vector, and the other stores a pointer to the elements of the vector:

    #include <cstddef>
    #include <vector>

    struct VectorRef {
        const std::vector<int>* vec;
    };

    struct DataRef {
        const int* data;
    };

    VectorRef make_vector_ref(const std::vector<int>& v [[clang::lifetimebound]]);
    DataRef make_data_ref(const std::vector<int>& v [[clang::lifetimebound]]);

    std::size_t f1() {
        std::vector<int> v = {1};
        VectorRef r = make_vector_ref(v);
        v.push_back(2);
        return r.vec->size();
    }

    int f2() {
        std::vector<int> v = {1};
        DataRef r = make_data_ref(v);
        v.push_back(2);
        return *r.data;
    }

Both functions have the same annotation, and it says the same thing for both: the result is bound to `v`. If either result outlives `v` then it is dangling, and Cppcheck reports that for both.

However, `f1` and `f2` do exactly the same thing with the result, and only one of them is a bug:

- `f1` is fine. `r.vec` points to the vector, and the vector is still at the same address after `push_back`.
- `f2` is a bug. `r.data` points to the elements, and `push_back` can move the elements to a new allocation.

So the lifetime association is the same but the outcome is different. The attribute has no way to say which of the two a function returns, and this is why it is ambiguous. If Cppcheck assumed that the result always points to the contents, it would warn about `f1`. If it assumed that the result always points to the object itself, it would miss `f2`.

# How Cppcheck resolves it

By default Cppcheck assumes that the result refers to the object itself and not to its contents. This means the result is checked for dangling but not for invalidation. Cppcheck would rather miss a bug than report a false positive.

For the example above this means that nothing is reported. `VectorRef` and `DataRef` do not show what they refer to, so `f2` is missed.

The result is treated as referring to the contents only when there is evidence for it. The evidence comes from the types, from the argument, or from the function body.

## Types

When the function returns a pointer instead of a struct, the return type shows what it points to:

    #include <cstddef>
    #include <vector>

    const std::vector<int>* addr(const std::vector<int>& v [[clang::lifetimebound]]);
    const int* first(const std::vector<int>& v [[clang::lifetimebound]]);

    std::size_t f1() {
        std::vector<int> v = {1};
        const std::vector<int>* p = addr(v);
        v.push_back(2);
        return p->size();
    }

    int f2() {
        std::vector<int> v = {1};
        const int* p = first(v);
        v.push_back(2);
        return *p;
    }

This is the same pair as before, but now the types are different. `addr` returns a pointer to the type of the parameter, so the result is the vector itself. `first` returns a pointer to the element type of the vector, so the result points to the contents. Only `f2` is reported:

    test.cpp:18:13: error: Using pointer to local variable 'v' that may be invalid. [invalidContainer]

The same works for iterators and references:

    #include <vector>

    std::vector<int>::const_iterator find_first(const std::vector<int>& v [[clang::lifetimebound]]);
    const int& front(const std::vector<int>& v [[clang::lifetimebound]]);
    const std::vector<int>& same(const std::vector<int>& v [[clang::lifetimebound]]);

    int f1() {
        std::vector<int> v = {1};
        auto it = find_first(v);
        v.push_back(2);
        return *it;
    }

    int f2() {
        std::vector<int> v = {1};
        const int& r = front(v);
        v.push_back(2);
        return r;
    }

    int f3() {
        std::vector<int> v = {1};
        const std::vector<int>& r = same(v);
        v.push_back(2);
        return r[0];
    }

`find_first` returns an iterator of the container and `front` returns a reference to the element type, so `f1` and `f2` are reported. `same` returns a reference to the vector itself, so `f3` is not:

    test.cpp:11:13: error: Using iterator to local container 'v' that may be invalid. [invalidContainer]
    test.cpp:18:12: error: Reference to v that may be invalid. [invalidContainerReference]

A view such as `std::string_view` or `std::span` refers to the contents when it has the same element type as the container:

    #include <cstddef>
    #include <string>
    #include <string_view>

    std::string_view trim(const std::string& s [[clang::lifetimebound]]);

    std::size_t f() {
        std::string s = " a ";
        std::string_view sv = trim(s);
        s += "b";
        return sv.size();
    }

Appending to `s` can reallocate the characters that `sv` refers to:

    test.cpp:11:12: error: Using object that points to local variable 's' that may be invalid. [invalidContainer]

These are the rules that Cppcheck uses when it compares the return type with the type of the bound parameter:

| Return type | Parameter | The result refers to |
|---|---|---|
| iterator of the container | the same container type | the contents, as an iterator |
| pointer or reference to the element type | container | the contents |
| view with the same element type | container | the contents |
| pointer or reference to the same type | any type | the object itself |
| anything else | any type | the object itself, since nothing is known |

The last row is the default. Here the return type does not show that the result points to an element:

    #include <vector>

    const void* raw(const std::vector<int>& v [[clang::lifetimebound]]);
    void write(const void* p);

    void f() {
        std::vector<int> v = {1};
        const void* p = raw(v);
        v.push_back(2);
        write(p);
    }

Nothing is reported for this. The structs in the previous section are in the last row as well.

## The argument

When the parameter is passed by value, the result refers to the same thing as the argument, in the same way. So what is known about the argument is carried over to the result:

    #include <cstddef>
    #include <vector>

    const int* skip(const int* p [[clang::lifetimebound]]);
    const std::vector<int>* pass(const std::vector<int>* p [[clang::lifetimebound]]);

    int f1() {
        std::vector<int> v = {1, 2};
        const int* p = skip(v.data());
        v.push_back(3);
        return *p;
    }

    std::size_t f2() {
        std::vector<int> v = {1, 2};
        const std::vector<int>* p = pass(&v);
        v.push_back(3);
        return p->size();
    }

`skip(v.data())` points to the contents of `v` because `v.data()` does. `pass(&v)` points to `v` itself because `&v` does. So only `f1` is reported:

    test.cpp:11:13: error: Using pointer to local variable 'v' that may be invalid. [invalidContainer]

A view parameter always refers to the contents of the container that it was created from:

    #include <cstddef>
    #include <string>
    #include <string_view>

    std::string_view trim(std::string_view sv [[clang::lifetimebound]]);

    std::size_t f() {
        std::string s = " a ";
        std::string_view sv = trim(s);
        s += "b";
        return sv.size();
    }

The result of `trim` refers to the characters of `s`:

    test.cpp:11:12: error: Using object that points to local variable 's' that may be invalid. [invalidContainer]

The same applies when a reference parameter is bound to an element of a container:

    #include <vector>

    const int& clamp(const int& x [[clang::lifetimebound]]);

    int f() {
        std::vector<int> v = {1};
        const int& r = clamp(v[0]);
        v.push_back(2);
        return r;
    }

The result is the same `int` as the argument, and the argument is an element of `v`:

    test.cpp:9:12: error: Reference to v that may be invalid. [invalidContainerReference]

## The function body

When the body of the function is available, Cppcheck also looks at what it returns. The body shows which part of the argument is returned even when the types do not. Let's go back to the two structs and add the function bodies:

    #include <cstddef>
    #include <vector>

    struct VectorRef {
        const std::vector<int>* vec;
    };

    struct DataRef {
        const int* data;
    };

    VectorRef make_vector_ref(const std::vector<int>& v [[clang::lifetimebound]]) {
        return VectorRef{&v};
    }

    DataRef make_data_ref(const std::vector<int>& v [[clang::lifetimebound]]) {
        return DataRef{v.data()};
    }

    std::size_t f1() {
        std::vector<int> v = {1};
        VectorRef r = make_vector_ref(v);
        v.push_back(2);
        return r.vec->size();
    }

    int f2() {
        std::vector<int> v = {1};
        DataRef r = make_data_ref(v);
        v.push_back(2);
        return *r.data;
    }

Now Cppcheck can see that `VectorRef` stores the address of the vector and that `DataRef` stores a pointer to the elements. So `f2` is reported and `f1` is not:

    test.cpp:31:13: error: Using object that points to local variable 'v' that may be invalid. [invalidContainer]

The attribute and the body are combined like this:

- When both say that the result refers to the same variable, the body decides whether it refers to the object itself or to its contents.
- The result refers to everything that either of them finds.
- A function with several return statements normally gives inconclusive results. When the attribute says the same thing as one of the return statements, the result is not inconclusive.

In this example the body cannot be followed for `p`, since `lookup` is not defined, but the attribute covers it:

    const int* lookup(const int* p);

    const int* pick(const int* p [[clang::lifetimebound]], const int* q, bool b) {
        if (b)
            return lookup(p);
        return q;
    }

    const int* f1(const int* q) {
        int x = 0;
        return pick(&x, q, true);
    }

    const int* f2(const int* p) {
        int y = 0;
        return pick(p, &y, false);
    }

The result of `pick` is bound to `p` because of the attribute, and to `q` because of the body. So `f1` is reported:

    test.cpp:11:16: error: Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]

`f2` is only reported with `--inconclusive`, because `q` is returned from one of two return statements and it has no attribute:

    test.cpp:16:16: error, inconclusive: Returning pointer to local variable 'y' that will be invalid when returning. [returnDanglingLifetime]

When the parameter has the attribute, the result is not inconclusive even though there are two return statements:

    const int* find(const int* p [[clang::lifetimebound]], bool b) {
        if (b)
            return nullptr;
        return p;
    }

    const int* f() {
        int x = 0;
        return find(&x, false);
    }

This is reported without `--inconclusive`:

    test.cpp:9:16: error: Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]

## Several annotated parameters

When more than one parameter has the attribute, Cppcheck does not know which one the result refers to:

    const int& max(const int& x [[clang::lifetimebound]], const int& y [[clang::lifetimebound]]);

    const int& f(const int& x) {
        int y = 0;
        return max(x, y);
    }

This is only reported with `--inconclusive`:

    test.cpp:5:15: error, inconclusive: Reference to local variable returned. [returnReference]

# Limitations

- The type comparison only works when the types are written the same way. `std::basic_string_view<char>` from a `std::string` is not recognized as a view of the contents.
- A result that points to the contents but does not show it in its type, such as `const void*` or a struct, is only checked for invalidation when the function body is available.
- The attribute cannot say that the result refers to the contents. When the types do not show it and there is no body, invalidation bugs are missed.
