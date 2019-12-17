#include <iostream>
#include <functional>

#include "LazySeqLib.h"
#include "Lambdas.h"
#include "main.h"

#define ll long long

int main() {
  /*testRecursion();
  testSizesOfLazySeqs();
  std::vector<int> vec = {1, 2, 3, 4, 5, 6, 10};
  auto lazy = testMakeLazy<int>(vec);
  LazySeq<int> lazy1 = testNodeConstructor(lazy);

  testMap(lazy1);
  LazySeq<int> even = testFilterRemove(lazy1);
  testReduce(even);
  testCast(even);

  testConcat(lazy);
  testJoinMapManyMapIf();
  testFromInitList();
  testRange();

  testRepeating();
  testCartesianMultiplication();
  testEveryAnyNoneContains(lazy);
  testSkipsTakes();

  testCount();
  testArithmeticOperations();
  testMinMax();
  testToString();

  testLastButLast();
  testOutputOperator();
  testEqualityOperator();
  testMatch();

  testToCollectionFunctions();
  testReverse();
  testMakeLazyPaired();
  testFirstRestElementAt();

  testEmplace();
  testDistinctUnionExceptIntersect();
  testGroupBy();
*/
  testOrder();
  /*
  testIndexed();
  testComparisons();

  testHash();
  testSetsOfLazySeq();
  testForeach();

  testFunctionsByIndex();
  testSimpleFunctions();
  testRandom();
  testSmartSkips();
*/
  return 0;
}

namespace {
template<class T>
T trace(T x) {
  std::cout << x << std::endl;
  return x;
}

void trace() {
  std::cout << std::endl;
}

template<>
bool trace(bool x) {
  std::cout << (x ? "true" : "false") << std::endl;
  return x;
}

template<class K, class V>
std::pair<K, V> tracePair(const std::pair<K, V> &pair) {
  std::cout << "[" << pair.first << ", " << pair.second << "] ";
  return pair;
}

template<class K, class V>
LazySeq<std::pair<K, V>> trace(const LazySeq<std::pair<K, V>> &seq) {
  seq.foreach(tracePair<K, V>);
  std::cout << std::endl;
  return seq;
}

template<class T, template<class> class Container>
void traceCollection(const Container<T> &container) {
  for (auto item: container) {
    std::cout << item << ' ';
  }
  std::cout << std::endl;
}

template<class K, class V, template<class, class> class Container>
void traceCollection(const Container<K, V> &container) {
  for (const auto &item: container) {
    std::cout << '[' << item.first << ":\t" << item.second << "]\n";
  }
  std::cout << std::endl;
}
}

void testSmartSkips() {
  trace("testSmartSkips");
  trace(identitySeq(3).skip(1000000000000ull).take(15));
  trace(randomNumbers(1, 10).skip(1000000000).take(15));
  trace(powersByFixedExponent<natural_t>(2).skip(1000000).take(15));
  trace(powersByFixedExponent<natural_t>(2).skip(500000).skip(500000).take(15));
  trace(powersByFixedBase<natural_t>(2).skip(1).take(15));
  for (auto count: {0, 1, 2, 3, 5}) {
    trace(fibonacciSeq().skip(count).take(15));
  }
  trace(infiniteRange(100).skip(1000000).take(15));
  trace(range(100, 2000000).skip(1000000).take(15));
  trace(range(0, 10000000).match(infiniteRange(1000)).skip(1000000).take(15));
  trace(makeLazy(std::vector(1000000, 1)).skip(999990));
  trace(makeLazy(std::list(100, 1)).skip(90));
  trace(makeLazy(std::initializer_list<int>{1, 2, 3}).skip());
  auto initList = {1, 2, 3};
  auto vec = std::vector(initList);
  auto list = std::list(initList);
  trace(LazySeq(initList.begin(), initList.end()).skip());
  trace(LazySeq(vec.begin(), vec.end()).skip());
  trace(LazySeq(list.begin(), list.end()).skip());
  trace(LazySeq(1000000ull, 1).concat(LazySeq{3}).skip(999990));
  trace(LazySeq(1000000ull, 1).concat(LazySeq{0, 0, 0}).skip(1000000));
  trace(LazySeq(1000000ull, 1).concat(LazySeq{0, 0, 0}).skip(1000001));
  trace(LazySeq(1000000ull, 1).concat(LazySeq{0, 0, 0}).skip(1000003));
  trace(LazySeq(1000000ull, 1).concat(LazySeq{0, 0, 0}).skip(1000010));
  trace(range(1, 10).concat(range(100, 10)).skip(10));
  trace(range(1, 10).concat(range(100, 10)).skip(0));
  trace(join(LazySeq(10, std::vector(1000000, 3))).skip(9999990));
  std::vector<int> simpleVector = range(1, 10).toVector();
  trace(join(LazySeq(10, simpleVector)).skip(89));
  trace(join(LazySeq(10, simpleVector)).skip(90));
  trace(join(LazySeq(10, simpleVector)).skip(91));
  trace(join(LazySeq(10, simpleVector)).skip(100));
  trace(join(LazySeq(10, simpleVector)).skip(900));
  trace(join(LazySeq(3, std::vector{1, 2, 3})).skip(0));
  trace(makeLazy(std::vector(1000000, 3)).repeat(100000).skip(99999999990ull));
  trace(makeLazy(simpleVector).repeat(10).skip(89));
  trace(makeLazy(simpleVector).repeat(10).skip(90));
  trace(makeLazy(simpleVector).repeat(10).skip(91));
  trace(makeLazy(simpleVector).repeat(10).skip(100));
  trace(makeLazy(simpleVector).repeat(10).skip(101));
  trace(makeLazy(std::vector{1, 2}).repeat(4).skip(0));
  trace(makeLazy(std::vector{1, 2}).repeat(4).skip(1));
  trace(makeLazy(std::vector{1, 2}).repeat(0));
  trace(makeLazy(std::vector{1, 2}).repeat(0).skip(0));
  trace(makeLazy(std::vector{1, 2}).repeat(0).skip(1));
  trace(range(0, 5).map(partial(std::divides<>(), 720)).skip());
  trace(range(1, 1000000000).count());
  trace(range(1, 10).filter(odd<int>).count());
  trace(range(1, 10).count(odd<int>));
  trace(positiveRationalNumbers().take(15));
  trace(positiveRationalNumbers().skip(0).take(15));
  trace(positiveRationalNumbers().skip(3).take(15));
  const auto positiveRational = positiveRationalNumbers().itemAt(400000);
  std::cout << positiveRational.first << "/" << positiveRational.second << std::endl;
  trace(range(1, 10000).emplaceFront(0).skip(0).take(15));
  trace(range(1, 10000).emplaceFront(0).skip(1).take(15));
  trace(range(1, 10000).emplaceFront(0).skip(2).take(15));
  trace(range(1, 10000).emplaceFront(0).skip(100000000).take(15));
  trace(rationalNumbers().take(15));
  for (auto i : range(0, 4)) {
    trace(rationalNumbers().skip(i).take(15));
  }
  trace(rationalNumbers().skip(400000).take(10));
  trace(integerNumbers().skip(400000).take(10));
  trace(integerNumbers().skip(400001).take(10));
  trace(integerNumbers().skip(0).take(10));
  trace(integerNumbers().skip(1).take(10));
  trace(integerNumbers().skip(2).take(10));
  trace(naturalNumbers().skip(400000).take(10));
  trace(naturalNumbers().skip(2).take(10));
  trace(naturalNumbers().rest().hasSpecialSkipHelper());
  trace(naturalNumbers().rest().first());
  trace(naturalNumbers().rest().skip().first());
  trace(naturalNumbers().rest().rest().skip().first());
  trace(naturalNumbers().rest().rest().skip(1000000000000ull).first());
  for (const auto &seq: {naturalNumbers().groupBy(5), naturalNumbers().groupBy(5).skip(2),
                         naturalNumbers().filter(isGreaterThan(0)).groupBy(5).skip(2),
                         naturalNumbers().groupBy(0).skip(1).take(3),
                         naturalNumbers().groupBy(3).skip(0).take(5),
                         naturalNumbers().groupBy(0).skip(0).take(3)}) {
    trace(seq.take(10).toString(fn1(it.toString()), "\n"));
    trace();
  }
  auto groups = infiniteRange(0ull).take(100000000000ull).groupBy(1000000000);
  trace(groups.itemAt(99).take(100));
//  trace(groups.last().last());
  trace(range(1, 10).butLast());
  trace(range(1, 10).butLast().count());
  trace(range(1, 1).butLast());
  trace(range(1, 1).butLast().count());
  trace(range(1, 0).butLast());
  trace(range(1, 0).butLast().count());
  for (auto i: {0ull, 1ull, 100000000000ull}) {
    trace(LazySeq<wide_size_t>(fn0(infiniteRange(0ull))).skip(i).take(10));
  }
}

void testSizesOfLazySeqs() {
  trace("testSizesOfLazySeqs");
  trace(sizeof(LazySeq<int>));
  trace(sizeof(ReversedLazySeq<int>));
  trace(sizeof(OrderedLazySeq<int>));
}

void testRandom() {
  trace("testRandom");
  trace(getRandomIndex());
  trace(randomNumbers().take(30));
  trace(randomNumbers(10).take(30));
  trace(randomNumbers(1, 3).take(30));
}

void testSimpleFunctions() {
  trace("testSimpleFunctions");
  const auto f1 = dividesBy(3);
  trace(f1(1));
  trace(f1(2));
  trace(f1(3));
  trace(even(1));
  trace(even(2));
  trace(odd(1));
  trace(odd(2));
  const auto f2 = constantly("cat");
  trace(f2(3));
  trace(f2());
  trace(f2(std::vector(1, 1), std::hash<int>()));
  auto r = range(1, 10);
  trace(r.takeWhile(isLessThan(3)));
  trace(r.takeWhile(isNotGreaterThan(3)));
  trace(r.reverse().takeWhile(isGreaterThan(3)));
  trace(r.reverse().takeWhile(isNotLessThan(3)));
  trace(r.takeWhile(isEqualTo(1)));
  trace(r.takeWhile(isNotEqualTo(5)));
  trace(identitySeq(4).take(15));
  trace(partial(std::plus<>(), 23)(2));
}

void testFunctionsByIndex() {
  trace("testFunctionsByIndex");
  auto range1to5 = range(1, 5);
  auto seq = 2 * range1to5.reverse();
  auto toString = fn1("[" + std::to_string(it.first) + ": " + std::to_string(it.second) + "] ");
  auto write = [](auto coll) {
    for (const auto &item: coll) {
      std::cout << item << ' ';
    }
    trace();
  };
  auto writePairs = [](auto coll) {
    for (const auto &item: coll) {
      std::cout << "[" << item.first << ": " << item.second << "] ";
    }
    trace();
  };
  std::function<int(indexed_t<int>)> value = fn1(it.second);
  std::function<indexed_t<int>(indexed_t<int>)> pairs = identity<indexed_t<int>>;

  write(seq.toVectorByIndex(value));
  write(seq.toSetByIndex(value));
  write(seq.toUnorderedSetByIndex(value));
  write(seq.toUnorderedSetByIndex<std::string>(toString));
  trace();
  write(seq.toMultisetByIndex(value));
  write(seq.toUnorderedMultisetByIndex(value));
  write(seq.toUnorderedMultisetByIndex<std::string>(toString));
  trace();
  writePairs(seq.toMapByIndex(pairs));
  writePairs(seq.toUnorderedMapByIndex(pairs));
  writePairs(seq.toMultimapByIndex(pairs));
  writePairs(seq.toUnorderedMultimapByIndex(pairs));
  trace();
  writePairs(seq.toMapByIndex(value, value));
  writePairs(seq.toUnorderedMapByIndex(value, value));
  writePairs(seq.toMultimapByIndex(value, value));
  writePairs(seq.toUnorderedMultimapByIndex(value, value));
  trace();
  trace();

  auto pred = fn1(it.first % 2 != 0);
  trace(seq.firstByIndex(pred));
  trace(seq.restByIndex(pred));

  trace(range('a', 26).filterByIndex(pred));
  trace(range('a', 20).removeByIndex(pred));
  trace();
  trace(range('a', 15).mapByIndex(fn1(std::string(1 + it.first, it.second))));
  trace(seq.mapManyByIndex(fn1((LazySeq{(int) it.first, it.second}))));
  trace(seq.mapManyByIndex(fn1((std::vector{(int) it.first, it.second}))));
  trace(range(100, 10).mapIfByIndex(fn1(2 * it.second), fn1(it.first % 3 == 0)));
  trace();
  range('a', 8).foreachByIndex([](auto pair) { std::cout << pair.first << pair.second << ' '; });
  trace();
  trace(range('a', 10)
            .matchByIndex(integerNumbers(),
                          fn1(std::to_string(it.first) + ":\t"
                                  + std::string(1, it.second.first) + " "
                                  + std::to_string(it.second.second) + "\n"))
            .toString(""));
  trace();
  auto cond = fn1(it.first % 2 == 0 && it.second < 0);
  for (const auto &data:
      range(0, 3).map(fn1(integerNumbers().skip(it).take(10)))
          * LazySeq<std::function<bool(indexed_t<int>)>>{cond, negate(cond)}) {
    trace(data.first);
    trace(data.first.mapByIndex(data.second));
    trace(data.first.noneByIndex(data.second));
    trace(data.first.anyByIndex(data.second));
    trace(data.first.everyByIndex(data.second));
  }
  trace();
  auto smallRange = range(1, 20).map(fn1(it / 10.));
  const auto differenceIsSmall = fn1(it.first - it.second < 4);
  trace(smallRange.takeWhileByIndex(differenceIsSmall));
  trace(smallRange.skipWhileByIndex(differenceIsSmall));
  trace(smallRange.countByIndex(differenceIsSmall));
  trace();
  std::function<wide_size_t(indexed_t<int>)> multiplicationCriterion = fn1((it.first + 1) * it.second);
  trace(range1to5);
  trace(range1to5.mapByIndex(multiplicationCriterion));
  trace(range1to5.sumByIndex(multiplicationCriterion));
  trace(-range1to5.subtractByIndex(multiplicationCriterion));
  trace(range1to5.multiplyByIndex(multiplicationCriterion));
  trace(range1to5.mapIfByIndex(constantly(720 * 720), fn1(it.first == 0))
            .divideByIndex(multiplicationCriterion));
  trace(range1to5.averageByIndex(multiplicationCriterion));
  trace(range1to5.minByIndex(multiplicationCriterion));
  trace(range1to5.minByIndex(multiplicationCriterion, std::greater<>()));
  trace(range1to5.maxByIndex(multiplicationCriterion));
  trace(range1to5.maxByIndex(multiplicationCriterion, std::greater<>()));
  tracePair(range1to5.minMaxByIndex(multiplicationCriterion));
  tracePair(range1to5.minMaxByIndex(multiplicationCriterion, std::greater<>()));
  trace();
  trace(rationalNumbers()
            .take(36)
            .toStringByIndex(fn1(std::to_string(it.first + 1)
                                     + ":\t"
                                     + std::to_string(it.second.first) + "/"
                                     + std::to_string(it.second.second)), "\n"));
  trace();
  trace(smallRange.lastByIndex(differenceIsSmall));
  trace(smallRange.butLastByIndex(differenceIsSmall));
  trace();
  writePairs(smallRange.groupByIndexBy(fn1(it.first % 3)));
  writePairs(smallRange.groupByIndexBy(
      fn1(it.first % 3),
      [](const LazySeq<indexed_t<double>> &seq) { return seq.sum(fn1(it.second)); }
  ));
  trace();
}

void testForeach() {
  trace("testForeach");
  range<size_t>(1, 10).foreach(trace<size_t>);
  range<size_t>(1, 10).map(trace<size_t>).foreach();
  auto end = range(0, 0).end();
  auto begin = range(1, 20).begin();
  trace(LazySeq(begin + 0, end));
  trace(LazySeq(begin += 0, end));
  trace(LazySeq(begin + 5, end));
  trace(LazySeq(begin += 5, end));
  trace(LazySeq(begin + 5, end));
  trace(LazySeq(begin += 5, end));
  trace(LazySeq(begin + 25, end));
  trace(LazySeq(begin += 25, end));
}

void testHash() {
  trace("testHash");
  auto a = LazySeq{1, 2, 3};
  trace(std::hash<LazySeq<int>>()(a));
  auto b = a;
  trace(std::hash<LazySeq<int>>()(b));
  auto c = LazySeq{1, 2, 3};
  trace(std::hash<LazySeq<int>>()(c));
  auto d = LazySeq{1, 2, 3, 4};
  trace(std::hash<LazySeq<int>>()(d));
  trace(std::hash<LazySeq<int>>()({1, 2, 3}));
}

void testSetsOfLazySeq() {
  trace("testSetsOfLazySeq");
  std::unordered_set<LazySeq<int>> unordered{};
  std::set<LazySeq<int>> ordered;
  auto equal1 = range(10, 11);
  auto equal2 = equal1;
  for (const auto &seq: {equal1, equal2, LazySeq{1, 2, 3}, LazySeq{1, 2, 3, 4}, LazySeq{1, 2, 3}}) {
    unordered.insert(seq);
    ordered.insert(seq);
  }
  for (const auto &item: ordered) {
    trace(item);
  }
  trace();
  for (const auto &item: unordered) {
    trace(item);
  }
}

void testIndexed() {
  trace("testIndexed");
  trace(indexed(range(10, 15)));
  trace((2 * range(10, 5)).indexOf(10, 1));
  trace(naturalNumbers().take(15));
  trace(integerNumbers().take(15));
  trace(positiveRationalNumbers().take(15));
  trace(rationalNumbers().take(15));
  trace(rationalNumbers()
            .filter(fn1(it.second % 3 == 0))
            .takeWhile(fn1(it.second <= 10)));
  trace(indexesOf(range(100, 4)));
  trace(indexesOf(std::vector(5, 100)));
  auto seq = range('a', 10).map(fn1((std::pair{std::string(4, it), std::string(7, it)})));
  trace(keys(seq));
  trace(values(seq));
  for (const auto &lazySeq: {powersByFixedBase<natural_t>(2), powersByFixedExponent<natural_t>(2),
                             fibonacciSeq(), factorials()}) {
    trace(lazySeq.take(15));
  }
}

void testComparisons() {
  trace("testComparisons");
  auto seq = LazySeq<LazySeq<int>>{{1, 2, 3}, {1, 2, 4}, {1, 2, 5}, {1, 2, 4, 3}};
  for (const auto &pair: square(seq)) {
    trace(pair.first);
    trace(pair.second);
    for (const auto &f: std::vector<std::function<bool(LazySeq<int>, LazySeq<int>)>>
        {std::less<>(), std::greater<>(),
         std::less_equal<>(), std::greater_equal<>(),
         std::equal_to<>(), std::not_equal_to<>()}) {
      std::cout << f(pair.first, pair.second);
    }
    trace();
  }
}

void testOrder() {
  trace("testOrder");
  trace(range(1, 2000).orderByDescendingByItself());
  auto range1 = range(1, 12);
  auto orderedTrace = [](const auto &orderedSeq) {
    trace(orderedSeq);
    using T = typename std::remove_reference<decltype(orderedSeq)>::type::value_type;
    LazySeq<T> commonSeq = orderedSeq;
    trace(commonSeq);
    trace();
  };
  for (const auto &seq: {range1.orderByItself(),
                         range1.orderByDescendingByItself(),
                         range1.orderByItselfWith(std::greater<>()),
                         range1.orderByDescendingByItselfWith(std::greater<>())}) {
    orderedTrace(seq);
  }
  trace();
  auto range2 = range(1, 3);
  const auto range3 = (range2 * 4).match(range1);
  auto seq1 = range3.orderBy(fn1(it.first));
  trace<int, int>(seq1);
  auto seq2 = range3.orderByDescendingBy(fn1(it.first));
  trace<int, int>(seq2);
  trace();
  for (const auto &seq: {seq1.thenBy(fn1(it.second)),
                         seq1.thenByDescendingBy(fn1(it.second)),
                         seq1.thenBy(fn1(it.second), std::greater<>()),
                         seq1.thenByDescendingBy(fn1(it.second), std::greater<>())}) {
    trace<int, int>(seq);
    LazySeq<std::pair<int, int>> common = seq;
    trace(common);
    trace();
  }

  auto seq3 = range(1, 12).orderBy(fn1(std::to_string(it)[0]));
  for (const auto &seq: {seq3, seq3.thenByItself(), seq3.thenByDescendingByItself()}) {
    orderedTrace(seq);
    orderedTrace(seq.map([](int x) {
      auto str = std::to_string(x);
      std::reverse(str.begin(), str.end());
      return str;
    }));
    trace();
  }
  auto seq4 = (square(range2) * range2).map(fn1(std::to_string(it.first.first)
                                                    + std::to_string(it.first.second)
                                                    + std::to_string(it.second)));
  auto charGetter = [](size_t i) { return fn1(it[i]); };
  trace(seq4);
  trace(seq4.orderByDescendingBy(charGetter(0)).thenBy(charGetter(1)));
  trace(seq4.orderBy(charGetter(0))
            .thenByDescendingBy(charGetter(1))
            .thenByDescendingBy(charGetter(2)));
  trace(range(1, 2000).orderByDescendingByItself());
  trace();
  trace(range(1, 100000).orderByDescendingByItself().skip(99690));
  trace();
  trace(static_cast<LazySeq<int>>(range(1, 100000).orderByDescendingByItself()).skip(99690));
  trace();
  trace(range(1, 100000).orderByDescendingByItself().skip(49000).skip(50690));
  trace();
  trace(range(1, 100000).orderByDescendingByItself().map(partial(std::multiplies<>(), 2)).skip(99690));
  trace();
  trace(seq4.orderByDescendingBy(charGetter(0)).skip(3));
  trace(seq4.orderByDescendingBy(charGetter(0)).skip(3).thenByItself());
  trace(seq4.orderByDescendingBy(charGetter(0)).skip(3).thenByDescendingByItself());
  trace(seq4.orderByDescendingBy(charGetter(0)).thenByItself().skip(3));
  trace(seq4.orderByDescendingBy(charGetter(0)).thenByDescendingByItself().skip(3));
  trace(seq4.orderByItself().filter(fn1(it[0] != '1')).skip(1));
  trace(seq4.orderByItself().take(5));
  trace(seq4.orderByItself().take(5).skip(1));
  trace(seq4.orderByItself().skip(1).take(5));
  auto seq5 = seq4.orderBy(charGetter(0)).thenBy(charGetter(1));
  trace(seq5);
  trace(seq5.take(5).thenByDescendingByItself());
  trace(seq5.thenByDescendingByItself().take(5));
  for (const auto &seq: {range(0, 5).orderByItself().take(0), range(0, 5).orderByItself().take(100),
                         range(0, 5).orderByItself().skip(0), range(0, 5).orderByItself().skip(100),
                         range(0, 5).orderByItself().takeWhile(constantly(true)),
                         range(0, 5).orderByItself().takeWhile(constantly(false)),
                         range(0, 5).orderByItself().skipWhile(constantly(true)),
                         range(0, 5).orderByItself().skipWhile(constantly(false)),
                         range(0, 5).orderByItself().takeWhile(isLessThan(3)),
                         range(0, 5).orderByItself().skipWhile(isLessThan(3)),
                         range(0, 5).orderByItself().rest()}) {
    trace(seq);
    trace(seq.skip(2));
    trace();
  }
}

void testGroupBy() {
  trace("testGroupBy");
  const auto keyFinder = fn1(it % 2);
  const std::function<int(int)> valueFunc = fn1(it * 2);
  const auto seqFunc = [](const LazySeq<int> &x) { return x.toString(""); };
  for (const auto &seq: {range(1, 10).groupBy(keyFinder),
                         range(1, 10).groupBy(keyFinder, valueFunc)}) {
    trace(seq);
  }
  for (const auto &seq: {range(1, 10).groupBy(keyFinder, seqFunc),
                         range(1, 10).groupBy(keyFinder, valueFunc, seqFunc)}) {
    trace(seq);
  }
  for (const auto &groups: {range(1, 20).groupBy(5), range(1, 20).groupBy(6), range(1, 20).groupBy(1000),
                            range(1, 20).groupBy(0).take(25),
                            range(1, 0).groupBy(0).take(25),
                            range(1, 0).groupBy(10).take(25)}) {
    trace("start.");
    for (const auto &group: groups) {
      trace(group);
    }
    trace("finish.");
    trace();
  }
}

void testDistinctUnionExceptIntersect() {
  trace("testDistinctUnionExceptIntersect");
  auto seq1 = range(1, 10);
  auto seq2 = range(5, 10);
  trace(seq1.intersect(seq2));
  trace(seq1.unite(seq2));
  trace(seq1.except(seq2));
  trace((2 * seq1).distinct());
}

void testRecursion() {
  trace("testRecursion");
  std::function<ll(ll)> f;
  f = [&](ll value) { return value <= 1 ? value : value * f(value - 1); };
  trace(f(5));
  int done = 0;
  std::function<int(int)> recur;
  recur = [&recur, &done](int i) mutable { return i ? (++done, recur(i - 1)) : done; };
  trace(recur(16283));
}

template<class T>
LazySeq<T> testMakeLazy(const std::vector<int> &vec) {
  trace("testMakeLazy");
  auto vec1 = std::vector{1, 2, 3};
  trace(makeLazy(vec1));
  trace(makeLazy(std::move(vec1)));
  trace(makeLazy(vec1));
  trace(makeLazy(std::initializer_list<int>{1, 2, 3}));
  auto list = {4, 5, 6};
  trace(makeLazy(list));
  trace(makeLazy(naturalNumbers()).take(15));
  return trace(makeLazy(vec));
}

template<class T>
LazySeq<int> testNodeConstructor(const LazySeq<T> &lazy) {
  trace("testNodeConstructor");
  return trace(LazySeq(std::pair<int, LazySeq<int>>{100, lazy}));
}

void testMap(const LazySeq<int> &seq) {
  trace("testMap");
  trace(seq.map(partial(std::multiplies<>(), 3)));
}

LazySeq<int> testFilterRemove(const LazySeq<int> &seq) {
  trace("testFilterRemove");
  const auto pred = even<int>;
  trace(seq.remove(pred));
  trace(seq.remove(constantly(false)));
  trace(seq.remove(constantly(true)));
  trace(seq.filter(constantly(false)));
  trace(seq.filter(constantly(true)));
  trace(range(1, 10).repeat(4).filter(7));
  trace(range(1, 10).repeat(4).remove(7));
  return trace(seq.filter(pred));
}

template<class T>
void testReduce(const LazySeq<T> &even) {
  trace("testReduce");
  trace(even.reduce(std::plus<>()));
  trace(even.template reduce<ll>(0l, [](ll a, int b) { return a * 10 + b; }));
}

template<class T>
void testCast(const LazySeq<T> &even) {
  trace("testCast");
  trace(even.template castTo<float>().first() / 3);
}

template<class T>
void testConcat(const LazySeq<T> &lazy) {
  trace("testConcat");
  trace(lazy.concat({0, 0, 0}));
  trace(lazy + lazy);
}

void testJoinMapManyMapIf() {
  trace("testJoinMapManyMapIf");
  LazySeq<LazySeq<int>> nested = {{1, 2, 3}, {3, 4}};
  LazySeq<std::vector<int>> nestedVec = {{1, 2, 3}, {3, 4}};
  for (const auto &item: nested) {
    trace(item);
  }
  trace(join(nested));
  trace(join(nestedVec));
  trace(range(1, 5).mapMany(fn1('\n' + it * LazySeq{'x'})).toString(""));
  trace(range(1, 5).mapMany(fn1((std::vector{'1', '2', 'a'}))).toString(""));
  trace(range(1, 10).mapIf(fn1(it / 2), even<int>));
}

void testFromInitList() {
  trace("testFromInitList");
  LazySeq fromInit = {1, 10, 100, 1000, 1000, 10000};
  trace(fromInit);
  trace(LazySeq{1, 10, 100, 1000, 1000, 10000});
  std::initializer_list<int> initList = {1, 10, 100, 1000, 1000, 10000};
  fromInit = initList;
  trace(fromInit);
  trace(std::move(fromInit));
}

void testRange() {
  trace("testRange");
  trace(range(10, 10).filter(dividesBy(3)));
  trace(infiniteRange(10).take(10));
}

void testRepeating() {
  trace("testRepeating");
  const auto seq = range(10, 10).filter(dividesBy(3));
  trace(seq.repeat(3));
  trace(seq * 3);
  trace(3 * seq);
  trace(LazySeq(3, seq));
}

void testCartesianMultiplication() {
  trace("testCartesianMultiplication");
  for (auto item: square(range(1, 10))) {
    std::cout << item.first << " * " << item.second << " = " << item.first * item.second << '\n';
  }
  trace(range(1, 10).pow<3>().count());
  range(1, 10).pow<3>().take(200).foreach([](const auto &tup) {
    std::cout << "("
              << std::get<0>(tup) << ", "
              << std::get<1>(tup) << ", "
              << std::get<2>(tup) << ")\n";
  });
  trace(range(2, 3).pow<0>().count());
  std::cout << std::endl;
}

template<class T>
void testEveryAnyNoneContains(const LazySeq<T> &lazy) {
  trace("testEveryAnyNoneContains");
  trace(lazy.every(isLessThan(11)));
  trace(lazy.every(even<T>));
  trace(lazy.any(even<T>));
  trace(lazy.any(isGreaterThan(11)));
  trace(lazy.none(even<T>));
  trace(lazy.none(isGreaterThan(1000)));
  trace(range(1, 10).contains(3));
  trace(range(1, 10).contains(11));
  trace(range(1, 10).every(7));
  trace(range(1, 10).any(7));
  trace(range(1, 10).none(7));
  trace(LazySeq{7}.repeat(10).every(7));
  trace(LazySeq{7}.repeat(10).any(7));
  trace(LazySeq{7}.repeat(10).none(7));
}

void testSkipsTakes() {
  trace("testSkipsTakes");
  trace(range(1, 1000).take(5));
  trace(range(1, 5).take(1000));
  const auto pred = isLessThan(7);
  trace(range(1, 1000).takeWhile(pred));
  trace(range(1, 5).takeWhile(constantly(true)));
  trace(range(1, 10).skip(3));
  trace(range(1, 3).skip(10));
  trace(range(1, 10).skipWhile(pred));
  trace(LazySeq{1, 1, 1, 2, 3}.takeWhile(1));
  trace(LazySeq{1, 1, 1, 2, 3}.skipWhile(1));
  trace(range(1, 10).skipWhile(constantly(true)));
}

void testCount() {
  trace("testCount");
  trace(range(1, 10).count());
  trace(range(1, 10).count(dividesBy(3)));
  trace(range(1, 10).repeat(4).count(5));
}

void testArithmeticOperations() {
  trace("testArithmeticOperations");
  trace(range(1, 4).sum());
  const auto func = partial(std::multiplies<>(), 2);
  trace(range(1, 4).sum(func));
  trace(range(1, 4).average());
  trace(range(1, 4).average(func));
  trace(range(1, 4).subtract());
  trace(range(1, 4).subtract(func));
  trace(range(1, 4).multiply());
  trace(range(1, 4).multiply(func));
  trace(range(1, 4).castTo<float>().divide());
  trace(range(1, 4).castTo<float>().divide(func));
}

void testMinMax() {
  trace("testMinMax");
  trace(range(1, 4).min());
  trace(range(1, 4).min(std::greater<>()));
  trace(range(1, 4).min(fn1(-it)));
  trace(range(1, 4).min(fn1(-it), std::greater<>()));
  trace(range(1, 4).max());
  trace(range(1, 4).max(std::greater<>()));
  trace(range(1, 4).max(fn1(-it)));
  trace(range(1, 4).max(fn1(-it), std::greater<>()));
  tracePair(range(1, 4).minMax());
  tracePair(range(1, 4).minMax(std::greater<>()));
  tracePair(range(1, 4).minMax(fn1(-it)));
  tracePair(range(1, 4).minMax(fn1(-it), std::greater<>()));
  trace();
}

void testToString() {
  trace("testToString");
  std::cout << "[" << range(1, 10).toString(", ") << "]" << std::endl;
}

void testLastButLast() {
  trace("testLastButLast");
  std::cout << range(1, 100).last() << " " << range(10, 0).last() << std::endl;
  trace(range(1, 10).butLast().butLast());
  auto pred = dividesBy(7);
  std::cout << range(1, 100).last(pred) << " " << range(10, 0).last(pred) << std::endl;
  trace(range(1, 70).butLast(pred).butLast(pred));
  trace(LazySeq{1, 2, 3}.repeat(5).butLast(3));
}

void testOutputOperator() {
  trace("testOutputOperator");
  trace(range(1, 200).filter(dividesBy(17)));
}

void testEqualityOperator() {
  trace("testEqualityOperator");
  trace(range(0, 10) == range(0, 10));
  trace(range(0, 10) == range(1, 10));
  trace(range(0, 10) == range(0, 11));
  trace(range(0, 11) == range(0, 10));

  trace(range(0, 10) != range(0, 10));
  trace(range(0, 10) != range(1, 10));
  trace(range(0, 10) != range(0, 11));
  trace(range(0, 11) != range(0, 10));
}

void testMatch() {
  trace("testMatch");
  const auto func = [](std::pair<char, size_t> pair) { return std::string(pair.second, pair.first); };
  trace(range('a', 8).match(range<size_t>(1, 100)).toString(func));
  trace(range('a', 8).match(range<size_t>(1, 100), func));
}

template<template<class> class Container>
void testUnaryCollection(const std::string &name,
                         const Container<int> &cont1,
                         const Container<int> &cont2) {
  trace(name);
  traceCollection<int, Container>(cont1);
  traceCollection<int, Container>(cont2);
  std::cout << std::endl;
}

template<template<class, class> class Container>
void testBinaryCollection(const std::string &name,
                          const Container<int, std::string> &cont1,
                          const Container<int, std::string> &cont2) {
  trace(name);
  traceCollection<int, std::string, Container>(cont1);
  traceCollection<int, std::string, Container>(cont2);
  std::cout << std::endl;
}

void testToCollectionFunctions() {
  trace("testToCollectionFunctions");
  auto seq = 2 * range(1, 10);
  auto unaryFunc = partial(std::multiplies<>(), 2);
  auto keyFunc = identity<int>;
  const auto valueFunc = [](int x) {
    auto seq1 = range<ll>(1, static_cast<wide_size_t>(x));
    return seq1.toString(" * ") + " = " + std::to_string(seq1.multiply());
  };
  const auto KeyValueFunc = [&valueFunc](int x) { return std::pair{x, valueFunc(x)}; };

  testUnaryCollection<std::vector>("Vector", seq.toVector(), seq.toVector<int>(unaryFunc));
  testUnaryCollection<std::set>("Set", seq.toSet(), seq.toSet<int>(unaryFunc));
  testUnaryCollection<std::unordered_set>("Unordered set", seq.toUnorderedSet(), seq.toUnorderedSet<int>(unaryFunc));
  testUnaryCollection<std::multiset>("Multiset", seq.toMultiset(), seq.toMultiset<int>(unaryFunc));
  testUnaryCollection<std::unordered_multiset>("Unordered multiset",
                                               seq.toUnorderedMultiset(),
                                               seq.toUnorderedMultiset<int>(unaryFunc));

  testBinaryCollection<std::map>("Map",
                                 seq.toMap<int, std::string>(KeyValueFunc),
                                 seq.toMap<int, std::string>(keyFunc, valueFunc));
  testBinaryCollection<std::unordered_map>("Unordered map",
                                           seq.toUnorderedMap<int, std::string>(KeyValueFunc),
                                           seq.toUnorderedMap<int, std::string>(keyFunc, valueFunc));
  testBinaryCollection<std::multimap>("Multimap",
                                      seq.toMultimap<int, std::string>(KeyValueFunc),
                                      seq.toMultimap<int, std::string>(keyFunc, valueFunc));
  testBinaryCollection<std::unordered_multimap>("Unordered multimap",
                                                seq.toUnorderedMultimap<int, std::string>(KeyValueFunc),
                                                seq.toUnorderedMultimap<int, std::string>(keyFunc, valueFunc));
  trace();
  trace("Abstract collection #1");
  traceCollection(range(1, 5).match(range(6, 5)).toContainer<std::map<int, int>>());
  traceCollection(range(1, 5).match(range(6, 5)).toContainer<std::map<int, int>, std::pair<int, int>>(
      identity<std::pair<int, int>>));
  trace("Abstract collection #2");
  traceCollection(range(1, 5).toContainerByIndex<std::map<wide_size_t, int>, std::pair<wide_size_t, int>>(
      identity<std::pair<wide_size_t, int>>));
  traceCollection(range(1, 5)
                      .toContainerByIndex<std::map<wide_size_t, int>, wide_size_t, int>(fn1(it.first), fn1(it.second)));
}

void testReverse() {
  trace("testReverse");
  trace(range(1, 11).reverse());
  trace(range(1, 11).reverse(fn1(it / 2.)));
  trace(integerNumbers().reverse(identity<integer_t>).reverse().take(15));
  trace(integerNumbers().reverse().reverse().take(15));
  trace(LazySeq(range(1, 10).reverse()).reverse());
  trace();
  trace(naturalNumbers().reverse().filter(even<natural_t>).reverse().take(10));
  trace(range(1, 10).reverse().filter(even<natural_t>));
  auto multiplyBy2 = partial(std::multiplies<>(), 2);
  trace(naturalNumbers().reverse().map(multiplyBy2).reverse().take(10));
  trace(range(1, 10).reverse().map(multiplyBy2));
  trace(naturalNumbers().reverse().concat(range<natural_t>(1, 5)).reverse().take(10));
  trace(range(1, 10).reverse().concat(range(50, 5)));
  trace(naturalNumbers().reverse().match(integerNumbers().reverse()).reverse().take(10));
  trace(LazySeq(range(100, 10).reverse().match(integerNumbers())));
  trace(naturalNumbers().reverse().butLast().reverse().take(10));
  trace(range(1, 10).reverse().butLast());
  trace(naturalNumbers().reverse().reverse().rest().take(10));
  trace(range(1, 10).reverse().rest());
  trace(join(identitySeq(std::vector<int>{1, 2, 3}).reverse()).reverse().take(10));
  trace(join(identitySeq(std::vector<int>{1, 2, 3}).take(10).reverse()));
  trace();
}

void testMakeLazyPaired() {
  trace("testMakeLazyPaired");
  auto map = std::map<int, float>{{0, 1.1}, {1, 2.2}};
  const auto pairToString = fn1("[" + std::to_string(it.first) + ": " + std::to_string(it.second) + "]");
  trace(makeLazy(map).map(pairToString));
  trace(makeLazy(std::move(map)).map(pairToString));
  trace(makeLazy(map).map(pairToString));
}

void testFirstRestElementAt() {
  trace("testFirstRestElementAt");
  auto seq = range(1, 10);
  trace(seq.first());
  trace(seq.rest());
  const auto pred = dividesBy(3);
  trace(seq.first(pred));
  trace(seq.rest(pred));
  trace((LazySeq{3} * 5).rest(3));
  trace(range(1, 10).itemAt(5));
}

void testEmplace() {
  trace("testEmplace");
  trace(range(1, 10).emplaceFront(100));
  trace(range(1, 10).emplaceBack(100));
}