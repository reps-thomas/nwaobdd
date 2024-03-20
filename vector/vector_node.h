#ifndef VECTOR_NODE_GUARD
#define VECTOR_NODE_GUARD

#include <string.h>
#include "../nwaobdd_node.h"
#include <boost/multiprecision/cpp_dec_float.hpp>
// #include "cflobdd_top_node_linear_map.h"

namespace NWA_OBDD {

	//typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<1000> > BIG_FLOAT;

	extern NWAOBDDNodeHandle MkBasisVectorNode(unsigned int level, unsigned int index);
	extern NWAOBDDNodeHandle MkBasisVectorNode(unsigned int level, std::string s);
	
	extern void VectorInitializerNode();  // Empty for now

	// extern NWAOBDDNodeHandle VectorToMatrixInterleavedNode(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
	// extern NWAOBDDNodeHandle MatrixToVectorNode(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
	// extern NWAOBDDNodeHandle MkColumn1MatrixNode(unsigned int level);
	// extern NWAOBDDNodeHandle MkVectorWithVoc12Node(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
	// extern NWAOBDDNodeHandle VectorShiftVocs1To2Node(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
//#ifdef PATH_COUNTING_ENABLED
	// extern std::pair<std::string,std::string> SamplingNode(NWAOBDDNodeHandle nh, unsigned int index, bool voctwo = false);
//#endif
	// needs to be removed and linked to the one in cflobdd_node.cpp
	// extern long double addNumPathsToExit(long double path1, long double path2);
	// extern long double addNumPathsToExit(std::vector<long double>& paths);
	// extern BIG_FLOAT addNumPathsToExit(std::vector<BIG_FLOAT>& paths);
	// long double getLogSumNumPaths(std::vector<std::pair<long double, unsigned int>>& numBPaths, unsigned int size);
	// BIG_FLOAT getLogSumNumPaths(std::vector<std::pair<BIG_FLOAT, unsigned int>>& numBPaths, unsigned int size);
	
	// template <typename T>
	// bool sortNumPathPairs(const std::pair<T, unsigned int>& p1, const std::pair<T, unsigned int> &p2){
	// 	if (p1.first < p2.first)
	// 		return true;
	// 	else if (p1.first > p2.first)
	// 		return false;
	// 	return p1.second < p2.second;
	// }
}

#endif

