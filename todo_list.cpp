Constructors and Destructors:
    InitNoDistinctionTable(); // done 
    InitAdditionInterleavedTable(); // unnecessary
    InitReduceCache(); // done
    InitPairProductCache(); // done
    InitTripleProductCache(); // done

    MatrixInit(); // done
    VectorInit(); // done

    Vector::MkBasisVector();

    DisposePairProduct();
    DisposeTripleProduct();

GateF:
GateFWithParam:
    Matrix::MkIdRelationInterleaved();
    Matrix::KroneckerProduct2Vocs();


IdentityGate:
    Matrix::MatrixMultiplyV4WithInfo();

HadamardGate:
    Matrix::MkWalshInterleaved();

NotGate:
    Matrix::MkNegationMatrixInterleaved();

PauliGate:
    Matrix::MkPauliYMatrixInterleaved();
    Matrix::MkPauliZMatrixInterleaved();

SGate:
    Matrix::MkSGateInterleaved();

SwapGate:
    Matrix::MkSwap();

CNotGate:
    Matrix::MkCNOT();
    
CZGate:
    Matrix::MkCPGate();

PhaseShiftGate:
    MkPhaseShiftGateInterleaved();

CCNotGate:
    MkCCNot();

CSwapGate:
    MkCSwap();

ApplyGlobalPhase();
    schalar-product of NWAOBDDs (operator *)

GetProbability():
    Vector::VectorWithAmplitude();
    MkRestrictMatrix();
    Vector::GetNonZeroProbability();


Measure():
    Vector::Sampling();

GetPathCount():
    Vector::GetPathCount();