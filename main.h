//
// Created by zhele on 01.07.2019.
//

#ifndef FUNCTIONAL_MAIN_H
#define FUNCTIONAL_MAIN_H

void testMakeLazyPaired();
void testReverse();
void testToCollectionFunctions();
void testMatch();

void testEqualityOperator();
void testOutputOperator();
void testLastButLast();
void testToString();

void testMinMax();
void testArithmeticOperations();
void testCount();
void testSkipsTakes();

template<class T>
void testEveryAnyNoneContains(const LazySeq<T> &lazy);
void testCartesianMultiplication();
void testRepeating();

void testRange();
void testFromInitList();
void testJoinMapManyMapIf();

template<class T>
void testConcat(const LazySeq<T> &lazy);
template<class T>
void testCast(const LazySeq<T> &even);

template<class T>
void testReduce(const LazySeq<T> &even);
LazySeq<int> testFilterRemove(const LazySeq<int> &seq);
void testMap(const LazySeq<int> &seq);

template<class T>
LazySeq<int> testNodeConstructor(const LazySeq<T> &lazy);
template<class T>
LazySeq<T> testMakeLazy(const std::vector<int> &vec);
void testRecursion();

void testFirstRestElementAt();
void testEmplace();
void testDistinctUnionExceptIntersect();
void testGroupBy();

void testOrder();
void testIndexed();
void testComparisons();

void testHash();
void testSetsOfLazySeq();
void testForeach();

void testFunctionsByIndex();
void testSimpleFunctions();
void testRandom();

void testSizesOfLazySeqs();
void testSmartSkips();

#endif //FUNCTIONAL_MAIN_H
