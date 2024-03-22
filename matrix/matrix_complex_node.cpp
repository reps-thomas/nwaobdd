#include "matrix_complex_node.h"

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
    NWAOBDDNodeHandle MkIdRelationInterleavedNode(unsigned int level) {
        assert(level > 0);
        NWAOBDDInternalNode *n;
        n = new NWAOBDDInternalNode(level);
        n -> numBConnections = n -> numExits = 2;
        n -> BConnection[0] = new Connection[2];
        n -> BConnection[1] = new Connection[2];
        if(level == 1) {
            NWAOBDDNodeHandle eps = NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
            n -> AConnection[0] = Connection(eps, commonly_used_return_maps[1]); // 01
            n -> AConnection[1] = Connection(eps, commonly_used_return_maps[2]); // 10

            n -> BConnection[0][0] = Connection(eps, commonly_used_return_maps[1]); // 01
            n -> BConnection[1][0] = Connection(eps, commonly_used_return_maps[2]); // 10
            n -> BConnection[0][1] = Connection(eps, commonly_used_return_maps[3]); // 11
            n -> BConnection[1][1] = Connection(eps, commonly_used_return_maps[3]); // 11
        }
        else {
            NWAOBDDNodeHandle rec = MkIdRelationInterleavedNode(level - 1);
            NWAOBDDNodeHandle no_dist = NWAOBDDNodeHandle::NoDistinctionNode[level - 1];

            ReturnMapHandle<intpair> r0011;
            r0011.AddToEnd(intpair(0, 0));
            r0011.AddToEnd(intpair(1, 1));
            r0011.Canonicalize();

            n -> AConnection[0] = Connection(rec, r0011);
            n -> AConnection[1] = n -> AConnection[0];

            n -> BConnection[0][0] = Connection(rec, r0011); // 0011
            n -> BConnection[1][0] = n -> BConnection[0][0];
            n -> BConnection[0][1] = Connection(no_dist, commonly_used_return_maps[3]); // 11
            n -> BConnection[1][1] = n -> BConnection[0][1];
        }

#ifdef PATH_COUNTING_ENABLED
		n->InstallPathCounts();
#endif
        NWAOBDDNodeHandle handle(n);
        return handle;
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
}  // namespace NWA_OBDD
