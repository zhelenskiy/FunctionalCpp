//
// Created by zhele on 18.06.2019.
//

#ifndef FUNCTIONAL_ORDERED_LAZY_SEQ_H
#define FUNCTIONAL_ORDERED_LAZY_SEQ_H

#include "LazySeq.h"

template<class T>
class OrderedLazySeq : public LazySeq<T> {
 public:
  using partial_skip_helper_t = SmartFunction<std::pair<wide_size_t, equivClasses<T>>(wide_size_t)>;

  constexpr OrderedLazySeq() = default;
  constexpr OrderedLazySeq(const OrderedLazySeq<T> &) = default;
  constexpr OrderedLazySeq(OrderedLazySeq<T> &&) noexcept = default;
  constexpr OrderedLazySeq(const equivClasses<T> &classes, const partial_skip_helper_t &partialSkipHelper = nullptr);

  ~OrderedLazySeq() override = default;

  constexpr OrderedLazySeq<T> &operator=(const OrderedLazySeq<T> &) = default;
  constexpr OrderedLazySeq<T> &operator=(OrderedLazySeq<T> &&) noexcept = default;

  template<class Comparer, class = when_is_comparer<Comparer, T>>
  constexpr OrderedLazySeq<T> thenBy(const Comparer &comp) const;
  constexpr OrderedLazySeq<T> thenBy() const;
  template<class Comparer, class = when_is_comparer<Comparer, T>>
  constexpr OrderedLazySeq<T> thenByDescending(const Comparer &comp) const;
  constexpr OrderedLazySeq<T> thenByDescending() const;
  template<class ValueFunc, class Comparer, class R = ResType<ValueFunc, T>, class = when_is_comparer<Comparer, R>>
  constexpr OrderedLazySeq<T> thenBy(const ValueFunc &func,
                                     const Comparer &comp) const;
  template<class ValueFunc, class R = ResType<ValueFunc, T>, class = void>
  constexpr OrderedLazySeq<T> thenBy(const ValueFunc &func) const;
  template<class ValueFunc, class Comparer, class R = ResType<ValueFunc, T>, class = when_is_comparer<Comparer, R>>
  constexpr OrderedLazySeq<T> thenByDescending(const ValueFunc &func,
                                               const Comparer &comp) const;
  template<class ValueFunc, class = ResType<ValueFunc, T>, class = void>
  constexpr OrderedLazySeq<T> thenByDescending(const ValueFunc &func) const;

  template<class Func, class R = ResType<Func, T>>
  constexpr OrderedLazySeq<R> map(const Func &func) const;

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

  [[nodiscard]] constexpr bool hasSpecialPartialSkipHelper() const;
  constexpr std::pair<wide_size_t, equivClasses<T>> applyPartialSkipHelper(wide_size_t count) const;

 private:
  static const wide_size_t BUCKET_SIZE_FOR_STD_SORT_CALL = 256;

  equivClasses<T> classes_;
  partial_skip_helper_t partialSkipHelper_;

  template<class Func, class = when_is_comparer<Func, T>>
  static auto partition(const equivClass<T> &items, const Func &comp);

//  template<bool stable>
  template<class Func, class = when_is_comparer<Func, T>>
  static equivClasses<T> separateMore(const equivClasses<T> &seq, const Func &comp);

//  template<bool stable>
  template<class Func, class = when_is_comparer<Func, T>>
  static std::pair<wide_size_t, equivClasses<T>> smartSkip(const equivClass<T> &items,
                                                           wide_size_t count,
                                                           const Func &comp);
//  template<bool stable>
  static std::pair<wide_size_t, node_ptr<equivClass<T>>> simpleSkip(wide_size_t count, const equivClasses<T> &items);

//  template<bool stable>
  template<class Func, class = when_is_comparer<Func, T>>
  static equivClass<T> stdSort(const equivClass<T> &items, const Func &comp);
  template<class Func, class R = ResType<Func, T>>
  static auto vectorMap(const Func &func);
  template<class Func, class = when_is_predicate<Func, T>>
  static auto vectorFilter(const Func &pred);
  constexpr static equivClasses<T> getTakenClasses(const equivClasses<T> &classes, wide_size_t count);
  template<class Func, class = when_is_predicate<Func, T>>
  constexpr static equivClasses<T> getTakenWhileClasses(const equivClasses<T> &classes, const Func &pred);
  template<class Func, class = when_is_predicate<Func, T>>
  constexpr static equivClasses<T> getSkippedWhileClasses(const equivClasses<T> &classes, const Func &pred);
};
#include "OrderedLazySeq.hpp"

#endif //FUNCTIONAL_ORDERED_LAZY_SEQ_H
