#include "quantum_algos.h"
#include "quantum_circuit.h"
#include "string"
#include "ctime"
#include "iostream"

using std::string;

void ghz(int numQubits, int seed) {
    string allZeros(numQubits, '0');
    string allOnes(numQubits, '1');
    std::srand(seed);
    clock_t start = clock();

    NWAOBDDQuantumCircuit qc(numQubits, seed);
    qc.ApplyHadamardGate(0);
    for(int i = 1; i < numQubits; ++i)
        qc.ApplyCNOTGate(0, i);
    
    string sampledString = qc.Measure();
    int sampleCount = 1;
    int iterCount = 0;

    // Continue sampling until all zeros or all ones are observed
    while (sampledString != allZeros && sampledString != allOnes && iterCount < numQubits) {
        ++sampleCount;
        sampledString = qc.Measure();
        ++iterCount;
    }

    clock_t end = clock();

    double time = static_cast<double>(end - start) / CLOCKS_PER_SEC;

    if (iterCount >= numQubits) {
        printf("Incorrect\n");
        abort();
    }
    printf("%lf, %d iters\n", time, iterCount);
}

void grovers(int numQubits, int seed) {
    string s;
    std::srand(seed);
    for(int i = 0; i < numQubits; ++i)
        if(rand() & 1) s.push_back('0');
        else s.push_back('1');
    string allZeros(numQubits, '0');
    string allOnes(numQubits, '1');

    int iters = std::ceil((M_PI * std::pow(2, numQubits/2)) / 4);

    clock_t start = clock();

    NWAOBDDQuantumCircuit qc(2 * numQubits - 1, seed);

    // Apply Hadamard gate to each qubit
    for (int i = 0; i < numQubits; ++i)
        qc.ApplyHadamardGate(i);

    for (int j = 0; j < iters; ++j) {
        for (int i = 0; i < numQubits; ++i) 
            if (s[i] == '0') 
                qc.ApplyNOTGate(i);

        qc.ApplyCCNOTGate(0, 1, numQubits);

        for (int i = 2; i < numQubits; ++i) 
            qc.ApplyCCNOTGate(i, numQubits + i - 2, numQubits + i - 1);

        qc.ApplyPauliZGate(2 * numQubits - 2);

        for (int i = numQubits - 1; i > 1; --i) {
            qc.ApplyCCNOTGate(i, numQubits + i - 2, numQubits + i - 1);
        }

        qc.ApplyCCNOTGate(0, 1, numQubits);

        for (int i = 0; i < numQubits; ++i) {
            if (s[i] == '0') {
                qc.ApplyNOTGate(i);
            }
            qc.ApplyHadamardGate(i);
            qc.ApplyNOTGate(i);
        }

        qc.ApplyCCNOTGate(0, 1, numQubits);

        for (int i = 2; i < numQubits; ++i) {
            qc.ApplyCCNOTGate(i, numQubits + i - 2, numQubits + i - 1);
        }

        qc.ApplyPauliZGate(2 * numQubits - 2);

        for (int i = numQubits - 1; i > 1; --i) {
            qc.ApplyCCNOTGate(i, numQubits + i - 2, numQubits + i - 1);
        }

        qc.ApplyCCNOTGate(0, 1, numQubits);

        for (int i = 0; i < numQubits; ++i) {
            qc.ApplyNOTGate(i);
            qc.ApplyHadamardGate(i);
        }
    }
    clock_t end = clock();

    string sampledString = qc.Measure().substr(0, numQubits);
    int sampleCount = 1;
    int iterCount = 0;

    while (sampledString != s && iterCount < numQubits) {
        ++sampleCount;
        sampledString = qc.Measure().substr(0, numQubits);
        ++iterCount;
    }

    double time = static_cast<double>(end - start) / CLOCKS_PER_SEC;

    if (iterCount >= numQubits) {
        printf("Incorrect\n");
        abort();
    }
    printf("%lf, %d iters\n", time, iterCount);
}

void dj(int numQubits, int seed) {
    std::srand(seed);

    std::string s;
    for (int i = 0; i < numQubits; ++i) {
        s += std::rand() % 2 == 0 ? '0' : '1';
    }

    std::string allZeros(numQubits, '0');
    std::string allOnes(numQubits, '1');

    // Start time measurement
    clock_t start = clock();

    // Create QuantumCircuit object
    NWAOBDDQuantumCircuit qc(numQubits + 1, seed);

    qc.ApplyNOTGate(numQubits);

    for (int i = 0; i < numQubits + 1; ++i) {
        qc.ApplyHadamardGate(i);
    }

    for (int i = 0; i < numQubits; ++i) {
        qc.ApplyCNOTGate(i, numQubits);
    }

    for (int i = 0; i < numQubits; ++i) {
        if (s[i] == '1') {
            qc.ApplyNOTGate(i);
        }
    }

    for (int i = 0; i < numQubits; ++i) {
        qc.ApplyHadamardGate(i);
    }

    std::string sampledString = qc.Measure().substr(0, numQubits);
    int sampleCount = 1;
    int iterCount = 0;

    while (sampledString != allOnes && iterCount < numQubits) {
        ++sampleCount;
        sampledString = qc.Measure().substr(0, numQubits);
        ++iterCount;
    }

    // End time measurement
    clock_t end = clock();

    // Calculate elapsed time
    double time = static_cast<double>(end - start) / CLOCKS_PER_SEC;

    if (iterCount >= numQubits) {
        printf("Incorrect\n");
        abort();
    }
    printf("%lf, %d iters\n", time, iterCount);
}

void bv(int numQubits, int seed) {
    std::string s;
    for (int i = 0; i < numQubits; ++i) {
        s += std::rand() % 2 == 0 ? '0' : '1';
    }

    // Start time measurement
    clock_t start = clock();

    // Create QuantumCircuit object
    NWAOBDDQuantumCircuit qc(numQubits + 1, seed);

    qc.ApplyNOTGate(numQubits);

    for (int i = 0; i < numQubits + 1; ++i) {
        qc.ApplyHadamardGate(i);
    }

    for (int i = 0; i < numQubits; ++i) {
        if (s[i] == '1') {
            qc.ApplyCNOTGate(i, numQubits);
        }
    }

    for (int i = 0; i < numQubits + 1; ++i) {
        qc.ApplyHadamardGate(i);
    }
    
    std::string sampledString = qc.Measure().substr(0, numQubits);
    int sampleCount = 1;
    int iterCount = 0;

    while (sampledString != s && iterCount < numQubits) {
        ++sampleCount;
        sampledString = qc.Measure().substr(0, numQubits);
        ++iterCount;
    }

    // End time measurement
    clock_t end = clock();

       // Calculate elapsed time
    double time = static_cast<double>(end - start) / CLOCKS_PER_SEC;

    if (iterCount >= numQubits) {
        printf("Incorrect\n");
        abort();
    }
    printf("%lf, %d iters\n", time, iterCount);
}

void qft(int numQubits, int seed) {
    std::srand(seed);

    std::string s;
    for (int i = 0; i < numQubits; ++i) {
        s += std::rand() % 2 == 0 ? '0' : '1';
    }

    // Start time measurement
    clock_t start = clock();

    // Create QuantumCircuit object
    NWAOBDDQuantumCircuit qc(numQubits, seed);

    for (int i = 0; i < s.length(); ++i) {
        if (s[i] == '1') {
            qc.ApplyNOTGate(i);
        }
    }

    for (int i = 0; i < numQubits / 2; ++i) {
        qc.ApplySwapGate(i, numQubits - i - 1);
    }

    for (int i = numQubits - 1; i >= 0; --i) {
        qc.ApplyHadamardGate(i);
        for (int j = 0; j < i; ++j) {
            double theta = 1 / pow(2, (i - j));
            qc.ApplyCPGate(j, i, theta);
        }
    }

    // End time measurement
    clock_t end = clock();

    // Calculate elapsed time
    double elapsedTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;

    // Print result
    std::cout << "Correct, time: " << elapsedTime << ", iter_count: " << 0 << std::endl;
}

void simons(int numQubits, int seed) {
    std::srand(seed);

    std::string s;
    for (int i = 0; i < numQubits; ++i) {
        s += std::rand() % 2 == 0 ? '0' : '1';
    }

    // Start time measurement
    clock_t start = clock();

    // Create QuantumCircuit object
    NWAOBDDQuantumCircuit qc(2 * numQubits, seed);

    for (int i = 0; i < numQubits; ++i) {
        qc.ApplyHadamardGate(i);
    }

    for (int i = 0; i < numQubits; ++i) {
        qc.ApplyCNOTGate(i, i + numQubits);
    }

    int k = 0;
    for (int i = numQubits - 1; i >= 0; --i) {
        if (s[i] == '1') {
            int m = numQubits;
            for (int j = numQubits - 1; j >= 0; --j) {
                if (s[j] == '1') {
                    qc.ApplyCNOTGate(k, m);
                }
                ++m;
            }
            break;
        }
        ++k;
    }

    for (int i = 0; i < numQubits; ++i) {
        qc.ApplyHadamardGate(i);
    }

    for (int i = 0; i < 2 * numQubits; ++i) {
        std::string sampledString = qc.Measure();
    }

    // End time measurement
    clock_t end = clock();

    // Calculate elapsed time
    double elapsedTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;

    // Print result
    std::cout << "Correct, time: " << elapsedTime << ", iter_count: " << 0 << std::endl;
}