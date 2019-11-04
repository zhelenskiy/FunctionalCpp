# FunctionalCpp
Smart lazy sequence implementation in C++17.
The project implements lazy sequences as they are in C\#, Clojure, Kotlin and other languages.

The lazy sequences have a large number of methods apart from filter, map that are declared in LazySeq.h.

Also there are a lot of optimizations such as:
* ".reverse().reverse() does nothing":

That means that
  `std::cout << naturalNumbers().reverse().first();`
wouldn't work endlessly.*


* Lazy ordering:

If I take items 100 000th to 1 000 000th from 10 000 000, only 900 000 items become ordered.


Also there are some functions that can help you to develop functional style programs with C++17.
