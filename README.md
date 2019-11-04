# FunctionalCpp
**Smart lazy sequence implementation in C++17.**
The project implements lazy sequences as they are in C\#, Clojure, Kotlin and other languages.

The lazy sequences have a large number of methods apart from filter, map that are declared in LazySeq.h.

---

**Also there are a lot of optimizations such as:**
* ".reverse().reverse() does nothing":

That means that
  `std::cout << naturalNumbers().reverse().reverse().first();`
wouldn't work endlessly.*

---

* Lazy ordering:

If I take items 100 000th to 1 000 000th from 10 000 000, only 900 000 items become ordered.

`std::cout << range(1, 1000000).skip(10000).take(10000);` sorts only 10000 items.

---

`makeLazy(std::move(someStdVector)).map<long>(someFunc).itemAt(400)` is O(1).

`makeLazy(std::move(someStdVector)).map<long>(someFunc).count()` is O(1) too.

---

In next updates the lazy sequences are going even to resolve if it is faster to use smart skip helper or not.

```cpp
// SomeLazySeq is a seq that has SmartSkipper (O(1)) only for first item,
// all other are not defined and automaticaly generated with a previous skip helper.

for (auto seq = someLazySeq; !seq.isEmpty(); seq = seq.skip()) {
    cout << seq.itemAt(100) << ' ';
}
// After some iterations lazy sequences stop using smart skip helper
// because it becomes cheaper to make 100 iterations than to find first skipHelper and use it.
```

---

Also there are some functions that can help you to develop functional style programs with C++17.
