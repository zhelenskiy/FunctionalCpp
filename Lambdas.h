//
// Created by zhele on 25.10.2019.
//

#ifndef FUNCTIONAL_LAMBDAS_H
#define FUNCTIONAL_LAMBDAS_H

#define fn0(expr) [=] { return expr; }
#define fn1(expr) [=] (const auto& it) { return expr; }
#define fn1_copy(expr) [=] (auto it) { return expr; }
#define fn1_lval(expr) [=] (auto& it) { return expr; }
#define fn2(expr) [=] (const auto& first, const auto& second) { return expr; }
#define fn2_copy(expr) [=] (auto first, auto second) { return expr; }
#define fn2_lval(expr) [=] (auto& first, auto& second) { return expr; }

#endif //FUNCTIONAL_LAMBDAS_H
