#include "quantum_circuit.h"
#include <random>

QuantumCircuit::QuantumCircuit(unsigned int numQubits, int seed) :  numQubits (numQubits) 
{
    mt.seed(seed);
    srand(seed);
    hadamard_count = 0;
}
QuantumCircuit::QuantumCircuit() :  numQubits (0), hadamard_count (0) {}
QuantumCircuit::~QuantumCircuit() {}


using namespace NWA_OBDD;
NWAOBDDQuantumCircuit::NWAOBDDQuantumCircuit(unsigned int numQubits, int seed) : QuantumCircuit(numQubits, seed)
{
    // printf("NWA Constructor: %d %d\n", numQubits, seed);
    // Initialize
        NWAOBDDNodeHandle::InitNoDistinctionTable();
        // NWAOBDDNodeHandle::InitAdditionInterleavedTable();
        NWAOBDDNodeHandle::InitReduceCache();
        NWA_OBDD::InitPairProductCache();
        NWA_OBDD::InitPathSummaryCache();
        MatrixComplex::Matrix1234Initializer();
        VectorComplex::VectorInitializer();
    //
    unsigned int level = ceil(log2(numQubits)) + 1;
    stateVector = VectorComplex::MkBasisVector(level, 0);
}

NWAOBDDQuantumCircuit::NWAOBDDQuantumCircuit()
{
    // printf("NWA Constructor:\n");
    // Initialize
        NWAOBDDNodeHandle::InitNoDistinctionTable();
        NWAOBDDNodeHandle::InitReduceCache();
        NWA_OBDD::InitPathSummaryCache();
        NWA_OBDD::InitPairProductCache();
        MatrixComplex::Matrix1234Initializer();
        VectorComplex::VectorInitializer();
        numQubits = 0;
    //
}

NWAOBDDQuantumCircuit::~NWAOBDDQuantumCircuit()
{
	NWA_OBDD::DisposeOfPairProductCache();
	NWAOBDDNodeHandle::DisposeOfReduceCache();
}

void NWAOBDDQuantumCircuit::setNumQubits(unsigned int num)
{
    // printf("NWA setNumQubits: %d\n", num);
    numQubits = num;
    unsigned int level = ceil(log2(numQubits)) + 1;
    stateVector = VectorComplex::MkBasisVector(level, 0); 
}

NWAOBDD_COMPLEX_BIG ApplyGateF(unsigned int n, unsigned int i, NWAOBDD_COMPLEX_BIG(*f)(unsigned int))
{
    if (n == 1)
    {
        return f(1);
    }
    else {
        int level = ceil(log2(n/2));
        if (i < n/2)
        {
            NWAOBDD_COMPLEX_BIG T = MatrixComplex::MkId(level + 1);
            NWAOBDD_COMPLEX_BIG H = ApplyGateF(n/2, i, f);
            return MatrixComplex::KroneckerProduct(H, T);
        }
        else
        {
            NWAOBDD_COMPLEX_BIG T = MatrixComplex::MkId(level + 1);
            return MatrixComplex::KroneckerProduct(T, ApplyGateF(n/2, i - n/2, f)); 
        }
    }
}

NWAOBDD_COMPLEX_BIG ApplyGateFWithParam(unsigned int n, unsigned int i, NWAOBDD_COMPLEX_BIG(*f)(unsigned int, double), double theta)
{
    if (n == 1)
    {
        return f(1, theta);
    }
    else {
        int level = ceil(log2(n/2));
        if (i < n/2)
        {
            NWAOBDD_COMPLEX_BIG T = MatrixComplex::MkId(level + 1);
            NWAOBDD_COMPLEX_BIG H = ApplyGateFWithParam(n/2, i, f, theta);
            return MatrixComplex::KroneckerProduct(H, T);
        }
        else
        {
            NWAOBDD_COMPLEX_BIG T = MatrixComplex::MkId(level + 1);
            return MatrixComplex::KroneckerProduct(T, ApplyGateFWithParam(n/2, i - n/2, f, theta)); 
        }
    }
}

static bool checkForInit(unsigned int numQubits)
{
    return numQubits != 0;
}

void NWAOBDDQuantumCircuit::ApplyIdentityGate(unsigned int index)
{
    // printf("NWA ApplyIdentityGate: %d\n", index);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    auto H = ApplyGateF(std::pow(2, stateVector.root->level-1), index, MatrixComplex::MkId);
    stateVector = MatrixComplex::MatrixMultiply(H, stateVector);
}

void NWAOBDDQuantumCircuit::ApplyHadamardGate(unsigned int index)
{
    // printf("NWA ApplyHadamardGate: %d\n", index);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    auto H = ApplyGateF(std::pow(2, stateVector.root->level-1), index, MatrixComplex::MkWalsh);
    stateVector = MatrixComplex::MatrixMultiply(H, stateVector);
    hadamard_count++;
}

void NWAOBDDQuantumCircuit::ApplyNOTGate(unsigned int index)
{
    // printf("NWA ApplyNOTGate: %d\n", index);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    auto X = ApplyGateF(std::pow(2, stateVector.root->level-1), index, MatrixComplex::MkNegation);
    stateVector = MatrixComplex::MatrixMultiply(X, stateVector);
}

void NWAOBDDQuantumCircuit::ApplyPauliYGate(unsigned int index)
{
    // printf("NWA ApplyPauliYGate: %d\n", index);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    auto Y = ApplyGateF(std::pow(2, stateVector.root->level-1), index, MatrixComplex::MkPauliY);
    stateVector = MatrixComplex::MatrixMultiply(Y, stateVector);
}

void NWAOBDDQuantumCircuit::ApplyPauliZGate(unsigned int index)
{
    // printf("NWA ApplyPauliZGate: %d\n", index);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    auto Z = ApplyGateF(std::pow(2, stateVector.root->level-1), index, MatrixComplex::MkPauliZ);
    stateVector = MatrixComplex::MatrixMultiply(Z, stateVector);
}

void NWAOBDDQuantumCircuit::ApplySGate(unsigned int index)
{
    // printf("NWA ApplySGate: %d\n", index);
    auto S = ApplyGateF(std::pow(2, stateVector.root->level-1), index, MatrixComplex::MkS);
    stateVector = MatrixComplex::MatrixMultiply(S, stateVector);
}

void NWAOBDDQuantumCircuit::ApplyCNOTGate(long int controller, long int controlled)
{
    // printf("NWA ApplyCNOTGate: %lld %lld\n", controller, controlled);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    assert(controller != controlled);
    
    if (controller < controlled)
    {
        auto C = MatrixComplex::MkCNot(stateVector.root->level, std::pow(2, stateVector.root->level - 1), controller, controlled);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else
    {
        auto S = MatrixComplex::MkSwap(stateVector.root->level, controlled, controller);
        auto C = MatrixComplex::MkCNot(stateVector.root->level, std::pow(2, stateVector.root->level - 1), controlled, controller);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
}

void NWAOBDDQuantumCircuit::ApplySwapGate(long int index1, long int index2)
{
    // printf("NWA ApplySwapGate: %lld %lld\n", index1, index2);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    assert(index1 != index2);
    
    if (index1 < index2)
    {
        auto C = MatrixComplex::MkSwap(stateVector.root->level, index1, index2);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else
    {
        auto C = MatrixComplex::MkSwap(stateVector.root->level, index2, index1);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
}

void NWAOBDDQuantumCircuit::ApplyiSwapGate(long int index1, long int index2)
{
    // printf("NWA ApplyiSwapGate: %lld %lld\n", index1, index2);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    assert(index1 != index2);
    
    if (index1 < index2)
    {
        auto C = MatrixComplex::MkiSwap(stateVector.root->level, index1, index2);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else
    {
        auto C = MatrixComplex::MkiSwap(stateVector.root->level, index2, index1);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
}

void NWAOBDDQuantumCircuit::ApplyCZGate(long int controller, long int controlled)
{
    // printf("NWA ApplyCZGate: %lld %lld\n", controller, controlled);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    assert(controller != controlled);
    
    if (controller < controlled)
    {
        auto C = MatrixComplex::MkCP(stateVector.root->level, controller, controlled, 1.0);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else
    {
        auto S = MatrixComplex::MkSwap(stateVector.root->level, controlled, controller);
        auto C = MatrixComplex::MkCP(stateVector.root->level, controlled, controller, 1.0);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
}

void NWAOBDDQuantumCircuit::ApplyCPGate(long int controller, long int controlled, double theta)
{
    // printf("NWA ApplyCPGate: %lld %lld %lf\n", controller, controlled, theta);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    assert(controller != controlled);
    
    if (controller < controlled)
    {
        auto C = MatrixComplex::MkCP(stateVector.root->level, controller, controlled, theta);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else
    {
        auto S = MatrixComplex::MkSwap(stateVector.root->level, controlled, controller);
        auto C = MatrixComplex::MkCP(stateVector.root->level, controlled, controller, theta);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    } 
}

void NWAOBDDQuantumCircuit::ApplyCSGate(long int controller, long int controlled)
{
    // printf("NWA ApplyCSGate: %lld %lld\n", controller, controlled);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    ApplyCPGate(controller, controlled, 0.5);
}

void NWAOBDDQuantumCircuit::ApplyPhaseShiftGate(unsigned int index, double theta)
{
    // printf("NWA ApplyPhaseShiftGate: %d %lf\n", index, theta);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    auto S = ApplyGateFWithParam(std::pow(2, stateVector.root->level-1), index, MatrixComplex::MkPhaseShift, theta);
    stateVector = MatrixComplex::MatrixMultiply(S, stateVector); 
}

void NWAOBDDQuantumCircuit::ApplyTGate(unsigned int index)
{
    // printf("NWA ApplyTGate: %d\n", index);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    auto S = ApplyGateFWithParam(std::pow(2, stateVector.root->level-1), index, MatrixComplex::MkPhaseShift, 0.25);
    stateVector = MatrixComplex::MatrixMultiply(S, stateVector); 
}

void NWAOBDDQuantumCircuit::ApplyCCNOTGate(long int controller1, long int controller2, long int controlled)
{
    // printf("NWA ApplyCCNOTGate: %lld %lld %lld\n", controller1, controller2, controlled);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    assert(controller1 != controlled);
    assert(controller2 != controlled);
    assert(controller1 != controller2);
    if (controller1 < controller2 && controller2 < controlled)
    {
        // a b c
        auto C = MatrixComplex::MkCCNot(stateVector.root->level, std::pow(2, stateVector.root->level - 1), controller1, controller2, controlled);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else if (controller1 < controlled && controlled < controller2)
    {
        // a c b   
        auto S = MatrixComplex::MkSwap(stateVector.root->level, controlled, controller2);
        auto C = MatrixComplex::MkCCNot(stateVector.root->level, std::pow(2, stateVector.root->level - 1), controller1, controlled, controller2);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else if (controller2 < controller1 && controller1 < controlled)
    {
        // b a c
        ApplyCCNOTGate(controller2, controller1, controlled);
    }
    else if (controller2 < controlled && controlled < controller1)
    {
        // b c a
        auto S = MatrixComplex::MkSwap(stateVector.root->level, controlled, controller1);
        auto C = MatrixComplex::MkCCNot(stateVector.root->level, std::pow(2, stateVector.root->level - 1), controller2, controlled, controller1);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector); 
    }
    else if (controlled < controller1 && controller1 < controller2)
    {
        // c a b
        auto S = MatrixComplex::MkSwap(stateVector.root->level, controlled, controller2);
        // b a c
        auto C = MatrixComplex::MkCCNot(stateVector.root->level, std::pow(2, stateVector.root->level - 1), controlled, controller1, controller2);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else if (controlled < controller2 && controller2 < controller1)
    {
        // c b a
        auto S = MatrixComplex::MkSwap(stateVector.root->level, controlled, controller1);
        // a b c
        auto C = MatrixComplex::MkCCNot(stateVector.root->level, std::pow(2, stateVector.root->level - 1), controlled, controller2, controller1);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
}

void NWAOBDDQuantumCircuit::ApplyCSwapGate(long int controller, long int index1, long int index2)
{
    // printf("NWA ApplyCSwapGate: %lld %lld %lld\n", controller, index1, index2);
    if (checkForInit(numQubits) == false)
    {
        std::cout << "Number of Qubits is unset" << std::endl;
        abort();   
    }
    assert(controller != index1);
    assert(controller != index2);
    assert(index1 != index2);
    
    if (controller < index1 && index1 < index2)
    {
        // a b c
        auto C = MatrixComplex::MkCSwap(stateVector.root->level, controller, index1, index2);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else if (controller < index2 && index2 < index1)
    {
        // a c b   
        auto C = MatrixComplex::MkCSwap(stateVector.root->level, controller, index2, index1);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else if (index1 < controller && controller < index2)
    {
        // b a c
        auto S = MatrixComplex::MkSwap(stateVector.root->level, index1, controller);
        auto C = MatrixComplex::MkCSwap(stateVector.root->level, index1, controller, index2);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else if (index1 < index2 && index2 < controller)
    {
        // b c a
        auto S = MatrixComplex::MkSwap(stateVector.root->level, index1, controller);
        auto C = MatrixComplex::MkCSwap(stateVector.root->level, index1, index2, controller);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else if (index2 < controller && controller < index1)
    {
        // c a b
        auto S = MatrixComplex::MkSwap(stateVector.root->level, index2, controller);
        // b a c
        auto C = MatrixComplex::MkCSwap(stateVector.root->level, index2, controller, index1);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
    else if (index2 < index1 && index1 < controller)
    {
        // c b a
        auto S = MatrixComplex::MkSwap(stateVector.root->level, index2, controller);
        // a b c
        auto C = MatrixComplex::MkCSwap(stateVector.root->level, index2, index1, controller);
        C = MatrixComplex::MatrixMultiply(C, S);
        C = MatrixComplex::MatrixMultiply(S, C);
        stateVector = MatrixComplex::MatrixMultiply(C, stateVector);
    }
}

void NWAOBDDQuantumCircuit::ApplyGlobalPhase(double phase)
{
    // printf("Not Implemented: SchalarProd\n");
    abort();
    // if (checkForInit(numQubits) == false)
    // {
    //     std::cout << "Number of Qubits is unset" << std::endl;
    //     abort();   
    // }
    // auto cos_v = boost::math::cos_pi(phase);
    // auto sin_v = boost::math::sin_pi(phase);
    // BIG_COMPLEX_FLOAT phase_complex(cos_v, sin_v);
    // stateVector = phase_complex * stateVector;
}

long double NWAOBDDQuantumCircuit::GetProbability(std::map<unsigned int, int>& qubit_vals)
{
    // printf("Not Implemented: MkRestrict\n");
    abort();
    
    // auto tmp = VectorComplex::VectorWithAmplitude(stateVector);
    // std::string s(std::pow(2, tmp.root->level-1), 'X');
    // for (unsigned int i = 0; i < numQubits; i++)
    // {
    //     if (qubit_vals.find(i) != qubit_vals.end())
    //     {
    //         if (qubit_vals[i] == 0)
    //             s[i] = '0';
    //         else if (qubit_vals[i] == 1)
    //             s[i] = '1';   
    //     }
    // }
    // auto restricted = MatrixComplex::MkRestrictMatrix(tmp.root->level, s);
    // tmp = tmp * restricted;
    // tmp.CountPaths();
    // return VectorComplex::getNonZeroProbability(tmp);
}

std::string NWAOBDDQuantumCircuit::Measure() 
{
    auto tmp = VectorComplex::VectorWithAmplitude(stateVector);
    // tmp.CountPaths();
    return VectorComplex::Sampling(tmp, true).substr(0, numQubits); 
}

unsigned long long int NWAOBDDQuantumCircuit::GetPathCount(long double prob)
{
    auto tmp = VectorComplex::VectorWithAmplitude(stateVector);
    // tmp.CountPaths();
    return VectorComplex::GetPathCount(tmp, prob);  
}
