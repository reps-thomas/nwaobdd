#include "testing/quantum_algos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "counter.h"

int call_cnt;
double total_time;


void func() {
    Counter a;
    
}
int main(int argc, char *argv[]) {
    if(argc < 3 || argc > 4) {
        printf("Usage: nwaobdd.out <Quantum-Algo> <Num-Qubits> [Random-Seed]\n");
        return 0;
    }
    int numQubits, seed;
    numQubits = atoi(argv[2]);
    if(argc == 4) seed = atoi(argv[3]);
    else seed = 1;
    if(strcasecmp(argv[1], "ghz") == 0) ghz(numQubits, seed);
    else if(strcasecmp(argv[1], "grovers") == 0) grovers(numQubits, seed);
    else if(strcasecmp(argv[1], "bv") == 0) bv(numQubits, seed);
    else if(strcasecmp(argv[1], "dj") == 0) dj(numQubits, seed);
    else if(strcasecmp(argv[1], "qft") == 0) qft(numQubits, seed);
    else if(strcasecmp(argv[1], "bv") == 0) simons(numQubits, seed);
    else {
        printf("Unknown Quantum-Algo: %s\n", argv[1]);
        return 0;
    }
    for(int i = 1; i <= 1000; ++i) 
        func();
    printf("Called %d times, taking %.6lf time\n", call_cnt, total_time);
}