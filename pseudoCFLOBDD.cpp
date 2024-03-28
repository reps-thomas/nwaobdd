#include"pseudoCFLOBDD.h"

// Be careful to make NWAConnections canonical

static ReturnMapHandle<intpair> rmh(int i) {
    ReturnMapHandle<intpair> r;
    r.AddToEnd(intpair(i, i));
    r.Canonicalize();
    return r;
}

static ReturnMapHandle<intpair> rmh(ReturnMapHandle<int> r1) {
    ReturnMapHandle<intpair> r2;
    for(unsigned i = 0; i < r1.Size(); ++i)
        r2.AddToEnd(intpair(r1[i], r1[i]));
    r2.Canonicalize();
    return r2;
}

namespace NWA_OBDD {
    pseudoCFLOBDDBase::pseudoCFLOBDDBase() {
        numBConnections = numExits = 0;
        bconn[0] = bconn[1] = intpair(0, 0);
    }
    pseudoCFLOBDDBase::pseudoCFLOBDDBase(const NWAOBDDInternalNode* n) {
        
    }

    //  Transforming a pesudoCFLOBDDBase to a level-1 NWAOBDDInternalNode
    //  It does not contain "InstallPathCounting".
    NWAOBDDInternalNode* pseudoCFLOBDDBase::toNWA() {
        NWAOBDDInternalNode* n = new NWAOBDDInternalNode(1);
        n -> numBConnections = numBConnections;
        n -> numExits = numExits;
        n -> BConnection[0] = new Connection[numBConnections];
        n -> BConnection[1] = new Connection[numBConnections];

        NWAOBDDNodeHandle eps = NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
        if(numBConnections == 1) {
            auto r0 = rmh(0);
            auto r1 = rmh(1);
            n -> AConnection[0] = Connection(eps, r0);
            n -> AConnection[1] = Connection(eps, r0);

            auto rb00 = rmh(bconn[0].First());
            auto rb10 = rmh(bconn[0].Second());
            n -> BConnection[0][0] = Connection(eps, rb00);
            n -> BConnection[1][0] = Connection(eps, rb10);
        }
        else {
            auto r0 = rmh(0);
            auto r1 = rmh(1);
            n -> AConnection[0] = Connection(eps, r0);
            n -> AConnection[1] = Connection(eps, r1);

            auto rb00 = rmh(bconn[0].First());
            auto rb10 = rmh(bconn[0].Second());
            n -> BConnection[0][0] = Connection(eps, rb00);
            n -> BConnection[1][0] = Connection(eps, rb10);
            
            auto rb01 = rmh(bconn[1].First());
            auto rb11 = rmh(bconn[1].Second());
            n -> BConnection[0][1] = Connection(eps, rb01);
            n -> BConnection[1][1] = Connection(eps, rb11);
        }
        return n;
    }

    pseudoCFLOBDDInternal::pseudoCFLOBDDInternal(unsigned l) {
        level = l;
        numBConnections = numExits = 0;
        BConnection = nullptr;
    }
    
    pseudoCFLOBDDInternal::pseudoCFLOBDDInternal(const NWAOBDDInternalNode* n) {
    }

    //  Transforming a pesudoCFLOBDDBase to an NWAOBDDInternalNode at the same level.
    //  It does not contain "InstallPathCounting".
    NWAOBDDInternalNode* pseudoCFLOBDDInternal::toNWA() {
        NWAOBDDInternalNode* n = new NWAOBDDInternalNode(level);
        n -> numBConnections = numBConnections;
        n -> numExits = numExits;
        n -> BConnection[0] = new Connection[numBConnections];
        n -> BConnection[1] = new Connection[numBConnections];

        auto aret = rmh(AConnection.returnMapHandle);
        n -> AConnection[0] = Connection(AConnection.entryPointHandle->handleContents, aret);
        n -> AConnection[1] = n -> AConnection[0];

        for(unsigned i = 0; i < numBConnections; ++i) {
            auto bret_i = rmh(BConnection[i].returnMapHandle);
            n -> BConnection[0][i] = Connection(BConnection[i].entryPointHandle->handleContents, bret_i);
            n -> BConnection[1][i] = n -> BConnection[0][i];
        }
        return n;
    }

    pseudoCFLOBDDInternal::~pseudoCFLOBDDInternal() {
        if(BConnection)
            delete [] BConnection;
    }

    ReturnMapHandle<int> cfl_return_map(unsigned a0) {
        ReturnMapHandle<int> r;
        r.AddToEnd(a0);
        r.Canonicalize();
        return r;
    }
    ReturnMapHandle<int> cfl_return_map(unsigned a0, unsigned a1) {
        ReturnMapHandle<int> r;
        r.AddToEnd(a0);
        r.AddToEnd(a1);
        r.Canonicalize();
        return r;
    }
    ReturnMapHandle<int> cfl_return_map(unsigned a0, unsigned a1, unsigned a2) {
        ReturnMapHandle<int> r;
        r.AddToEnd(a0);
        r.AddToEnd(a1);
        r.AddToEnd(a2);
        r.Canonicalize();
        return r;
    }
};

    