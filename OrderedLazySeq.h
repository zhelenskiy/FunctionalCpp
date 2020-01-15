//
// Created by zhele on 18.06.2019.
//

#ifndef FUNCTIONAL_ORDERED_LAZY_SEQ_H
#define FUNCTIONAL_ORDERED_LAZY_SEQ_H

#include "LazySeq.h"

template<class T>
class OrderedLazySeq : public LazySeq<T> {
public:
    constexpr OrderedLazySeq() = default;

    constexpr OrderedLazySeq(const OrderedLazySeq<T> &) = default;

    constexpr OrderedLazySeq(OrderedLazySeq<T> &&) noexcept = default;

    constexpr OrderedLazySeq(const LazySeq<T> &seq, const SmartFunction<bool(T, T)> &comparer);

    ~OrderedLazySeq() override = default;

    constexpr OrderedLazySeq<T> &operator=(const OrderedLazySeq<T> &) = default;

    constexpr OrderedLazySeq<T> &operator=(OrderedLazySeq<T> &&) noexcept = default;

    template<class Comparer, class = when_is_comparer<Comparer, T>>
    [[nodiscard]] constexpr OrderedLazySeq<T> thenByItselfWith(const Comparer &comp) const;

    [[nodiscard]] constexpr OrderedLazySeq<T> thenByItself() const;

    template<class Comparer, class Mapper, class R = ResType<Mapper, T>, class = when_is_comparer<Comparer, R>>
    constexpr OrderedLazySeq<T> thenBy(const Mapper &func, const Comparer &comp) const;

    template<class Mapper, class R = ResType<Mapper, T>>
    constexpr OrderedLazySeq<T> thenBy(const Mapper &func) const;

    template<class Comparer, class = when_is_comparer<Comparer, T>>
    [[nodiscard]] constexpr OrderedLazySeq<T> thenByDescendingByItselfWith(const Comparer &comp) const;

    [[nodiscard]] constexpr OrderedLazySeq<T> thenByDescendingByItself() const;

    template<class Comparer, class Mapper, class R = ResType<Mapper, T>, class = when_is_comparer<Comparer, R>>
    constexpr OrderedLazySeq<T> thenByDescendingBy(const Mapper &func, const Comparer &comp) const;

    template<class Mapper, class R = ResType<Mapper, T>>
    constexpr OrderedLazySeq<T> thenByDescendingBy(const Mapper &func) const;

    constexpr OrderedLazySeq<T> take(wide_size_t count) const;

    template<class Func, class = when_is_predicate<Func, T>>
    constexpr OrderedLazySeq<T> takeWhile(const Func &pred) const;

    constexpr OrderedLazySeq<T> rest() const;
//  constexpr OrderedLazySeq<T> butLast() const;

    constexpr OrderedLazySeq<T> skip(wide_size_t count) const;

    template<class Func, class = when_is_predicate<Func, T>>
    constexpr OrderedLazySeq<T> skipWhile(const Func &pred) const;

    template<class Func, class = when_is_predicate<Func, T>>
    constexpr OrderedLazySeq<T> filter(const Func &pred) const;

private:
    SmartFunction<bool(T, T)> comparer_;
};

#include "OrderedLazySeq.hpp"

#endif //FUNCTIONAL_ORDERED_LAZY_SEQ_H
