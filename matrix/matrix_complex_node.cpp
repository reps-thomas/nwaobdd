#include "matrix_complex_node.h"
#include "matrix_complex_top_node.h"
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
        // n->InstallPathCounts();
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
        // n->InstallPathCounts();
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

        // n -> InstallPathCounts();
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

// #ifdef PATH_COUNTING_ENABLED
// 		n->InstallPathCounts();
// #endif
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
        
        // n -> InstallPathCounts();
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
        // n -> InstallPathCounts();
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
        // n -> InstallPathCounts();
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
        // n -> InstallPathCounts();
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
        // n -> InstallPathCounts();
        return NWAOBDDNodeHandle(n);
    }

}  // namespace NWA_OBDD


extern int matmult_times[];
extern int matmult_hit[];

namespace NWA_OBDD {

    template <typename T, typename T1>
    typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr
    SchalarProductTop(T1 c, typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr g)
    {
        if (c == 1) return g;  // Nothing need be performed

        NWAOBDDNodeHandle eph;
        typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr ans;
        if (c == 0) {  // Special case
            eph = NWAOBDDNodeHandle::NoDistinctionNode[g->level];

			VAL_TYPE one = 1;
			MatMultMapHandle mm;
			mm.ForceAdd(std::make_pair(-1, -1), one);

            ReturnMapHandle<T> rmh;
			rmh.AddToEnd(mm);
			rmh.Canonicalize();
            return(new NWAOBDDTopNode<T>(eph, rmh));
        }
        else {
            eph = *(g->rootConnection.entryPointHandle);
            ReturnMapHandle<T> rmh = c * g->rootConnection.returnMapHandle;
            return(new NWAOBDDTopNode<T>(eph, rmh));
        }
    }

    // std::unordered_map<MatMultPair, NWAOBDDTopNodeMatMultMapRefPtr, MatMultPair::MatMultPairHash> matmult_hashMap;
	
	std::unordered_map<MatMultPairWithInfo, NWAOBDDTopNodeMatMultMapRefPtr, MatMultPairWithInfo::MatMultPairWithInfoHash> matmult_hashMap_info[26];
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
       		MatMultPairWithInfo mmp(c1, c2, c1_zero_index, c2_zero_index);

		unsigned level = c1.handleContents -> level;
		matmult_times[level]++;
		auto it0 = matmult_hashMap_info[level].find(mmp);
		if (it0 != matmult_hashMap_info[level].end()) {
			matmult_hit[level]++;
			return it0 -> second;
		}

		NWAOBDDMatMultMapHandle g_return_map;
		NWAOBDDInternalNode* g;

		NWAOBDDInternalNode* c1_internal = (NWAOBDDInternalNode *)c1.handleContents;
		NWAOBDDInternalNode* c2_internal = (NWAOBDDInternalNode *)c2.handleContents;

		// ReductionMapHandle reductionMapHandle;

		if ((c1_internal->numExits == 1 && c1_zero_index == 0) || (c2_internal->numExits == 1 && c2_zero_index == 0)) {
            // Short Circuit for 0-matrices
            g = new NWAOBDDInternalNode(c1.handleContents->level);
			g = (NWAOBDDInternalNode *)NWAOBDDNodeHandle::NoDistinctionNode[c1.handleContents->level].handleContents;
			MatMultMapHandle m;
			VAL_TYPE one = 1;
			m.ForceAdd(std::make_pair(-1, -1), one);
			m.Canonicalize();
			g_return_map.AddToEnd(m);
			// reductionMapHandle.AddToEnd(0);
		}
		else{
			if (c1.handleContents->level == 1) {
                
				std::vector<int> m1_indices{ 0, 0, 0, 0 }, m2_indices{ 0, 0, 0, 0 };
                // preprocessing c1 indexes
                pseudoCFLOBDDBase c1_cfl(c1_internal);
				for (unsigned int i = 0; i < c1_internal->numBConnections; i++) {
                    m1_indices[2 * i] = c1_cfl.bconn[i].First();
					m1_indices[2 * i + 1] = c1_cfl.bconn[i].Second();
					
				}
				if (c1_cfl.numBConnections == 1) {
					m1_indices[2] = m1_indices[0];
					m1_indices[3] = m1_indices[1];
				}
                for (unsigned int i = 0; i < m1_indices.size(); i++) {
					if (m1_indices[i] == c1_zero_index)
						m1_indices[i] = -1;
				}
                // preprocessing c2 indexes
                pseudoCFLOBDDBase c2_cfl(c2_internal);
				for (unsigned int i = 0; i < c2_cfl.numBConnections; i++) {
                    m2_indices[2 * i] = c2_cfl.bconn[i].First();
                    m2_indices[2 * i + 1] = c2_cfl.bconn[i].Second();
				}
				if (c2_internal->numBConnections == 1) {
					m2_indices[2] = m2_indices[0];
					m2_indices[3] = m2_indices[1];
				}
				for (unsigned int i = 0; i < m2_indices.size(); i++) {
					if (m2_indices[i] == c2_zero_index)
						m2_indices[i] = -1;
				}

                // performing the MatMult for 2 * 2 "manually"
                
				MatMultMapHandle m0, m1, m2, m3;
				VAL_TYPE one = 1;
				m0.Add(std::make_pair(m1_indices[0], m2_indices[0]), one);
				m0.Add(std::make_pair(m1_indices[1], m2_indices[2]), one);
				if (m0.Size() == 0)
					m0.ForceAdd(std::make_pair(-1, -1), one);
				m0.Canonicalize();

				m1.Add(std::make_pair(m1_indices[0], m2_indices[1]), one);
				m1.Add(std::make_pair(m1_indices[1], m2_indices[3]), one);
				if (m1.Size() == 0)
					m1.ForceAdd(std::make_pair(-1, -1), one);
				m1.Canonicalize();

				m2.Add(std::make_pair(m1_indices[2], m2_indices[0]), one);
				m2.Add(std::make_pair(m1_indices[3], m2_indices[2]), one);
				if (m2.Size() == 0)
					m2.ForceAdd(std::make_pair(-1, -1), one);
				m2.Canonicalize();

				m3.Add(std::make_pair(m1_indices[2], m2_indices[1]), one);
				m3.Add(std::make_pair(m1_indices[3], m2_indices[3]), one);
				if (m3.Size() == 0)
					m3.ForceAdd(std::make_pair(-1, -1), one);
				m3.Canonicalize();

                // build a correct structure manually.
                pseudoCFLOBDDBase c;
				if ((m0 == m2) && (m1 == m3)){
                    c.numBConnections = 1;
					if (m0 == m1) {
                        c.bconn[0] = intpair(0, 0);
						c.numExits = 1;
						g_return_map.AddToEnd(m0);
					} // 7 3 7 3   3 3 6 6
					else {
                        c.numExits = 2;
						c.bconn[0] = intpair(0, 1);
						g_return_map.AddToEnd(m0);
						g_return_map.AddToEnd(m1);
						// reductionMapHandle.AddToEnd(0);
						// reductionMapHandle.AddToEnd(1);
					}
				}
				else {
					c.numBConnections = 2;
					if (m0 == m1) {
						c.bconn[0] = intpair(0, 0);
						g_return_map.AddToEnd(m0);
						// reductionMapHandle.AddToEnd(0);
					}
					else{
                        c.bconn[0] = intpair(0, 1);
						g_return_map.AddToEnd(m0);
						g_return_map.AddToEnd(m1);
						// reductionMapHandle.AddToEnd(0);
						// reductionMapHandle.AddToEnd(1);
					}

					if(m2 == m3) {
						unsigned int k;
						for (k = 0; k < g_return_map.Size(); k++){
							if (g_return_map[k] == m2)
								break;
						}
                        c.bconn[1] = intpair(k, k);
						if (k == g_return_map.Size()){
							g_return_map.AddToEnd(m2);
							// reductionMapHandle.AddToEnd(k);
						}
					}
					else{
						unsigned int k1 = g_return_map.Size(), k2 = g_return_map.Size();
						for (unsigned int k = 0; k < g_return_map.Size(); k++){
							if (g_return_map[k] == m2){
								k1 = k;
							}
							if (g_return_map[k] == m3){
								k2 = k;
							}
						}
						if (k1 == g_return_map.Size() && k2 == g_return_map.Size())
							k2++;
                        c.bconn[1] = intpair(k1, k2);
						if (k1 == g_return_map.Size()){
							g_return_map.AddToEnd(m2);
							// reductionMapHandle.AddToEnd(k1);
						}
						if (k2 == g_return_map.Size()){
							g_return_map.AddToEnd(m3);
							// reductionMapHandle.AddToEnd(k2);
						}
					}
				}
                g = c.toNWA();
			}
			else {
				std::vector<int> b_zero_indices_c1(c1_internal->numBConnections, -1), b_zero_indices_c2(c2_internal->numBConnections, -1);
				int a_zero_index_c1 = -1, a_zero_index_c2 = -1;
				pseudoCFLOBDDInternal c1_cfl(c1_internal);
				pseudoCFLOBDDInternal c2_cfl(c2_internal);

				if (c1_zero_index != -1) {
					ZeroValNodeInfo c1_zero_val_node(c1, c1_zero_index);
					auto it = hashMap.find(c1_zero_val_node);
					if (it != hashMap.end()) {
						b_zero_indices_c1 = it->second.mapContents->b_indices;
						a_zero_index_c1 = it->second.mapContents->a_index;
					}
					else {
						for (unsigned int i = 0; i < b_zero_indices_c1.size(); i++){
							b_zero_indices_c1[i] = c1_cfl.BConnection[i].returnMapHandle.LookupInv(c1_zero_index);
							if (c1_cfl.BConnection[i].returnMapHandle.Size() == 1 && c1_cfl.BConnection[i].returnMapHandle[0] == c1_zero_index)
							{
								a_zero_index_c1 = i;
							}
						}
						ZeroIndicesMapHandle zm;
						zm.mapContents->b_indices = b_zero_indices_c1;
						zm.Set_AIndex(a_zero_index_c1);
						zm.Canonicalize();
						hashMap.emplace(c1_zero_val_node, zm);
					}
				}

				if (c2_zero_index != -1){
					ZeroValNodeInfo c2_zero_val_node(c2, c2_zero_index);
					auto it = hashMap.find(c2_zero_val_node);
					if (it != hashMap.end()){
						b_zero_indices_c2 = it->second.mapContents->b_indices;
						a_zero_index_c2 = it->second.mapContents->a_index;
					}
					else{
						for (unsigned int i = 0; i < b_zero_indices_c2.size(); i++){
							b_zero_indices_c2[i] = c2_cfl.BConnection[i].returnMapHandle.LookupInv(c2_zero_index);
							if (c2_cfl.BConnection[i].returnMapHandle.Size() == 1 && c2_cfl.BConnection[i].returnMapHandle[0] == c2_zero_index)
							{
								a_zero_index_c2 = i;
							}
						}
						ZeroIndicesMapHandle zm;
						zm.mapContents->b_indices = b_zero_indices_c2;
						zm.Set_AIndex(a_zero_index_c2);
						zm.Canonicalize();
						hashMap.emplace(c2_zero_val_node, zm);
					}
				}

				NWAOBDDTopNodeMatMultMapRefPtr aa = MatrixMultiplyNode(hashMap, *(c1_cfl.AConnection.entryPointHandle),
					*(c2_cfl.AConnection.entryPointHandle), a_zero_index_c1, a_zero_index_c2);

				pseudoCFLOBDDInternal c(c1_cfl.level);

				ReturnMapHandle<int> mI;
				for (unsigned int i = 0; i < aa->rootConnection.returnMapHandle.Size(); i++)
					mI.AddToEnd(i);
				// mI.Canonicalize();

				c.AConnection = CFLConnection(*(aa->rootConnection.entryPointHandle), mI);
				c.numBConnections = mI.Size();
				c.BConnection = new CFLConnection[c.numBConnections];
				c.numExits = 0;
				
				std::unordered_map<unsigned int, unsigned int> mapFromHandleToIndex;
				for (unsigned int i = 0; i < c.numBConnections; i++) {
					MatMultMapHandle matmult_returnmap = aa->rootConnection.returnMapHandle[i];
					NWAOBDDTopNodeMatMultMapRefPtr ans;
					bool first = true;
					if (matmult_returnmap.Size() == 1 &&
						(matmult_returnmap.mapContents->map.find(std::make_pair(-1, -1)) != matmult_returnmap.mapContents->map.end())){
						NWAOBDDMatMultMapHandle tmp_return_map;
						tmp_return_map.AddToEnd(matmult_returnmap);
						tmp_return_map.Canonicalize();
						ans = new NWAOBDDTopNodeMatMultMap(NWAOBDDNodeHandle::NoDistinctionNode[c1.handleContents->level - 1], tmp_return_map);
					}
					else{
						// Consider Multiplication of M1 and M2
						for (auto &v : matmult_returnmap.mapContents->map){
							unsigned int M1_index = v.first.first;
							unsigned int M2_index = v.first.second;
							NWAOBDDTopNodeMatMultMapRefPtr bb_old =
								MatrixMultiplyNode(hashMap, *(c1_cfl.BConnection[M1_index].entryPointHandle),
								*(c2_cfl.BConnection[M2_index].entryPointHandle), b_zero_indices_c1[M1_index], b_zero_indices_c2[M2_index]);
							NWAOBDDMatMultMapHandle new_bb_return;
							for (unsigned int j = 0; j < bb_old->rootConnection.returnMapHandle.Size(); j++) {
								MatMultMapHandle tmp;
								for (auto& it : bb_old->rootConnection.returnMapHandle[j].mapContents->map) {
									if (it.first.first != -1 && it.first.second != -1)
										tmp.Add(std::make_pair(c1_cfl.BConnection[M1_index].returnMapHandle[it.first.first],
											c2_cfl.BConnection[M2_index].returnMapHandle[it.first.second]), it.second);
								}
								if (tmp.Size() == 0) {
									VAL_TYPE one = 1;
									tmp.ForceAdd(std::make_pair(-1, -1), one);
								}
								tmp.Canonicalize();
								new_bb_return.AddToEnd(tmp);
							}
							new_bb_return.Canonicalize();
							NWAOBDDTopNodeMatMultMapRefPtr bb =
								new NWAOBDDTopNodeMatMultMap(*(bb_old->rootConnection.entryPointHandle), new_bb_return);
							if (!(new_bb_return.Size() == 1 &&
								new_bb_return[0].mapContents->contains_zero_val))
								bb = SchalarProductTop<MatMultMapHandle, VAL_TYPE>(v.second, bb);
								// bb = v.second * bb;
							if (first){
								ans = bb;
								first = false;
							}
							else{
								if (!(ans->rootConnection.returnMapHandle.Size() == 1 &&
									ans->rootConnection.returnMapHandle[0].mapContents->contains_zero_val))
									// ans = ans + bb;
									ans = MkPlusTopNode<MatMultMapHandle>(ans, bb);
								else
									ans = bb;
							}
						}
					}
					ReturnMapHandle<int> ans_return_map;
					for (unsigned int j = 0; j < ans->rootConnection.returnMapHandle.Size(); j++){
						//std::string map_as_string = ans->rootConnection.returnMapHandle[j].ToString();
						unsigned int map_hash_check = ans->rootConnection.returnMapHandle[j].mapContents->hashCheck;
						auto it1 = mapFromHandleToIndex.find(map_hash_check);
						if (it1 == mapFromHandleToIndex.end()){
							ans_return_map.AddToEnd(c.numExits++);
							g_return_map.AddToEnd(ans->rootConnection.returnMapHandle[j]);
							// reductionMapHandle.AddToEnd(g->numExits - 1);
							mapFromHandleToIndex[map_hash_check] = c.numExits - 1;
						}
						else{
							unsigned int index = it1 -> second;
							ans_return_map.AddToEnd(index);
						}
					}
					// ans_return_map.Canonicalize();
					c.BConnection[i] = CFLConnection(*(ans->rootConnection.entryPointHandle), ans_return_map);
				}
				g = c.toNWA();
			}
		}
		g_return_map.Canonicalize();
		g->numExits = g_return_map.Size();
		// reductionMapHandle.Canonicalize();
// #ifdef PATH_COUNTING_ENABLED
// 		g->InstallPathCounts();
// #endif

		NWAOBDDNodeHandle gHandle(g);
		//gHandle = gHandle.Reduce(reductionMapHandle, g_return_map.Size(), true);
		NWAOBDDTopNodeMatMultMapRefPtr return_ans = new NWAOBDDTopNodeMatMultMap(gHandle, g_return_map);
		//hashMap[mmp] = return_ans;
		matmult_hashMap_info[level][mmp] = return_ans;
		return return_ans;

    }

    void clearMultMap(){
    // std::cout << "mapSize: " << matmult_hashMap.size() << std::endl;
    // matmult_hashMap.clear();
	}
} 

#include"matrix_complex_node_ext.cpp"