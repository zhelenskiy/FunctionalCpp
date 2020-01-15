//
// Created by zhele on 18.06.2019.
//

#include "OrderedLazySeq.h"

template<class T>
template<class Func, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByItselfWith(const Func &comp) const {
    auto new_comp = [old_comp = comparer_, new_comp = comp](const T &a, const T &b) {
        return old_comp(a, b) || (!old_comp(b, a) && new_comp(a, b));
    };
    return OrderedLazySeq<T>(LazySeq<T>::orderByItselfWith(new_comp), new_comp);
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByItself() const {
    return thenByItselfWith(std::less<T>());
}

template<class T>
template<class Comparer, class Mapper, class R, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenBy(const Mapper &func, const Comparer &comp) const {
    return thenByItselfWith([comp = comp, func = func](const T &a, const T &b) { return comp(func(a), func(b)); });
}

template<class T>
template<class Mapper, class R>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenBy(const Mapper &func) const {
    return thenBy(func, std::less<R>());
}

template<class T>
template<class Comparer, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescendingByItselfWith(const Comparer &comp) const {
    return thenByItselfWith(descendingComparer(comp));
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescendingByItself() const {
    return thenByDescendingByItselfWith(std::less<T>());
}

template<class T>
template<class Comparer, class Mapper, class R, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescendingBy(const Mapper &func, const Comparer &comp) const {
    return thenBy(func, descendingComparer(comp));
}

template<class T>
template<class Mapper, class R>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescendingBy(const Mapper &func) const {
    return thenByDescendingBy(func, std::less<R>());
}

template<class T>
constexpr OrderedLazySeq<T>::OrderedLazySeq(const LazySeq<T> &seq, const SmartFunction<bool(T, T)> &comparer)
        : LazySeq<T>(seq), comparer_(comparer) {}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::rest() const {
    return skip(1);
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::skip(wide_size_t count) const {
    return OrderedLazySeq<T>(LazySeq<T>::skip(count), comparer_);
}

template<class T>
template<class Func, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::filter(const Func &pred) const {
    return OrderedLazySeq<T>(LazySeq<T>::filter(pred), comparer_);
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::take(wide_size_t count) const {
    return OrderedLazySeq<T>(LazySeq<T>::take(count), comparer_);
}

template<class T>
template<class Func, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::takeWhile(const Func &pred) const {
    return OrderedLazySeq<T>(LazySeq<T>::takeWhile(pred), comparer_);
}

template<class T>
template<class Func, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::skipWhile(const Func &pred) const {
    return OrderedLazySeq<T>(LazySeq<T>::skipWhile(pred), comparer_);
}