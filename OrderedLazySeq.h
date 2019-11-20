//
// Created by zhele on 18.06.2019.
//

#ifndef FUNCTIONAL_ORDERED_LAZY_SEQ_H
#define FUNCTIONAL_ORDERED_LAZY_SEQ_H

#include "LazySeq.h"

template<class T>
class OrderedLazySeq : public LazySeq<T> {
 public:
  using partial_skip_helper_t = std::function<std::pair<wide_size_t, equivClasses<T>>(wide_size_t)>;

  constexpr OrderedLazySeq() = default;
  constexpr OrderedLazySeq(const OrderedLazySeq<T> &) = default;
  constexpr OrderedLazySeq(OrderedLazySeq<T> &&) noexcept = default;
  constexpr OrderedLazySeq(const equivClasses<T> &classes, const partial_skip_helper_t &partialSkipHelper = nullptr);

  ~OrderedLazySeq() override = default;

  constexpr OrderedLazySeq<T> &operator=(const OrderedLazySeq<T> &) = default;
  constexpr OrderedLazySeq<T> &operator=(OrderedLazySeq<T> &&) noexcept = default;

  constexpr OrderedLazySeq<T> thenBy(const comparer<T> &comp = std::less<T>()) const;
  constexpr OrderedLazySeq<T> thenByDescending(const comparer<T> &comp = std::less<T>()) const;
  template<class R>
  constexpr OrderedLazySeq<T> thenBy(const std::function<R(T)> &func,
                                     const comparer<R> &comp = std::less<R>()) const;
  template<class R>
  constexpr OrderedLazySeq<T> thenByDescending(const std::function<R(T)> &func,
                                               const comparer<R> &comp = std::less<R>()) const;
  template<class R>
  constexpr OrderedLazySeq<R> map(const std::function<R(T)> &func) const;

  constexpr OrderedLazySeq<T> take(wide_size_t count) const;
  constexpr OrderedLazySeq<T> takeWhile(const predicate<T> &pred) const;

  constexpr OrderedLazySeq<T> rest() const;
//  constexpr OrderedLazySeq<T> butLast() const;

  constexpr OrderedLazySeq<T> skip(wide_size_t count) const;
  constexpr OrderedLazySeq<T> skipWhile(const predicate<T> &pred) const;

  constexpr OrderedLazySeq<T> filter(const predicate<T> &pred) const;

  [[nodiscard]] constexpr bool hasSpecialPartialSkipHelper() const;
  constexpr std::pair<wide_size_t, equivClasses<T>> applyPartialSkipHelper(wide_size_t count) const;

 private:
  static const wide_size_t BUCKET_SIZE_FOR_STD_SORT_CALL = 256;

  equivClasses<T> classes_;
  partial_skip_helper_t partialSkipHelper_;

  static auto partition(const equivClass<T> &items, const comparer<T> &comp);

//  template<bool stable>
  static equivClasses<T> separateMore(const equivClasses<T> &seq, const comparer<T> &comp);

//  template<bool stable>
  static std::pair<wide_size_t, equivClasses<T>> smartSkip(const equivClass<T> &items,
                                                           wide_size_t count,
                                                           const comparer<T> &comp);
//  template<bool stable>
  static std::pair<wide_size_t, node_ptr<equivClass<T>>> simpleSkip(wide_size_t count, const equivClasses<T> &items);

//  template<bool stable>
  static equivClass<T> stdSort(const equivClass<T> &items, const comparer<T> &comp);
  template<class R>
  static std::function<equivClass<R>(equivClass<T>)> vectorMap(const std::function<R(T)> &func);
  static std::function<equivClass<T>(equivClass<T>)> vectorFilter(const predicate<T> &pred);
  constexpr static equivClasses<T> getTakenClasses(const equivClasses<T> &classes, wide_size_t count);
  constexpr static equivClasses<T> getTakenWhileClasses(const equivClasses<T> &classes, const predicate<T> &pred);
  constexpr static equivClasses<T> getSkippedWhileClasses(const equivClasses<T> &classes, const predicate<T> &pred);
};
#include "OrderedLazySeq.hpp"

#endif //FUNCTIONAL_ORDERED_LAZY_SEQ_H
