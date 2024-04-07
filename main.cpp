#include "testing/quantum_algos.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "nwaobdd_node.h"

int reduce_times[26], reduce_hit[26];
int matmult_times[26], matmult_hit[26];
int pairprod_times[26], pairprod_hit[26];

int node_times[26];
int node_hit[26];

int call_cnt;
void print_summary() {
    // printf("Reduce Hit Rate:\n");
    // for(unsigned i = 0; i < 25; ++i)
    //     printf("  Level %d: %d / %d\n", i, reduce_hit[i], reduce_times[i]);

    // printf("MatMult Hit Rate:\n");
    // for(unsigned i = 0; i < 25; ++i)
    //     printf("  Level %d: %d / %d\n", i, matmult_hit[i], matmult_times[i]);
    
    // printf("PairProd Hit Rate:\n");
    // for(unsigned i = 0; i < 20; ++i)
    //     printf("  Level %d: %d / %d\n", i, pairprod_hit[i], pairprod_times[i]);
    // printf("%d\n", call_cnt);

    // printf("Node Hit Rate:\n");
    // for(unsigned i = 0; i < 20; ++i)
    //     printf("  Level %d: %d / %d\n", i, node_hit[i], node_times[i]);
    // unsigned siz = 0;
    // for(unsigned i = 0; i <= 25; ++i)
    //     siz += NWA_OBDD::NWAOBDDNodeHandle::canonicalNodeTableLeveled[i].size();
    // printf("%u\n", siz);
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
    else if(strcasecmp(argv[1], "simons") == 0) simons(numQubits, seed);
    else {
        printf("Unknown Quantum-Algo: %s\n", argv[1]);
        return 0;
    }

    print_summary();
    // for(int i = 1; i <= 1000; ++i) 
    //     func();
    // printf("Called %d times, taking %.6lf time\n", call_cnt, total_time);
}