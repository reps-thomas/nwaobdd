#ifndef VECTOR_COMPLEX_FLOAT_BOOST_TOP_NODE_GUARD
#define VECTOR_COMPLEX_FLOAT_BOOST_TOP_NODE_GUARD

#include <iostream>
#include <string>
#include <fstream>
#include <boost/multiprecision/cpp_complex.hpp>
#include "vector_complex.h"
#include "../return_map_T.h"
namespace mp = boost::multiprecision;

typedef boost::multiprecision::cpp_dec_float_100 BIG_FLOAT;
namespace NWA_OBDD {
	typedef mp::cpp_complex_100 BIG_COMPLEX_FLOAT;
	typedef ReturnMapBody<BIG_COMPLEX_FLOAT> ComplexFloatBoostReturnMapBody;
	typedef ReturnMapHandle<BIG_COMPLEX_FLOAT> ComplexFloatBoostReturnMapHandle;
}
#include "../connection.h"
namespace NWA_OBDD {
	typedef ConnectionT<ReturnMapHandle<intpair> > Connection;
}

namespace NWA_OBDD {

	typedef NWAOBDDTopNode<BIG_COMPLEX_FLOAT> NWAOBDDTopNodeComplexFloatBoost;
	typedef NWAOBDDTopNode<BIG_COMPLEX_FLOAT>::NWAOBDDTopNodeTRefPtr NWAOBDDTopNodeComplexFloatBoostRefPtr;

	namespace VectorComplex {

		// Initialization routine
		extern void VectorInitializerTop();

		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkBasisVectorTop(unsigned int level, unsigned int index); // Representation of basis vector at index
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkBasisVectorTop(unsigned int level, std::string s); // Representation of basis vector at index
		// extern NWAOBDDTopNodeComplexFloatBoostRefPtr VectorToMatrixInterleavedTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n); // Convert vector to matrix with variables interleaved
		// extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkColumn1MatrixTop(unsigned int level);
		// extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkVectorWithVoc12Top(NWAOBDDTopNodeComplexFloatBoostRefPtr n);
		// extern NWAOBDDTopNodeComplexFloatBoostRefPtr VectorShiftVocs1To2Top(NWAOBDDTopNodeComplexFloatBoostRefPtr n);
		// extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixToVectorTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c);
		// extern NWAOBDDTopNodeComplexFloatBoostRefPtr NoDistinctionNodeTop(unsigned int level, BIG_COMPLEX_FLOAT val);
		// extern NWAOBDDTopNodeComplexFloatBoostRefPtr ConvertToDoubleTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c);
		extern long double getNonZeroProbabilityTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n);
		extern unsigned long long int GetPathCountTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, long double p);
		//ifdef PATH_COUNTING_ENABLED
		extern std::string SamplingTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, bool voctwo = false);
		// extern std::string SamplingV2Top(NWAOBDDTopNodeComplexFloatBoostRefPtr n);
		//#endif
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr VectorWithAmplitudeTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n);
		// extern void VectorPrintColumnMajorTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, std::ostream & out);
		// extern void VectorPrintColumnMajorInterleavedTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, std::ostream & out);
		void DumpVectorTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n);
	}
}

#endif

