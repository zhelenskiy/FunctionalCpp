# FunctionalCpp

### TL;DR

**Smart lazy sequence implementation in C++17.**
The project implements lazy sequences as they are in C\#, Clojure, Kotlin and other languages.

The lazy sequences have a large number of methods apart from filter, map that are declared in LazySeq.h.

---

**Also there are a lot of optimizations such as:**
* `.reverse().reverse()` does nothing

That means that
  `std::cout << naturalNumbers().reverse().reverse().first();`
wouldn't work endlessly.

---

* Lazy ordering:

If I take items 100 000th to 1 000 000th from 10 000 000, only 900 000 items become ordered.

`std::cout << range(1, 1000000).skip(10000).take(10000);` sorts only 10000 items.

---

`makeLazy(std::move(someStdVector))map(someFunc).itemAt(400)` is O(1).

`makeLazy(std::move(someStdVector))map(someFunc).count()` is O(1) too.

---

### Library structure

* It consists of an evaluator and skip helper.
    * Evaluator is a function that generates current item and the next lazy sequence. 
    * Skip helper is a function that can speed up skip operation when it is possible.
       * For example, when you use lazy sequence generated with a vector or a range, you know how to skip n items in O(1) complexity.
       * Lots of operations such as `map`, `match`, `repeat`, `concat` can keep skip helper if a base sequence had it.
       * It can help making ordering lazy.
       * It also can be implicitly used in different functions such as `itemAt`, `count` to speed up them.
* There is a lazy iterator that contains evaluated value and is used as an iterator to make using range-based for-s with lazy sequences possible.
* There are two ancestors of lazy sequences:
    * Reversed lazy sequences: stores not reversed sequence too to be able to do nothing if the number of `.reverse(...)` is even. (`naturalNumbers().reverse().reverse().first()` doesn't work infinitely)
    * Ordered lazy sequences: it has its own special `.thenBy(...)` and `thenByDescending(...)` functions apart from ones from the base class.
        * Is lazy operation
        * Uses std::sort when the data size is small.
* There are additional functions that can simplify using lazy sequences in functional style code.
* Some macros perform anonymous functions (almost) as they are in Kotlin. But they are not included in the main library because that would mean that I preserve some global names for my library macros.

### Listing

*A big change is going to happen to list of methods soon, so it is not worth listing now.*

### Known Issues

| Name | Problem | Solution |
| ---- | ------- | -------- |
| Slowness | The speed is small due to excessive heap memory usage | it is going to be reduced in upcoming updates. Also ordering is going to become in place. |
| Order | OrderedLazySeq does not override methods of LazySeq because it has a different return type. *For example, LazySeq::skip returns LazySeq while OrderedLazySeq::skip returns OrderedLazySeq.* | Future replacements: Pipe operator instead of member access operator; implicit cast instead of inheritance. |
| Skip slowness | Sometimes it is faster to eval skip straight than using smart skip helper if `n` is small and skipHelper complexity is not O(1). The problem is especially relevant when skipHelper is deeply nested. *For example, after using* `skip(1)` `n` *times*, current smart skip helper is `Ω(n)`. | Use `float skipHelperComplexityFunction(wide_size_t n)` that returns approximate number of operations of applying `smartSkipHelper(n)`. Complexity of this function is supposed to be O(1). `float` is used to avoid overflows because of no necessity of exact result.
| Overflow | In `.count()` I use the fact that `count = WIDE_SIZE_T_MAX - skipHelper(WIDE_SIZE_T_MAX).stillNotSkipped` which can lead to overflows. | Use a special helper for this purpose. Complexity is supposed to be O(1).

### In the upcoming updates

In the next updates the lazy sequences are going even to resolve if it is faster to use smart skip helper or not. But methods signatures are self-documented, so you can have a look at the methods in *.h-s.

```cpp
// SomeLazySeq is a seq that has SmartSkipper (O(1)) only for first item,
// all other are not defined and automatically generated with a previous skip helper.

for (auto seq = someLazySeq; !seq.isEmpty(); seq = seq.skip()) {
    cout << seq.itemAt(100) << ' ';
}
// After some iterations lazy sequences stop using smart skip helper
// because it becomes cheaper to make 100 iterations than to find first skipHelper and use it.
```

---

Also, some functions can help you to develop functional style programs with C++17.