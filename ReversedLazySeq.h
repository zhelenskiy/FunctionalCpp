//
// Created by zhele on 31.07.2019.
//

#ifndef FUNCTIONAL_REVERSED_LAZY_SEQ_H
#define FUNCTIONAL_REVERSED_LAZY_SEQ_H

#include "LazySeq.h"

template<class T>
class ReversedLazySeq : public LazySeq<T> {
 public:
  constexpr explicit ReversedLazySeq(const LazySeq<T> &nonReversedSeq);
  constexpr ReversedLazySeq(const LazySeq<T> &reversedSeq, const LazySeq<T> &nonReversedSeq);
  constexpr ReversedLazySeq(const ReversedLazySeq<T> &) = default;
  constexpr ReversedLazySeq(ReversedLazySeq<T> &&) noexcept = default;
  constexpr ReversedLazySeq<T> &operator=(ReversedLazySeq<T> &&) noexcept = default;
  constexpr ReversedLazySeq<T> &operator=(const ReversedLazySeq<T> &) = default;

  ~ReversedLazySeq() override = default;

  constexpr ReversedLazySeq<T> reverse() const override;

  template<class Lambda, class = when_is_predicate<Lambda, T>>
  constexpr ReversedLazySeq<T> filter(const Lambda &pred) const;
  constexpr ReversedLazySeq<T> rest() const;
  constexpr ReversedLazySeq<T> butLast() const;
  template<class Lambda, class R = ResType<Lambda, T>>
  constexpr ReversedLazySeq<R> map(const Lambda &func) const;
  constexpr ReversedLazySeq<T> concat(const LazySeq<T>& other) const;
  template<class R>
  constexpr ReversedLazySeq<std::pair<T, R>> match(const LazySeq<R>& other) const;

 private:

  LazySeq<T> nonReversedSeq_;
};

#include "ReversedLazySeq.hpp"
#endif //FUNCTIONAL_REVERSED_LAZY_SEQ_H
