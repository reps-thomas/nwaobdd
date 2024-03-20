#include"vector_node.h"

namespace NWA_OBDD {
    std::vector<ReturnMapHandle<intpair>>commonly_used_return_maps;

    void VectorInitializerNode() {
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
        return;
    }

    NWAOBDDNodeHandle MkBasisVectorNode(unsigned int level, unsigned int index) {
        assert(level <= 29);
        std::string s;
        for(unsigned i = 0; i < (1 << (level + 1)); ++i) {
            if(index & (1 << i))
                s.push_back('1');
            else 
                s.push_back('0');
        }
        return MkBasisVectorNode(level, s);
    }

    NWAOBDDNodeHandle MkBasisVectorNode(unsigned int level, std::string s) {

        assert(level > 0);
        Connection c00(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, commonly_used_return_maps[0]);
        Connection c01(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, commonly_used_return_maps[1]);
        Connection c10(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, commonly_used_return_maps[2]);
        Connection c11(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, commonly_used_return_maps[3]);

        ReturnMapHandle<intpair> r0011;
        r0011.AddToEnd(intpair(0, 0));
        r0011.AddToEnd(intpair(1, 1));
        r0011.Canonicalize();

        ReturnMapHandle<intpair> r1100;
        r1100.AddToEnd(intpair(1, 1));
        r1100.AddToEnd(intpair(0, 0));
        r1100.Canonicalize();

        NWAOBDDInternalNode *n = new NWAOBDDInternalNode(level);
        n -> numBConnections = n -> numExits = 2;
        n -> BConnection[0] = new Connection[2];
        n -> BConnection[1] = new Connection[2];

        if(level == 1) {
            assert(s.length() == 4);
            if(s[0] == '0' && s[1] == '0') {
                n -> AConnection[0] = c01;
                n -> AConnection[1] = c11;
                if(s[2] == '0' && s[3] == '0') {
                    n -> BConnection[0][0] = c01;
                    n -> BConnection[0][1] = c11;
                    n -> BConnection[1][0] = c11;
                    n -> BConnection[1][1] = c11;
                }
                else {
                    if(s[2] == '0') { // case 01 only
                        n -> BConnection[0][0] = c01;
                        n -> BConnection[0][1] = c00;
                        n -> BConnection[1][0] = c00;
                        n -> BConnection[1][1] = c00;
                    }
                    else { // case 10 and case 11
                        n -> BConnection[0][0] = c00;
                        n -> BConnection[0][1] = c00;
                        n -> BConnection[1][1] = c00;
                        if(s[3] == '1') // case 11
                            n -> BConnection[1][0] = c01;
                        else  // case 10
                            n -> BConnection[1][0] = c10;
                    }
                }
            }
            else {
                // ------ A-Connection Part ------
                if(s[0] == '0') { // case 01 only
                    n -> AConnection[0] = c01;
                    n -> AConnection[1] = c00;
                }
                else { // case 10 and 11
                    n -> AConnection[0] = c00;
                    if(s[1] == '0') // case 10
                        n -> AConnection[1] = c10;
                    else // case 11
                        n -> AConnection[1] = c01;
                }
                // ------ B-Connection Part ------
                n -> BConnection[0][0] = c00;
                n -> BConnection[1][0] = c00;

                Connection r_alive;
                if(s[3] == '1')
                    r_alive = c01;
                else 
                    r_alive = c10;
                if(s[2] == '0') {
                    n -> BConnection[0][1] = r_alive;
                    n -> BConnection[1][1] = c00;
                }
                else {
                    n -> BConnection[0][1] = c00;
                    n -> BConnection[1][1] = r_alive;
                }
            }
        }
        else {
            std::string first_half = s.substr(0, s.length() / 2);
            std::string second_half = s.substr(s.length() / 2);

            auto ACallee = MkBasisVectorNode(level - 1, first_half);
            auto BCallee = MkBasisVectorNode(level - 1, second_half);
            auto NoDist = NWAOBDDNodeHandle::NoDistinctionNode[level - 1];
            n -> AConnection[0] = Connection(ACallee, r0011);
            n -> AConnection[1] = n -> AConnection[0];

			if (first_half.find('1') == std::string::npos) {
                n -> BConnection[0][0] = Connection(BCallee, r0011);
                n -> BConnection[1][0] = n -> BConnection[0][0];
				if (second_half.find('1') == std::string::npos) {
                   n -> BConnection[0][1] = Connection(NoDist, commonly_used_return_maps[3]);
                   n -> BConnection[1][1] = n -> BConnection[0][1];
                }
				else {
                    n -> BConnection[0][1] = Connection(NoDist, commonly_used_return_maps[0]);
                    n -> BConnection[1][1] = n -> BConnection[0][1];
                }	
			}
			else {
                n -> BConnection[0][0] = Connection(NoDist, commonly_used_return_maps[0]);
                n -> BConnection[1][0] = n -> BConnection[0][0];
				if(second_half.find('1') == std::string::npos) {
                    n -> BConnection[0][1] = Connection(BCallee, r1100);
                    n -> BConnection[1][1] = n -> BConnection[0][1];
                }
                else {
                    n -> BConnection[0][1] = Connection(BCallee, r0011);
                    n -> BConnection[1][1] = n -> BConnection[0][1];
                }
			}
        }

#ifdef PATH_COUNTING_ENABLED
		n->InstallPathCounts();
#endif
        NWAOBDDNodeHandle handle(n);
        return handle;
    }
} // namespace NWA_OBDD