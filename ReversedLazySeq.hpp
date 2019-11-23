//
// Created by zhele on 31.07.2019.
//

#include "ReversedLazySeq.h"

template<class T>
constexpr ReversedLazySeq<T> ReversedLazySeq<T>::reverse() const {
  return ReversedLazySeq(nonReversedSeq_, *this);
}

template<class T>
constexpr ReversedLazySeq<T>::ReversedLazySeq(const LazySeq<T> &reversedSeq, const LazySeq<T> &nonReversedSeq)
    : LazySeq<T>(reversedSeq), nonReversedSeq_(nonReversedSeq) {}

template<class T>
constexpr ReversedLazySeq<T>::ReversedLazySeq(const LazySeq<T> &nonReversedSeq)
    : ReversedLazySeq<T>(LazySeq<T>([nonReversedSeq] {
  return nonReversedSeq.template reduce<LazySeq<T>>(LazySeq<T>(), [](const auto &seq, const T &item) {
    return seq.emplaceFront(item);
  });
}), nonReversedSeq) {}

template<class T>
template<class Func, class>
constexpr ReversedLazySeq<T> ReversedLazySeq<T>::filter(const Func &pred) const {
  return ReversedLazySeq<T>(LazySeq<T>::filter(pred), nonReversedSeq_.filter(pred));
}

template<class T>
constexpr ReversedLazySeq<T> ReversedLazySeq<T>::rest() const {
  return ReversedLazySeq<T>(LazySeq<T>::skip(), nonReversedSeq_.butLast());
}

template<class T>
constexpr ReversedLazySeq<T> ReversedLazySeq<T>::butLast() const {
  return ReversedLazySeq<T>(LazySeq<T>::butLast(), nonReversedSeq_.skip());
}

template<class T>
template<class Func, class R>
constexpr ReversedLazySeq<R> ReversedLazySeq<T>::map(const Func &func) const {
  return ReversedLazySeq<R>(LazySeq<T>::map(func), nonReversedSeq_.map(func));
}

template<class T>
constexpr ReversedLazySeq<T> ReversedLazySeq<T>::concat(const LazySeq<T> &other) const {
  return ReversedLazySeq<T>(LazySeq<T>::concat(other), static_cast<LazySeq<T>>(other.reverse()) + nonReversedSeq_);
}

template<class T>
template<class R>
constexpr ReversedLazySeq<std::pair<T, R>> ReversedLazySeq<T>::match(const LazySeq<R> &other) const {
  return ReversedLazySeq<std::pair<T, R>>(LazySeq<T>::match(other), nonReversedSeq_.match(other.reverse()));
}
