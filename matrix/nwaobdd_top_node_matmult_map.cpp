#include"nwaobdd_top_node_matmult_map.h"


namespace NWA_OBDD {

    MatMultPair::MatMultPair(NWAOBDDNodeHandle p1, NWAOBDDNodeHandle p2) {
        m1 = p1;
        m2 = p2;
    }

    size_t MatMultPair::MatMultPairHash::operator()(const MatMultPair& p) const {
        NWAOBDDNodeHandle t1 = p.m1;
        NWAOBDDNodeHandle t2 = p.m2;
        auto hash1 = t1.Hash(997);
        auto hash2 = t2.Hash(997);
        return 117 * (hash1 + 1) + hash2;
    }

    bool MatMultPair::operator==(const MatMultPair& p) const {
        NWAOBDDNodeHandle m11 = m1;
        NWAOBDDNodeHandle m12 = m2;
        NWAOBDDNodeHandle m21 = p.m1;
        NWAOBDDNodeHandle m22 = p.m2;
        return (m11 == m21) && (m12 == m22);
    }



    MatMultPairWithInfo::MatMultPairWithInfo(NWAOBDDNodeHandle p1, NWAOBDDNodeHandle p2, int c1, int c2)
    {
        m1 = p1;
        m2 = p2;
        c1_index = c1;
        c2_index = c2;
    }

    size_t MatMultPairWithInfo::MatMultPairWithInfoHash::operator()(const MatMultPairWithInfo& p) const
    {
        NWAOBDDNodeHandle t1 = p.m1;
        NWAOBDDNodeHandle t2 = p.m2;
        auto hash1 = t1.Hash(997);
        auto hash2 = t2.Hash(997);
        return 117 * (hash1 + 1) + 97 * 97 * hash2 + (size_t)(97 * p.c1_index + p.c2_index);
    }

    bool MatMultPairWithInfo::operator==(const MatMultPairWithInfo& p) const
    {
        NWAOBDDNodeHandle m11 = m1;
        NWAOBDDNodeHandle m12 = m2;
        NWAOBDDNodeHandle m21 = p.m1;
        NWAOBDDNodeHandle m22 = p.m2;
        return (m11 == m21) && (m12 == m22) && (c1_index == p.c1_index) && (c2_index == p.c2_index);
    }


    MatMultAddPair::MatMultAddPair(NWAOBDDTopNodeMatMultMapRefPtr p1, NWAOBDDTopNodeMatMultMapRefPtr p2)
    {
        m1 = p1;
        m2 = p2;
    }

    size_t MatMultAddPair::MatMultAddPairHash::operator()(const MatMultAddPair& p) const
    {
        NWAOBDDTopNodeMatMultMapRefPtr t1 = p.m1;
        NWAOBDDTopNodeMatMultMapRefPtr t2 = p.m2;
        auto hash1 = t1->Hash(997);
        auto hash2 = t2->Hash(997);
        return 117 * hash1 + hash2;
    }

    bool MatMultAddPair::operator==(const MatMultAddPair& p) const
    {
        NWAOBDDTopNodeMatMultMapRefPtr m11 = m1;
        NWAOBDDTopNodeMatMultMapRefPtr m12 = m2;
        NWAOBDDTopNodeMatMultMapRefPtr m21 = p.m1;
        NWAOBDDTopNodeMatMultMapRefPtr m22 = p.m2;
        return (m11 == m21) && (m12 == m22);
    }


    ZeroValNodeInfo::ZeroValNodeInfo(NWAOBDDNodeHandle p, unsigned int i) : m(p), index(i)
    {
    }
    
    size_t ZeroValNodeInfo::ZeroValNodeInfoHash::operator()(const ZeroValNodeInfo& p) const
    {
        NWAOBDDNodeHandle t1 = p.m;
        auto hash1 = t1.Hash(997);
        return 117 * (hash1 + 1) + 997 * p.index;
    }

    bool ZeroValNodeInfo::operator==(const ZeroValNodeInfo& p) const
    {
        NWAOBDDNodeHandle m11 = m;
        NWAOBDDNodeHandle m21 = p.m;
        return (m11 == m21) && (index == p.index);
    }
} // namespace NWA_OBDD