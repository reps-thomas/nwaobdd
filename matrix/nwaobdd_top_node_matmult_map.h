#ifndef NWAOBDD_TOP_NODE_MATMULT_MAP_GUARD
#define NWAOBDD_TOP_NODE_MATMULT_MAP_GUARD

//
//    Copyright (c) 2017 Thomas W. Reps
//    All Rights Reserved.
//
//    This software is furnished under a license and may be used and
//    copied only in accordance with the terms of such license and the
//    inclusion of the above copyright notice.  This software or any
//    other copies thereof or any derivative works may not be provided
//    or otherwise made available to any other person.  Title to and
//    ownership of the software and any derivative works is retained
//    by Thomas W. Reps.
//
//    THIS IMPLEMENTATION MAY HAVE BUGS, SOME OF WHICH MAY HAVE SERIOUS
//    CONSEQUENCES.  THOMAS W. REPS PROVIDES THIS SOFTWARE IN ITS "AS IS"
//    CONDITION, AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
//    BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
//    THOMAS W. REPS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//


// #include <iostream>
// #include <fstream>
// #include <complex>
#include "matmult_map.h"
#include "../nwaobdd_top_node.h"
// #include "matrix1234_complex_double_top_node.h"
// #include "matrix1234_double_top_node.h"
// #include "matrix1234_float_boost_top_node.h"

namespace NWA_OBDD {

	typedef ReturnMapHandle<MatMultMapHandle> NWAOBDDMatMultMapHandle;
	typedef ConnectionT<NWAOBDDMatMultMapHandle> MatMultMapConnection;

	typedef NWAOBDDTopNode<MatMultMapHandle> NWAOBDDTopNodeMatMultMap;
	typedef NWAOBDDTopNode<MatMultMapHandle>::NWAOBDDTopNodeTRefPtr NWAOBDDTopNodeMatMultMapRefPtr;

	class MatMultPair{
	public:
		NWAOBDDNodeHandle m1;
		NWAOBDDNodeHandle m2;
		MatMultPair(NWAOBDDNodeHandle p1, NWAOBDDNodeHandle p2);

		struct MatMultPairHash {
			size_t operator()(const MatMultPair& p) const;
		};

		bool operator==(const MatMultPair& p) const;
	};

	class MatMultPairWithInfo {
	public:
		NWAOBDDNodeHandle m1;
		NWAOBDDNodeHandle m2;
		int c1_index;
		int c2_index;
		MatMultPairWithInfo(NWAOBDDNodeHandle p1, NWAOBDDNodeHandle p2, int c1, int c2);

		struct MatMultPairWithInfoHash {
			size_t operator()(const MatMultPairWithInfo& p) const;
		};

		bool operator==(const MatMultPairWithInfo& p) const;
	};

	class MatMultAddPair{
	public:
		NWAOBDDTopNodeMatMultMapRefPtr m1;
		NWAOBDDTopNodeMatMultMapRefPtr m2;
		MatMultAddPair(NWAOBDDTopNodeMatMultMapRefPtr p1, NWAOBDDTopNodeMatMultMapRefPtr p2);

		struct MatMultAddPairHash {
			size_t operator()(const MatMultAddPair& p) const;
		};

		bool operator==(const MatMultAddPair& p) const;
	};

	class ZeroValNodeInfo {
	public:
		NWAOBDDNodeHandle m;
		unsigned int index;
		ZeroValNodeInfo(NWAOBDDNodeHandle p, unsigned int i);

		struct ZeroValNodeInfoHash {
			size_t operator()(const ZeroValNodeInfo& p) const;
		};

		bool operator==(const ZeroValNodeInfo& p) const;
	};


} // namespace NWA_OBDD

#endif
