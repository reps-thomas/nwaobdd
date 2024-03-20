#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdarg>
// #include "cflobdd_int.h"
// #include "cflobdd_top_node_int.h"
#include "vector_complex_top_node.h"
#include "vector_complex.h"
// #include "matrix1234_complex_float_boost.h"

namespace NWA_OBDD {

	template class NWAOBDD<BIG_COMPLEX_FLOAT>;
	namespace VectorComplex {

		void VectorInitializer()
		{
			VectorInitializerTop();
		}

		NWAOBDD_COMPLEX_BIG MkBasisVector(unsigned int level, unsigned int index)
		{
			return NWAOBDD_COMPLEX_BIG(MkBasisVectorTop(level, index));
		}

		NWAOBDD_COMPLEX_BIG MkBasisVector(unsigned int level, std::string s)
		{
			return NWAOBDD_COMPLEX_BIG(MkBasisVectorTop(level, s));
		}

		// NWAOBDD_COMPLEX_BIG MkColumn1Matrix(unsigned int level)
		// {
		// 	return NWAOBDD_COMPLEX_BIG(MkColumn1MatrixTop(level));
		// }

		// NWAOBDD_COMPLEX_BIG VectorToMatrixInterleaved(NWAOBDD_COMPLEX_BIG c)
		// {
		// 	NWAOBDD_COMPLEX_BIG tempNode = NWAOBDD_COMPLEX_BIG(VectorToMatrixInterleavedTop(c.root));
		// 	NWAOBDD_COMPLEX_BIG v = MkColumn1Matrix(tempNode.root->level);
		// 	return tempNode * v;
		// }

		// NWAOBDD_COMPLEX_BIG MkVectorWithVoc12(NWAOBDD_COMPLEX_BIG c)
		// {
		// 	return NWAOBDD_COMPLEX_BIG(MkVectorWithVoc12Top(c.root));
		// }

		// NWAOBDD_COMPLEX_BIG KroneckerProduct(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2)
		// {
		// 	assert(m1.root->level == m2.root->level);
		// 	NWAOBDD_COMPLEX_BIG m2_1To2 = VectorShiftVocs1To2(m2);
		// 	return m1 * m2_1To2;
		// }

		// NWAOBDD_COMPLEX_BIG VectorShiftVocs1To2(NWAOBDD_COMPLEX_BIG m1)
		// {
		// 	return NWAOBDD_COMPLEX_BIG(VectorShiftVocs1To2Top(m1.root));
		// }

		// NWAOBDD_COMPLEX_BIG NoDistinctionNode(unsigned int level, BIG_COMPLEX_FLOAT val)
		// {
		// 	return NWAOBDD_COMPLEX_BIG(NoDistinctionNodeTop(level, val));
		// }


		//#ifdef PATH_COUNTING_ENABLED
		std::string Sampling(NWAOBDD_COMPLEX_BIG c, bool isTwoVoc)
		{
			// return SamplingTop(c.root, isTwoVoc);
		}

		// std::string SamplingV2(NWAOBDD_COMPLEX_BIG c)
		// {
		// 	return SamplingV2Top(c.root);
		// }
		//#endif

		NWAOBDD_COMPLEX_BIG VectorWithAmplitude(NWAOBDD_COMPLEX_BIG c)
		{
			// return NWAOBDD_COMPLEX_BIG(VectorWithAmplitudeTop(c.root));
		}

		// void VectorPrintColumnMajor(NWAOBDD_COMPLEX_BIG c, std::ostream & out)
		// {
		// 	VectorPrintColumnMajorTop(c.root, out);
		// 	return;
		// }

		// void VectorPrintColumnMajorInterleaved(NWAOBDD_COMPLEX_BIG c, std::ostream & out)
		// {
		// 	VectorPrintColumnMajorInterleavedTop(c.root, out);
		// 	return;
		// }

		long double getNonZeroProbability(NWAOBDD_COMPLEX_BIG n)
		{
			// return getNonZeroProbabilityTop(n.root);
		}

		unsigned long long int GetPathCount(NWAOBDD_COMPLEX_BIG n, long double p)
		{
			// return GetPathCountTop(n.root, p);
		}
	}
}

