#include"nwaobdd.h"
namespace NWA_OBDD {
    typedef ConnectionT< ReturnMapHandle<int> > CFLConnection;
    struct pseudoCFLOBDDBase {
        intpair bconn[2];
        unsigned numBConnections;
        unsigned numExits;
        pseudoCFLOBDDBase();
        pseudoCFLOBDDBase(const NWAOBDDInternalNode* n);
        NWAOBDDInternalNode* toNWA();
    };

    struct pseudoCFLOBDDInternal {
        unsigned level;
        CFLConnection AConnection;
        CFLConnection* BConnection;
        unsigned numBConnections;
        unsigned numExits;
        pseudoCFLOBDDInternal(unsigned l);
        pseudoCFLOBDDInternal(const NWAOBDDInternalNode* n);
        ~pseudoCFLOBDDInternal();
        NWAOBDDInternalNode* toNWA();
    };

    ReturnMapHandle<int> cfl_return_map(unsigned a0);
    ReturnMapHandle<int> cfl_return_map(unsigned a0, unsigned a1);
    ReturnMapHandle<int> cfl_return_map(unsigned a0, unsigned a1, unsigned a2);
};