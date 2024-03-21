#include"vector_node.h"
#include <random>

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

    std::pair<std::string,std::string> SamplingNode(NWAOBDDNodeHandle nh, unsigned int index, bool voctwo) {
        NWAOBDDNode *n = nh.handleContents;
        if(n->level == 1) {
            std::vector<std::string>sat;
            for(unsigned i = 0; i < 16; ++i) {
                std::string s;
                SH_OBDD::Assignment a(4);
                for(unsigned j = 0; j < 4; ++j) {
                    if(i & (1 << j)) 
                        s.push_back('1'), a[j] = 1;
                    else 
                        s.push_back('0'), a[j] = 0;
                }
                SH_OBDD::AssignmentIterator ai(a);
                unsigned r = n -> Traverse(ai);
                if(r == index)
                    sat.push_back(s);
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
            return std::make_pair(res, "");
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
            else{
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
            std::pair<std::string,std::string> AString = SamplingNode(*((nh -> AConnection[0]).entryPointHandle), BConnectionIndex, voctwo);
            std::pair<std::string, std::string> BString = SamplingNode(*(BConn[BConnectionIndex].entryPointHandle), BConn[BConnectionIndex].returnMapHandle.LookupInv(intpair(index, index)), voctwo);
            if (nh->level == 1)
                return std::make_pair(AString.first, BString.first);
            if (nh->level == 2 && !voctwo)
                return std::make_pair(AString.first, BString.first);
            return std::make_pair(AString.first + BString.first, AString.second + BString.second);
        }
    }
} // namespace NWA_OBDD