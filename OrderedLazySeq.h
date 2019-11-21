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

  template<class Lambda, class = when_is_comparer<Lambda, T>>
  constexpr OrderedLazySeq<T> thenBy(const Lambda &comp) const;
  constexpr OrderedLazySeq<T> thenBy() const;
  template<class Lambda, class = when_is_comparer<Lambda, T>>
  constexpr OrderedLazySeq<T> thenByDescending(const Lambda &comp) const;
  constexpr OrderedLazySeq<T> thenByDescending() const;
  template<class R, class Lambda, class = when_is_comparer<Lambda, R>>
  constexpr OrderedLazySeq<T> thenBy(const std::function<R(T)> &func,
                                     const Lambda &comp) const;
  template<class R>
  constexpr OrderedLazySeq<T> thenBy(const std::function<R(T)> &func) const;
  template<class R, class Lambda, class = when_is_comparer<Lambda, R>>
  constexpr OrderedLazySeq<T> thenByDescending(const std::function<R(T)> &func,
                                               const Lambda &comp) const;
  template<class R>
  constexpr OrderedLazySeq<T> thenByDescending(const std::function<R(T)> &func) const;

  template<class Lambda, class R = ResType<Lambda, T>>
  constexpr OrderedLazySeq<R> map(const Lambda &func) const;

  constexpr OrderedLazySeq<T> take(wide_size_t count) const;
  template<class Lambda, class = when_is_predicate<Lambda, T>>
  constexpr OrderedLazySeq<T> takeWhile(const Lambda &pred) const;

  constexpr OrderedLazySeq<T> rest() const;
//  constexpr OrderedLazySeq<T> butLast() const;

  constexpr OrderedLazySeq<T> skip(wide_size_t count) const;
  template<class Lambda, class = when_is_predicate<Lambda, T>>
  constexpr OrderedLazySeq<T> skipWhile(const Lambda &pred) const;

  template<class Lambda, class = when_is_predicate<Lambda, T>>
  constexpr OrderedLazySeq<T> filter(const Lambda &pred) const;

  [[nodiscard]] constexpr bool hasSpecialPartialSkipHelper() const;
  constexpr std::pair<wide_size_t, equivClasses<T>> applyPartialSkipHelper(wide_size_t count) const;

 private:
  static const wide_size_t BUCKET_SIZE_FOR_STD_SORT_CALL = 256;

  equivClasses<T> classes_;
  partial_skip_helper_t partialSkipHelper_;

  template<class Lambda, class = when_is_comparer<Lambda, T>>
  static auto partition(const equivClass<T> &items, const Lambda &comp);

//  template<bool stable>
  template<class Lambda, class = when_is_comparer<Lambda, T>>
  static equivClasses<T> separateMore(const equivClasses<T> &seq, const Lambda &comp);

//  template<bool stable>
  template<class Lambda, class = when_is_comparer<Lambda, T>>
  static std::pair<wide_size_t, equivClasses<T>> smartSkip(const equivClass<T> &items,
                                                           wide_size_t count,
                                                           const Lambda &comp);
//  template<bool stable>
  static std::pair<wide_size_t, node_ptr<equivClass<T>>> simpleSkip(wide_size_t count, const equivClasses<T> &items);

//  template<bool stable>
  template<class Lambda, class = when_is_comparer<Lambda, T>>
  static equivClass<T> stdSort(const equivClass<T> &items, const Lambda &comp);
  template<class Lambda, class R = ResType<Lambda, T>>
  static auto vectorMap(const Lambda &func);
  template<class Lambda, class = when_is_predicate<Lambda, T>>
  static auto vectorFilter(const Lambda &pred);
  constexpr static equivClasses<T> getTakenClasses(const equivClasses<T> &classes, wide_size_t count);
  template<class Lambda, class = when_is_predicate<Lambda, T>>
  constexpr static equivClasses<T> getTakenWhileClasses(const equivClasses<T> &classes, const Lambda &pred);
  template<class Lambda, class = when_is_predicate<Lambda, T>>
  constexpr static equivClasses<T> getSkippedWhileClasses(const equivClasses<T> &classes, const Lambda &pred);
};
#include "OrderedLazySeq.hpp"

#endif //FUNCTIONAL_ORDERED_LAZY_SEQ_H
