#include"vector_node.h"
#include <random>
#include"../pseudoCFLOBDD.h"

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
        for(unsigned i = 0; i < (1 << level); ++i) {
            if(index & (1 << i))
                s.push_back('1');
            else 
                s.push_back('0');
        }
        return MkBasisVectorNode(level, s);
    }

    NWAOBDDNodeHandle MkBasisVectorNode(unsigned int level, std::string s) {
        assert(level > 0);
        NWAOBDDInternalNode *n;
        if(level == 1) {
            assert(s.length() == 2);
            pseudoCFLOBDDBase c;
            c.numBConnections = c.numExits = 2;
            if(s[0] == '0') {
                if(s[1] == '0') {
                    c.bconn[0] = intpair(0, 1);
                    c.bconn[1] = intpair(1, 1);
                }
                else {
                    c.bconn[0] = intpair(0, 1);
                    c.bconn[1] = intpair(0, 0);
                }
            }
            else {
                c.bconn[0] = intpair(0, 0);
                if(s[1] == '0') 
                    c.bconn[1] = intpair(1, 0);
                else 
                    c.bconn[1] = intpair(0, 1);
            }
            n = c.toNWA();
        }
        else {
            std::string first_half = s.substr(0, s.length() / 2);
            std::string second_half = s.substr(s.length() / 2);

            auto ACallee = MkBasisVectorNode(level - 1, first_half);
            auto BCallee = MkBasisVectorNode(level - 1, second_half);
            auto NoDist = NWAOBDDNodeHandle::NoDistinctionNode[level - 1];
            pseudoCFLOBDDInternal c(level);
            c.numBConnections = c.numExits = 2;
            c.BConnection = new CFLConnection[2];
            
            auto m0 = cfl_return_map(0);
            auto m1 = cfl_return_map(1);
            auto m01 = cfl_return_map(0, 1);
            auto m10 = cfl_return_map(1, 0);

            c.AConnection = CFLConnection(ACallee, m01);

			if (first_half.find('1') == std::string::npos) {
                c.BConnection[0] = CFLConnection(BCallee, m01); // 0, 1
				if (second_half.find('1') == std::string::npos) 
					c.BConnection[1] = CFLConnection(NoDist, m1); // 1
				else
                    c.BConnection[1] = CFLConnection(NoDist, m0); // 0
			}
			else {
				c.BConnection[0] = CFLConnection(NoDist, m0); // 0
				if (second_half.find('1') == std::string::npos)
					c.BConnection[1] = CFLConnection(BCallee, m10); // 1, 0
				else
					c.BConnection[1] = CFLConnection(BCallee, m01); // 0, 1
			}
            n = c.toNWA();
        }
// #ifdef PATH_COUNTING_ENABLED
// 		n->InstallPathCounts();
// #endif
        NWAOBDDNodeHandle handle(n);
        return handle;
    }

    template <typename T>
	bool sortNumPathPairs(const std::pair<T, unsigned int>& p1, const std::pair<T, unsigned int> &p2) {
		if (p1.first < p2.first)
			return true;
		else if (p1.first > p2.first)
			return false;
		return p1.second < p2.second;
	}
    // template bool sortNumPathPairs<BIG_FLOAT>(const std::pair<BIG_FLOAT, unsigned int>& p1, const std::pair<BIG_FLOAT, unsigned int> &p2);
    // template bool sortNumPathPairs<BIG_COMPLEX_FLOAT>(const std::pair<BIG_COMPLEX_FLOAT, unsigned int>& p1, const std::pair<BIG_COMPLEX_FLOAT, unsigned int> &p2);

	long double addNumPathsToExit(std::vector<long double>& logOfPaths){
		if (logOfPaths.size() == 1)
			return logOfPaths.back();
		long double sum = 0.0;
		for (int i = 0; i < logOfPaths.size() - 1; i++){
			if (logOfPaths[i] != -1.0 * std::numeric_limits<double>::infinity())
				sum += pow(2, logOfPaths[i] - logOfPaths.back());
		}
		long double logOfSum = logOfPaths.back() + log1p(sum) / ((double)log(2));
		return logOfSum;
	}

	BIG_FLOAT addNumPathsToExit(std::vector<BIG_FLOAT>& logOfPaths){
		if (logOfPaths.size() == 1)
			return logOfPaths.back();
		BIG_FLOAT sum = 0.0;
		for (int i = 0; i < logOfPaths.size() - 1; i++){
			if (logOfPaths[i] != -1.0 * std::numeric_limits<BIG_FLOAT>::infinity())
				sum += boost::multiprecision::pow(2, logOfPaths[i] - logOfPaths.back());
		}
		BIG_FLOAT logOfSum = logOfPaths.back() + (boost::multiprecision::log1p(sum) / ((double)log(2)));
		return logOfSum;
	}

    long double getLogSumNumPaths(std::vector<std::pair<long double, unsigned int>>& numBPaths, unsigned int size){
		std::vector<long double> paths;
		assert(numBPaths.size() >= size);
		for (int i = 0; i < size; i++)
			paths.push_back(numBPaths[i].first);
		return addNumPathsToExit(paths);
	}

	BIG_FLOAT getLogSumNumPaths(std::vector<std::pair<BIG_FLOAT, unsigned int>>& numBPaths, unsigned int size){
		std::vector<BIG_FLOAT> paths;
		assert(numBPaths.size() >= size);
		for (int i = 0; i < size; i++)
			paths.push_back(numBPaths[i].first);
		return addNumPathsToExit(paths);
	}


    void forcePathCountNode(NWAOBDDNodeHandle n) {
        NWAOBDDNode* n1 = n.handleContents;
        if(n1 -> NodeKind() == NWAOBDD_INTERNAL) {
            NWAOBDDInternalNode* n2 = (NWAOBDDInternalNode*)n1;
            if(!(n2 -> numPathsToExit) ) {
                forcePathCountNode(*(n2->AConnection[0].entryPointHandle));
                forcePathCountNode(*(n2->AConnection[1].entryPointHandle));
                for(unsigned i = 0; i < n2 -> numBConnections; ++i) {
                    forcePathCountNode(*(n2->BConnection[0][i].entryPointHandle));
                    forcePathCountNode(*(n2->BConnection[1][i].entryPointHandle));
                }
                n2 -> InstallPathCounts();
            }
        }
    }

    std::string SamplingNode(NWAOBDDNodeHandle nh, unsigned int index, bool remove_column_index) {
        NWAOBDDNode *n = nh.handleContents;
        if(n->level == 1) {
            std::vector<std::string>sat;
            for(unsigned id = 0; id < 4; ++id) {
                SH_OBDD::Assignment a(4);
                a[0] = (id & 1);
                a[2] = (id & 2) ? 1 : 0;
                a[1] = a[3] = 0;
                SH_OBDD::AssignmentIterator ai(a);
                unsigned r = n -> Traverse(ai);
                if(r == index) {
                    std::string s;
                    s.push_back((id & 1) ? '1' : '0');
                    if(!remove_column_index)
                        s.push_back((id & 2) ? '1' : '0');
                    sat.push_back(s);
                }
            }
            assert(sat.size() > 0);
            double random_value = ((double)rand() / (RAND_MAX));
            std::string res;
            for(unsigned i = 0; i < sat.size(); ++i) {
                if(random_value * sat.size() <= 1.0 * (i + 1) + 1e-5) {
                    res = sat[i];
                    break;
                }
            }
            return res;
        }
        else {
            // XZ Warning: 
            // Current Implementation only work when NWAOBDDs are simulating CFLOBDDs.
            auto nh =  dynamic_cast<NWAOBDDInternalNode*>(n);
            // auto (nh -> AConnection[0]) = nh->AConnection[0];
            auto BConn = nh->BConnection[0];

            std::vector<std::pair<long double, unsigned int>> numBPaths;

    		long double numBTotalPaths = 0.0;
            for (unsigned int i = 0; i < nh->numBConnections; i++)
            {
                int BIndex = BConn[i].returnMapHandle.LookupInv(intpair(index, index));
                if(BIndex == -1) {
                    numBPaths.push_back(std::make_pair(-1 * std::numeric_limits<long double>::infinity(), i));
                }
                else {
                    numBPaths.push_back(std::make_pair(BConn[i].entryPointHandle->handleContents->numPathsToExit[BIndex] + 
                    (nh -> AConnection[0]).entryPointHandle->handleContents->numPathsToExit[i], i));
                }
            }
            
            sort(numBPaths.begin(), numBPaths.end(), sortNumPathPairs<long double>);
            numBTotalPaths = getLogSumNumPaths(numBPaths, numBPaths.size());
            long double random_value = 0.0;
            if (numBTotalPaths >= 64){
                std::random_device rd;
                std::default_random_engine generator(rd());
                std::uniform_int_distribution<long long unsigned> distribution(0, 0xFFFFFFFFFFFFFFFF);
                random_value = log2l(distribution(generator)) + numBTotalPaths - 64;
            }
            else {
                random_value = log2l((((double)rand()) / RAND_MAX)*pow(2, numBTotalPaths));
            }
            long double val = -1 * std::numeric_limits<long double>::infinity();
            /*cpp_int random_value = gen() % numBTotalPaths;
            cpp_int val = 0;*/
            /*unsigned long long int random_value = rand() % numBTotalPaths;
            unsigned long long int val = 0;*/
            int BConnectionIndex = -1;
            for (unsigned int i = 0; i < numBPaths.size(); i++)
            {
                if (numBPaths[i].first == -1 * std::numeric_limits<long double>::infinity())
                    continue;
                val = getLogSumNumPaths(numBPaths, i+1);
                if (val >= random_value)
                {
                    BConnectionIndex = numBPaths[i].second;
                    break;
                }
            }
            assert(BConnectionIndex != -1);
            assert(BConn[BConnectionIndex].returnMapHandle.LookupInv(intpair(index, index)) != -1);
            assert(BConnectionIndex < nh->numBConnections);
            assert(BConn[BConnectionIndex].returnMapHandle.LookupInv(intpair(index, index)) < BConn[BConnectionIndex].returnMapHandle.mapContents->mapArray.size());
            std::string AString = SamplingNode(*((nh -> AConnection[0]).entryPointHandle), BConnectionIndex, remove_column_index);
            std::string BString = SamplingNode(*(BConn[BConnectionIndex].entryPointHandle), BConn[BConnectionIndex].returnMapHandle.LookupInv(intpair(index, index)), remove_column_index);
            return AString + BString;
        }
    }
} // namespace NWA_OBDD