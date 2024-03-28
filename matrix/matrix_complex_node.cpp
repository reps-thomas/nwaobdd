#include "matrix_complex_node.h"
#include "../pseudoCFLOBDD.h"

namespace NWA_OBDD {
    extern std::vector<ReturnMapHandle<intpair>>commonly_used_return_maps;
    void Matrix1234InitializerNode() {
        return;

        ReturnMapHandle<intpair> m00, m11, m01, m10;

        m00.AddToEnd(intpair(0, 0));
        m00.Canonicalize();
        commonly_used_return_maps.push_back(m00);

        m01.AddToEnd(intpair(0, 1));
        m01.Canonicalize();
        commonly_used_return_maps.push_back(m01);

        m10.AddToEnd(intpair(1, 0));
        m10.Canonicalize();
        commonly_used_return_maps.push_back(m10);

        m11.AddToEnd(intpair(1, 1));
        m11.Canonicalize();
        commonly_used_return_maps.push_back(m11);
    }
    NWAOBDDNodeHandle MatrixShiftToAConnectionNode(NWAOBDDNodeHandle c) {
        NWAOBDDInternalNode* n = new NWAOBDDInternalNode(c.handleContents -> level + 1);
        ReturnMapHandle<intpair> m;
        for(unsigned i = 0; i < c.handleContents->numExits; ++i)
            m.AddToEnd(intpair(i, i));
        m.Canonicalize();
        n -> AConnection[0] = Connection(c, m);
        n -> AConnection[1] = n -> AConnection[0];

        int num_mid = c.handleContents->numExits;
        n -> numBConnections = num_mid;
        n -> BConnection[0] = new Connection[num_mid];
        n -> BConnection[1] = new Connection[num_mid];

        for(unsigned i = 0; i < num_mid; ++i) {
            ReturnMapHandle<intpair> mi;
            mi.AddToEnd(intpair(i, i));
            mi.Canonicalize();
            n->BConnection[0][i] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[c.handleContents -> level], mi);
            n->BConnection[1][i] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[c.handleContents -> level], mi);
        }
        n -> numExits = num_mid;
        n->InstallPathCounts();
        return NWAOBDDNodeHandle(n);
    }
	NWAOBDDNodeHandle MatrixShiftToBConnectionNode(NWAOBDDNodeHandle c) {
        NWAOBDDInternalNode* n = new NWAOBDDInternalNode(c.handleContents -> level + 1);
        n -> AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[c.handleContents->level], commonly_used_return_maps[0]);
        n -> AConnection[1] = n -> AConnection[0];
        n -> numBConnections = 1;

        n -> BConnection[0] = new Connection[1];
        n -> BConnection[1] = new Connection[1];
        ReturnMapHandle<intpair> m;
        for(unsigned i = 0; i < c.handleContents->numExits; ++i) 
            m.AddToEnd(intpair(i, i));
        m.Canonicalize();
        n -> BConnection[0][0] = Connection(c, m);
        n -> BConnection[1][0] = n -> BConnection[0][0];

        n -> numExits = c.handleContents->numExits;
        n->InstallPathCounts();
        return NWAOBDDNodeHandle(n);
    }
    NWAOBDDNodeHandle MatrixShiftLevel0Node(NWAOBDDNodeHandle c) {
        NWAOBDDInternalNode* n = new NWAOBDDInternalNode(1);
        NWAOBDDInternalNode* nc = dynamic_cast<NWAOBDDInternalNode*>(c.handleContents);

        n -> numBConnections = 1;
        n -> numExits = nc -> numExits;
        n -> BConnection[0] = new Connection [1];
        n -> BConnection[1] = new Connection [1];

        n -> AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[0], commonly_used_return_maps[0]);
        n -> AConnection[1] = n -> AConnection[0];

        n -> BConnection[0][0] = nc -> AConnection[0];
        n -> BConnection[1][0] = nc -> AConnection[1];

        n -> InstallPathCounts();
        return NWAOBDDNodeHandle(n);
    }
}


namespace NWA_OBDD {
    NWAOBDDNodeHandle MkIdNode(unsigned int level) {
        NWAOBDDInternalNode *n;
        assert(level > 0);
        if(level == 1) {
            pseudoCFLOBDDBase c;
            c.numBConnections = c.numExits = 2;
            c.bconn[0] = intpair(0, 1);
            c.bconn[1] = intpair(1, 0);
            n = c.toNWA();
        }
        else {
            NWAOBDDNodeHandle rec = MkIdNode(level - 1);
            NWAOBDDNodeHandle no_dist = NWAOBDDNodeHandle::NoDistinctionNode[level - 1];

            auto r01 = cfl_return_map(0, 1);
            auto r1 = cfl_return_map(1);

            pseudoCFLOBDDInternal c(level);
            c.numBConnections = c.numExits = 2;
            c.BConnection = new CFLConnection [2];
            
            c.AConnection = CFLConnection(rec, r01);
            c.BConnection[0] = CFLConnection(rec, r01);
            c.BConnection[1] = CFLConnection(no_dist, r1);
            n = c.toNWA();
        }

#ifdef PATH_COUNTING_ENABLED
		n->InstallPathCounts();
#endif
        NWAOBDDNodeHandle handle(n);
        return handle;
    }

    NWAOBDDNodeHandle MkNegationNode(unsigned int level) {
        NWAOBDDInternalNode *n;
        assert(level > 0);
        if(level == 1) {
            pseudoCFLOBDDBase c;
            c.numBConnections = c.numExits = 2;
            c.bconn[0] = intpair(0, 1);
            c.bconn[1] = intpair(1, 0);
            n = c.toNWA();
        }
        else {
            NWAOBDDNodeHandle rec = MkNegationNode(level - 1);
            NWAOBDDNodeHandle no_dist = NWAOBDDNodeHandle::NoDistinctionNode[level - 1];

            auto r01 = cfl_return_map(0, 1);
            auto r0 = cfl_return_map(0);

            pseudoCFLOBDDInternal c(level);
            c.numBConnections = c.numExits = 2;
            c.BConnection = new CFLConnection [2];

            c.AConnection = CFLConnection(rec, r01);
            c.BConnection[0] = CFLConnection(no_dist, r0);
            c.BConnection[1] = CFLConnection(rec, r01);
            n = c.toNWA();
        }
        
        n -> InstallPathCounts();
        return NWAOBDDNodeHandle(n);
    }
    NWAOBDDNodeHandle MkWalshNode(unsigned int level) {
        NWAOBDDInternalNode *n;
        assert(level == 1);
        pseudoCFLOBDDBase c;
        c.numBConnections = 2;
        c.numExits = 2;
        c.bconn[0] = intpair(0, 0);
        c.bconn[1] = intpair(0, 1);
        n = c.toNWA();

        n -> InstallPathCounts();
        return NWAOBDDNodeHandle(n);
    }
    NWAOBDDNodeHandle MkPauliYNode(unsigned int level) {
        NWAOBDDInternalNode *n;
        assert(level == 1);
        pseudoCFLOBDDBase c;
        c.numBConnections = 2;
        c.numExits = 3;
        c.bconn[0] = intpair(0, 1);
        c.bconn[1] = intpair(2, 0);
        n = c.toNWA();

        n -> InstallPathCounts();
        return NWAOBDDNodeHandle(n);
    }
    NWAOBDDNodeHandle MkPauliZNode(unsigned int level) {
        NWAOBDDInternalNode *n;
        assert(level == 1);
        pseudoCFLOBDDBase c;
        c.numBConnections = 2;
        c.numExits = 3;
        c.bconn[0] = intpair(0, 1);
        c.bconn[1] = intpair(1, 2);
        n = c.toNWA();

        n -> InstallPathCounts();
        return NWAOBDDNodeHandle(n);
    }
    NWAOBDDNodeHandle MkSNode(unsigned int level) {
        NWAOBDDInternalNode *n;
        assert(level == 1);

        pseudoCFLOBDDBase c;
        c.numBConnections = 2;
        c.numExits = 3;
        c.bconn[0] = intpair(0, 1);
        c.bconn[1] = intpair(1, 2);
        n = c.toNWA();
        n -> InstallPathCounts();
        return NWAOBDDNodeHandle(n);
    }

}  // namespace NWA_OBDD

namespace NWA_OBDD {

    std::unordered_map<MatMultPair, NWAOBDDTopNodeMatMultMapRefPtr, MatMultPair::MatMultPairHash> matmult_hashMap;
	std::unordered_map<MatMultPairWithInfo, NWAOBDDTopNodeMatMultMapRefPtr, MatMultPairWithInfo::MatMultPairWithInfoHash> matmult_hashMap_info;
	std::unordered_map<std::string, NWAOBDDNodeHandle> cnot_hashMap;
	std::unordered_map<std::string, NWAOBDDNodeHandle> cp_hashMap;
	std::unordered_map<std::string, NWAOBDDNodeHandle> ccp_hashMap;
	std::unordered_map<std::string, NWAOBDDNodeHandle> cswap_hashMap;
	std::unordered_map<std::string, NWAOBDDNodeHandle> swap_hashMap;
	std::unordered_map<std::string, NWAOBDDNodeHandle> iswap_hashMap;
	std::unordered_map<std::string, NWAOBDDNodeHandle> ccnot_hashMap;

    NWAOBDDTopNodeMatMultMapRefPtr MatrixMultiplyNode(
    std::unordered_map<ZeroValNodeInfo, ZeroIndicesMapHandle, ZeroValNodeInfo::ZeroValNodeInfoHash>& hashMap,
    NWAOBDDNodeHandle c1, NWAOBDDNodeHandle c2, int c1_zero_index, int c2_zero_index) {

    }

    void clearMultMap(){
    // std::cout << "mapSize: " << matmult_hashMap.size() << std::endl;
    matmult_hashMap.clear();
	}
} 