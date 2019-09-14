//
// Created by zhele on 06.06.2019.
//

#ifndef FUNCTIONAL_LAZY_SEQ_H
#define FUNCTIONAL_LAZY_SEQ_H

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <stack>
#include <sstream>
#include <iostream>
#include <list>
#include <chrono>
#include <random>

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

template<class T>
class LazySeq;

template<class T>
class OrderedLazySeq;

template<class T>
class ReversedLazySeq;

template<class T>
class LazyIterator;

template<class T>
using node = std::pair<T, LazySeq<T>>;
template<class T>
using node_ptr = std::shared_ptr<node<T>>;
template<class T>
using fabric = std::function<node_ptr<T>()>;
template<class T>
using fabric_ptr = std::shared_ptr<fabric<T>>;
template<class T>
using predicate = std::function<bool(T)>;
template<class T>
using comparer = std::function<bool(T, T)>;
template<class T>
using equivClass = std::vector<T>;
template<class T>
using equivClasses = LazySeq<equivClass<T>>;
using wide_size_t = unsigned long long;
#define WIDE_SIZE_T_MAX UINT64_MAX
using natural_t = unsigned long long;
using integer_t = signed long long;
using rational_t = std::pair<integer_t, natural_t>;
template<class T>
using indexed_t = std::pair<wide_size_t, T>;

template<class T>
constexpr T identity(const T &x);

template<class T>
class LazySeq {
 public:
  //DONE get rid of useless recursion
  //DONE orderBy functions
  //TODO get rid of copying data
  //DONE from init_list

  //DONE operators
  //DONE Range
  //DONE Repeat
  //DONE Bad filter
  //DONE from/toSTL functions
  //DONE OrderBy
  //DONE get rid of necessity of T()
  //DONE ::evaluator_
  //DONE rvalue bug fixing
  //DONE make public private
  //TODO add versions with specified (or default) comparer
  //DONE add versions with .map based on index
  //TODO add parallel functions support
  //TODO check laziness with natural numbers
  //TODO add ...OrDefault method versions
  //DONE simplify lambdas (c++14)

  typedef LazyIterator<T> const_iterator;
  typedef T value_type;
  typedef wide_size_t size_type;
  typedef signed long long difference_type;
  typedef std::allocator<T> allocator_type;
  typedef T &reference;
  typedef const T &const_reference;
  typedef typename std::allocator_traits<allocator_type>::pointer pointer;
  typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;
  [[nodiscard]] constexpr const_iterator begin() const;
  [[nodiscard]] constexpr const_iterator end() const;
  using skip_helper_t = std::function<std::pair<wide_size_t, LazySeq<T>>(wide_size_t)>;
  using skip_helper_ptr_t = std::shared_ptr<skip_helper_t>;

  /*implicit */LazySeq<T>(std::initializer_list<T> list);
  template<class Iter, class... Args>
  constexpr LazySeq<T>(Iter first, Iter last, Args... captured);

  constexpr LazySeq<T>();
  constexpr LazySeq<T>(const LazySeq<T> &) = default;
  constexpr LazySeq<T>(LazySeq<T> &&) noexcept = default;
  constexpr explicit LazySeq<T>(fabric_ptr<T> fabric, skip_helper_ptr_t skipHelper = nullptr);
  constexpr explicit LazySeq<T>(wide_size_t count, const T &value);
  constexpr explicit LazySeq<T>(const T &initializer, const std::function<T(T)> &next);
  constexpr explicit LazySeq<T>(const fabric<T> &fab);
  constexpr explicit LazySeq<T>(const node_ptr<T> &nodePtr);
  constexpr explicit LazySeq<T>(const node<T> &node1);
  constexpr explicit LazySeq<T>(const std::function<LazySeq<T>()> &generator);
  [[nodiscard]] constexpr LazySeq<T> setSkipHelper(const skip_helper_t &specialSkip) const;

  virtual ~LazySeq() = default;

  template<class Container>
  [[nodiscard]] auto toContainer() const;
  template<class Container, class R>
  auto toContainer(const std::function<R(T)> &func) const;
  template<class Container, class R>
  auto toContainerByIndex(const std::function<R(indexed_t<T>)> &func) const;
  template<class Container, class K, class V>
  auto toContainer(const std::function<K(T)> &keyFunc, const std::function<V(T)> &valueFunc) const;
  template<class Container, class K, class V>
  auto toContainerByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                          const std::function<V(indexed_t<T>)> &valueFunc) const;

  [[nodiscard]] std::vector<T> toVector() const;
  template<class R>
  std::vector<R> toVector(const std::function<R(T)> &func) const;
  template<class R>
  std::vector<R> toVectorByIndex(const std::function<R(indexed_t<T>)> &func) const;
  [[nodiscard]] std::set<T> toSet() const;
  template<class R>
  std::set<R> toSet(const std::function<R(T)> &func) const;
  template<class R>
  std::set<R> toSetByIndex(const std::function<R(indexed_t<T>)> &func) const;
  [[nodiscard]] std::unordered_set<T> toUnorderedSet() const;
  template<class R>
  std::unordered_set<R> toUnorderedSet(const std::function<R(T)> &func) const;
  template<class R>
  std::unordered_set<R> toUnorderedSetByIndex(const std::function<R(indexed_t<T>)> &func) const;
  template<class K, class V>
  std::map<K, V> toMap(const std::function<std::pair<K, V>(T)> &func) const;
  template<class K, class V>
  std::map<K, V> toMapByIndex(const std::function<std::pair<K, V>(indexed_t<T>)> &func) const;
  template<class K, class V>
  std::map<K, V> toMap(const std::function<K(T)> &keyFunc, const std::function<V(T)> &valueFunc) const;
  template<class K, class V>
  std::map<K, V> toMapByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                              const std::function<V(indexed_t<T>)> &valueFunc) const;
  template<class K, class V>
  std::unordered_map<K, V> toUnorderedMap(const std::function<std::pair<K, V>(T)> &func) const;
  template<class K, class V>
  std::unordered_map<K, V> toUnorderedMapByIndex(const std::function<std::pair<K, V>(indexed_t<T>)> &func) const;
  template<class K, class V>
  std::unordered_map<K, V> toUnorderedMap(const std::function<K(T)> &keyFunc,
                                          const std::function<V(T)> &valueFunc) const;
  template<class K, class V>
  std::unordered_map<K, V> toUnorderedMapByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                                                 const std::function<V(indexed_t<T>)> &valueFunc) const;

  [[nodiscard]] std::multiset<T> toMultiset() const;
  template<class R>
  std::multiset<R> toMultiset(const std::function<R(T)> &func) const;
  template<class R>
  std::multiset<R> toMultisetByIndex(const std::function<R(indexed_t<T>)> &func) const;
  [[nodiscard]] std::unordered_multiset<T> toUnorderedMultiset() const;
  template<class R>
  std::unordered_multiset<R> toUnorderedMultiset(const std::function<R(T)> &func) const;
  template<class R>
  std::unordered_multiset<R> toUnorderedMultisetByIndex(const std::function<R(indexed_t<T>)> &func) const;
  template<class K, class V>
  std::multimap<K, V> toMultimap(const std::function<std::pair<K, V>(T)> &func) const;
  template<class K, class V>
  std::multimap<K, V> toMultimapByIndex(const std::function<std::pair<K, V>(indexed_t<T>)> &func) const;
  template<class K, class V>
  std::multimap<K, V> toMultimap(const std::function<K(T)> &keyFunc, const std::function<V(T)> &valueFunc) const;
  template<class K, class V>
  std::multimap<K, V> toMultimapByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                                        const std::function<V(indexed_t<T>)> &valueFunc) const;
  template<class K, class V>
  std::unordered_multimap<K, V> toUnorderedMultimap(const std::function<std::pair<K, V>(T)> &func) const;
  template<class K, class V>
  std::unordered_multimap<K, V> toUnorderedMultimapByIndex(
      const std::function<std::pair<K, V>(indexed_t<T>)> &func) const;
  template<class K, class V>
  std::unordered_multimap<K, V> toUnorderedMultimap(const std::function<K(T)> &keyFunc,
                                                    const std::function<V(T)> &valueFunc) const;
  template<class K, class V>
  std::unordered_multimap<K, V> toUnorderedMultimapByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                                                           const std::function<V(indexed_t<T>)> &valueFunc) const;

  [[nodiscard]] constexpr node_ptr<T> eval() const;

  [[nodiscard]] constexpr T first() const;
  [[nodiscard]] constexpr LazySeq<T> rest() const;
  [[nodiscard]] constexpr T first(const predicate<T> &pred) const;
  [[nodiscard]] constexpr T firstByIndex(const predicate<indexed_t<T>> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> rest(const predicate<T> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> rest(const T &item) const;
  [[nodiscard]] constexpr LazySeq<T> restByIndex(const predicate<indexed_t<T>> &pred) const;
  [[nodiscard]] constexpr T itemAt(wide_size_t index) const;
  constexpr wide_size_t indexOf(const T &item, wide_size_t index = 0);
  [[nodiscard]] constexpr LazySeq<indexed_t<T>> getIndexed() const;

  [[nodiscard]] constexpr bool isEmpty() const;
  [[nodiscard]] constexpr explicit operator bool() const;

  [[nodiscard]] constexpr LazySeq<T> filter(const predicate<T> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> filter(const T &item) const;
  [[nodiscard]] constexpr LazySeq<T> filterByIndex(const predicate<indexed_t<T>> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> remove(const predicate<T> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> remove(const T &item) const;
  [[nodiscard]] constexpr LazySeq<T> removeByIndex(const predicate<indexed_t<T>> &pred) const;

  template<class R>
  constexpr LazySeq<R> mapByNode(const std::function<node<R>(node_ptr<T>)> &f) const;
  template<class R>
  constexpr LazySeq<R> mapByNode(const std::function<node_ptr<R>(node_ptr<T>)> &f) const;
  template<class R>
  constexpr LazySeq<R> map(const std::function<R(T)> &func) const;
  template<class R>
  constexpr LazySeq<R> mapByIndex(const std::function<R(indexed_t<T>)> &func) const;
  template<class Container>
  constexpr auto mapMany(const std::function<Container(T)> &func) const;
  template<class Container>
  constexpr auto mapManyByIndex(const std::function<Container(indexed_t<T>)> &func) const;
  constexpr LazySeq<T> mapIf(const std::function<T(T)> &func, const predicate<T> &pred) const;
  constexpr LazySeq<T> mapIfByIndex(const std::function<T(indexed_t<T>)> &func,
                                    const predicate<indexed_t<T>> &pred) const;

  template<class R = void>
  constexpr void foreach(const std::function<R(T)> &func = [](const T &) {}) const;
  template<class R = void>
  constexpr void foreachByIndex(const std::function<R(indexed_t<T>)> &func) const;
  T reduce(const std::function<T(T, T)> &func, T &&defaultValue = T()) const;
  template<class R>
  R reduce(const R &init, const std::function<R(R, T)> &func) const;

  [[nodiscard]] constexpr LazySeq<T> concat(const LazySeq<T> &other) const;
  [[nodiscard]] constexpr LazySeq<T> repeat(wide_size_t count) const;

  template<class R>
  constexpr LazySeq<std::pair<T, R>> match(const LazySeq<R> &other) const;
  template<class S, class R>
  constexpr LazySeq<R> match(const LazySeq<S> &other, const std::function<R(std::pair<T, S>)> &func) const;
  template<class S, class R>
  constexpr LazySeq<R> matchByIndex(const LazySeq<S> &other,
                                    const std::function<R(indexed_t<std::pair<T, S>>)> &func) const;

  [[nodiscard]] constexpr bool every(const predicate<T> &pred) const;
  [[nodiscard]] constexpr bool every(const T &item) const;
  [[nodiscard]] constexpr bool everyByIndex(const predicate<indexed_t<T>> &pred) const;
  [[nodiscard]] constexpr bool any(const predicate<T> &pred) const;
  [[nodiscard]] constexpr bool any(const T &item) const;
  [[nodiscard]] constexpr bool anyByIndex(const predicate<indexed_t<T>> &pred) const;
  [[nodiscard]] constexpr bool none(const predicate<T> &pred) const;
  [[nodiscard]] constexpr bool none(const T &item) const;
  [[nodiscard]] constexpr bool noneByIndex(const predicate<indexed_t<T>> &pred) const;
  [[nodiscard]] constexpr bool contains(const T &value) const;
  [[nodiscard]] constexpr LazySeq<T> take(wide_size_t count = 1) const;
  [[nodiscard]] constexpr LazySeq<T> takeWhile(const predicate<T> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> takeWhile(const T &item) const;
  [[nodiscard]] constexpr LazySeq<T> takeWhileByIndex(const predicate<indexed_t<T>> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> skip(wide_size_t count = 1) const;
  [[nodiscard]] constexpr LazySeq<T> skipWhile(const predicate<T> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> skipWhile(const T &item) const;
  [[nodiscard]] constexpr LazySeq<T> skipWhileByIndex(const predicate<indexed_t<T>> &pred) const;
  [[nodiscard]] wide_size_t count() const;
  [[nodiscard]] wide_size_t count(const predicate<T> &pred) const;
  [[nodiscard]] wide_size_t count(const T &item) const;
  [[nodiscard]] wide_size_t countByIndex(const predicate<indexed_t<T>> &pred) const;

  [[nodiscard]] constexpr LazySeq<T> emplaceFront(const T &value) const;
  [[nodiscard]] constexpr LazySeq<T> emplaceBack(const T &value) const;

  [[nodiscard]] auto sum() const;
  template<class R>
  auto sum(const std::function<R(T)> &f) const;
  template<class R>
  auto sumByIndex(const std::function<R(indexed_t<T>)> &f) const;
  template<class R>
  [[nodiscard]] auto average() const;
  template<class R, class C>
  auto average(const std::function<R(T)> &f) const;
  template<class R, class C>
  auto averageByIndex(const std::function<R(indexed_t<T>)> &f) const;
  [[nodiscard]] auto subtract() const;
  template<class R>
  auto subtract(const std::function<R(T)> &f) const;
  template<class R>
  auto subtractByIndex(const std::function<R(indexed_t<T>)> &f) const;
  [[nodiscard]] auto multiply() const;
  template<class R>
  auto multiply(const std::function<R(T)> &f) const;
  template<class R>
  auto multiplyByIndex(const std::function<R(indexed_t<T>)> &f) const;
  [[nodiscard]] auto divide() const;
  template<class R>
  auto divide(const std::function<R(T)> &f) const;
  template<class R>
  auto divideByIndex(const std::function<R(indexed_t<T>)> &f) const;
  [[nodiscard]] T min(const comparer<T> &comp = std::less<T>()) const;
  template<class R>
  T min(const std::function<R(T)> &f, const comparer<R> &comp = std::less<R>()) const;
  template<class R>
  T minByIndex(const std::function<R(indexed_t<T>)> &f, const comparer<R> &comp = std::less<R>()) const;
  [[nodiscard]] T max(const comparer<T> &comp = std::less<T>()) const;
  template<class R>
  T max(const std::function<R(T)> &f, const comparer<R> &comp = std::less<R>()) const;
  template<class R>
  T maxByIndex(const std::function<R(indexed_t<T>)> &f, const comparer<R> &comp = std::less<R>()) const;
  [[nodiscard]] std::pair<T, T> minMax(const comparer<T> &comp = std::less<T>()) const;
  template<class R>
  std::pair<T, T> minMax(const std::function<R(T)> &f, const comparer<R> &comp = std::less<R>()) const;
  template<class R>
  std::pair<T, T> minMaxByIndex(const std::function<R(indexed_t<T>)> &f,
                                const comparer<R> &comp = std::less<R>()) const;

  [[nodiscard]] std::string toString(const std::string &separator = " ") const;
  template<class R>
  std::string toString(const std::function<R(T)> &f, const std::string &separator = " ") const;
  template<class R>
  std::string toStringByIndex(const std::function<R(indexed_t<T>)> &f, const std::string &separator = " ") const;

  [[nodiscard]] T last() const;
  [[nodiscard]] T last(const predicate<T> &pred) const;
  [[nodiscard]] T lastByIndex(const predicate<indexed_t<T>> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> butLast() const;
  [[nodiscard]] constexpr LazySeq<T> butLast(const predicate<T> &pred) const;
  [[nodiscard]] constexpr LazySeq<T> butLast(const T &item) const;
  [[nodiscard]] constexpr LazySeq<T> butLastByIndex(const predicate<indexed_t<T>> &pred) const;

  [[nodiscard]] virtual ReversedLazySeq<T> reverse() const;
  template<class R>
  constexpr ReversedLazySeq<R> reverse(const std::function<R(T)> &func) const;
  template<class R>
  constexpr ReversedLazySeq<R> reverseByIndex(const std::function<R(indexed_t<T>)> &func) const;

  template<class R>
  constexpr LazySeq<R> castTo() const;
  template<class R>
  constexpr LazySeq<R> staticCastTo() const;
  template<class R>
  constexpr LazySeq<R> reinterpretCastTo() const;
  template<class R>
  constexpr LazySeq<R> constCastTo() const;
  template<class R>
  constexpr LazySeq<R> dynamicCastTo() const;

  LazySeq<T> &operator=(const LazySeq<T> &) = default;
  LazySeq<T> &operator=(LazySeq<T> &&) noexcept = default;

  template<class R>
  bool operator==(const LazySeq<R> &other) const;
  template<class R>
  bool operator!=(const LazySeq<R> &other) const;
  template<class R>
  bool operator<(const LazySeq<R> &other) const;
  template<class R>
  bool operator>(const LazySeq<R> &other) const;
  template<class R>
  bool operator<=(const LazySeq<R> &other) const;
  template<class R>
  bool operator>=(const LazySeq<R> &other) const;
  constexpr LazySeq<T> &operator+=(const LazySeq<T> &other);
  constexpr LazySeq<T> &operator+=(const T &item);
  constexpr LazySeq<T> &operator*=(wide_size_t count);
  template<class R>
  constexpr LazySeq<std::pair<T, R>> operator*(const LazySeq<R> &other) const;
  template<wide_size_t n>
  [[nodiscard]] constexpr auto pow() const;

  [[nodiscard]] constexpr LazySeq<T> distinct() const;
  [[nodiscard]] constexpr LazySeq<T> except(const LazySeq<T> &other) const;
  [[nodiscard]] constexpr LazySeq<T> intersect(const LazySeq<T> &other) const;
  [[nodiscard]] constexpr LazySeq<T> unite(const LazySeq<T> &other) const;

  [[nodiscard]] LazySeq<LazySeq<T>> groupBy(wide_size_t portion) const;

  template<class F>
  LazySeq<std::pair<F, LazySeq<T>>> groupBy(const std::function<F(T)> &keyFinder) const;

  template<class F, class R>
  LazySeq<std::pair<F, R>> groupBy(const std::function<F(T)> &keyFinder,
                                   const std::function<R(LazySeq<T>)> &seqFunc) const;
  template<class F, class R>
  LazySeq<std::pair<F, R>> groupByIndexBy(const std::function<F(indexed_t<T>)> &keyFinder,
                                          const std::function<R(LazySeq<indexed_t<T>>)
                                          > &seqFunc) const;

  template<class F, class S, class R = LazySeq<S>>
  LazySeq<std::pair<F, R>> groupBy(const std::function<F(T)> &keyFinder,
                                   const std::function<S(T)> &valueFunc = identity<T>,
                                   const std::function<R(LazySeq<S>)> &seqFunc = identity<LazySeq<S>>) const;
  template<class F, class S = T, class R = LazySeq<S>>
  LazySeq<std::pair<F, R>> groupByIndexBy(const std::function<F(indexed_t<T>)> &keyFinder,
                                          const std::function<S(indexed_t<T>)> &valueFunc =
                                          [](indexed_t<T> pair) -> T { return pair.second; },
                                          const std::function<R(LazySeq<S>)> &seqFunc = identity<LazySeq<S>>) const;

  [[nodiscard]] constexpr OrderedLazySeq<T> orderBy(const comparer<T> &comp = std::less<T>()) const;
  [[nodiscard]] constexpr OrderedLazySeq<T> orderByDescending(const comparer<T> &comp = std::less<T>()) const;
  template<class R>
  constexpr OrderedLazySeq<T> orderBy(const std::function<R(T)> &func,
                                      const comparer<R> &comp = std::less<R>()) const;
  template<class R>
  constexpr OrderedLazySeq<indexed_t<T>> orderByIndexBy(const std::function<R(indexed_t<T>)> &func,
                                                        const comparer<R> &comp = std::less<R>()) const;
  template<class R>
  constexpr OrderedLazySeq<T> orderByDescending(const std::function<R(T)> &func,
                                                const comparer<R> &comp = std::less<R>()) const;
  template<class R>
  constexpr OrderedLazySeq<indexed_t<T>> orderByDescendingByIndexBy(const std::function<R(indexed_t<T>)> &func,
                                                                    const comparer<R> &comp = std::less<R>()) const;

  [[nodiscard]] constexpr bool hasSpecialSkipHelper() const;
  [[nodiscard]] auto applySkipHelper(wide_size_t count) const;

 private:
  fabric_ptr<T> evaluator_;
  skip_helper_ptr_t skipHelper_ = nullptr;

  constexpr bool setAndCheckIfNotEmpty(node_ptr<T> &res) const;

  [[nodiscard]] constexpr OrderedLazySeq<T> makeOrdered() const;
};

namespace std {
template<class T>
struct hash<LazySeq<T>> {
  size_t operator()(const LazySeq<T> &seq) const {
    const size_t k = 65537; // Constant for polynomial hash
    return seq.LazySeq<T>::template reduce<size_t>(0, [k](size_t hash, const T &cur) {
      return hash * k + std::hash<T>()(cur);
    });
  }
};
}

template<class T>
class LazyIterator : public std::iterator<std::input_iterator_tag, T> {
 public:
  constexpr LazyIterator<T>(const LazyIterator<T> &other) = default;

  constexpr explicit LazyIterator<T>(const LazySeq<T> &lazySeq);

  virtual ~LazyIterator() = default;

  constexpr T operator*() const;

  constexpr bool operator!=(const LazyIterator<T> &other) const;
  constexpr bool operator==(const LazyIterator<T> &other) const;

  LazyIterator<T> &operator=(const LazyIterator<T> &other) = default;

  constexpr T operator->() const;

  constexpr LazyIterator<T> &operator++();
  const LazyIterator<T> operator++(int);
  LazyIterator<T> operator+(wide_size_t count) const;
  LazyIterator<T> operator+=(wide_size_t count);

 private:
  node_ptr<T> evaluated;
};
template<class T>
constexpr static comparer<T> descendingComparer(const comparer<T> &comp);

template<class Iter, class... Args>
LazySeq(Iter begin, Iter end, Args... captured) -> LazySeq<typename std::iterator_traits<Iter>::value_type>;

template<class LazySeq>
constexpr auto operator+(const LazySeq &a, const LazySeq &b);
template<class T>
constexpr LazySeq<T> operator+(const LazySeq<T> &a, const T &item);
template<class T>
constexpr LazySeq<T> operator*(const LazySeq<T> &a, wide_size_t count);

template<class... Args>
constexpr std::function<bool(Args...)> negate(const std::function<bool(Args...)> &pred);
//TODO replace with std::...

template<class T>
constexpr predicate<T> dividesBy(const T &n);

template<class T>
constexpr bool even(const T &item);
template<class T>
constexpr bool odd(const T &item);

template<class T>
constexpr auto constantly(const T &item);

template<class F, class... Args>
constexpr decltype(auto) partial(const F &f, const Args&... args);

template<class T>
constexpr T increment(T obj);
template<class T>
constexpr T decrement(T obj);

template<class T>
constexpr predicate<T> isEqualTo(const T &item);
template<class T>
constexpr predicate<T> isNotEqualTo(const T &item);
template<class T>
constexpr predicate<T> isLessThan(const T &item);
template<class T>
constexpr predicate<T> isGreaterThan(const T &item);
template<class T>
constexpr predicate<T> isNotLessThan(const T &item);
template<class T>
constexpr predicate<T> isNotGreaterThan(const T &item);

template<class T>
constexpr LazySeq<T> operator*(wide_size_t count, const LazySeq<T> &seq);

template<class T>
constexpr LazySeq<T> operator+(const T &item, const LazySeq<T> &seq);

template<class T>
constexpr LazySeq<T> range(const T &start, wide_size_t count);
template<class T>
constexpr LazySeq<T> infiniteRange(const T &start);

template<class T>
constexpr LazySeq<T> join(const LazySeq<LazySeq<T>> &seq);
template<class Container>
constexpr auto join(const LazySeq<Container> &seq);
template<class Container>
constexpr auto join(const ReversedLazySeq<Container> &seq);

template<class Container>
auto makeLazy(Container &&container);

template<class T>
constexpr auto square(const T &seq);

template<class T>
std::ostream &operator<<(std::ostream &out, const LazySeq<T> &seq);

template<class Container>
LazySeq<size_t> indexesOf(const Container &container);
template<class T>
LazySeq<wide_size_t> indexesOf(const LazySeq<T> &seq);
template<class T>
constexpr LazySeq<std::pair<wide_size_t, T>> indexed(const LazySeq<T> &seq);
template<template<class> class LazySeq, class K, class V>
constexpr LazySeq<K> keys(const LazySeq<std::pair<K, V>> &seq);
template<template<class> class LazySeq, class K, class V>
constexpr LazySeq<V> values(const LazySeq<std::pair<K, V>> &seq);
LazySeq<wide_size_t> indexes(wide_size_t start = 0);
LazySeq<natural_t> naturalNumbers();
LazySeq<integer_t> integerNumbers();
LazySeq<rational_t> positiveRationalNumbers();
LazySeq<rational_t> rationalNumbers();

template<class T = natural_t>
constexpr LazySeq<T> powersByFixedBase(const T &base);
template<class T = natural_t>
constexpr T fibonacci(const T &n);
template<class T = natural_t>
constexpr LazySeq<T> fibonacciSeq();
template<class T = natural_t>
constexpr LazySeq<T> factorials();
template<class T = natural_t>
constexpr LazySeq<T> powersByFixedExponent(const T &exponent);
template<class T>
constexpr LazySeq<T> identitySeq(const T &item);

inline wide_size_t getRandomIndex() {
  static std::mt19937_64 gen(std::chrono::system_clock::now().time_since_epoch().count());
  return gen();
}

inline wide_size_t getRandomIndex(wide_size_t max) {
  return getRandomIndex() % max;
}

inline wide_size_t getRandomIndex(wide_size_t min, wide_size_t max) {
  return min + getRandomIndex(max - min);
}

template<class... Args>
LazySeq<wide_size_t> randomNumbers(Args... args);

#include "LazySeq.hpp"

#endif //FUNCTIONAL_LAZY_SEQ_H
