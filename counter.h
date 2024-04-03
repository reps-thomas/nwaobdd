#include<chrono>

class Counter {
    public:
        Counter();
        ~Counter();
    private: 
        std::chrono::time_point<std::chrono::steady_clock> start;
};

extern int call_cnt;
extern double total_time;