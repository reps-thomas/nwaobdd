#include"counter.h"
#include"stdio.h"

Counter::Counter(): start(std::chrono::steady_clock::now()) { call_cnt++; }

Counter::~Counter() {
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    total_time += elapsed_seconds.count();
}