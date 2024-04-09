//
//    Copyright (c) 2017, 2018 Thomas W. Reps
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

#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <complex>
#include <math.h>
#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <boost/unordered_map.hpp>

#include "../nwaobdd_node.h"
#include "../nwaobdd_top_node.h"
#include "matrix_complex_node.h"
#include "matrix_complex_top_node.h"
#include "../assignment.h"

using namespace SH_OBDD;
namespace NWA_OBDD {

	namespace MatrixComplex {

		void Matrix1234InitializerTop()
		{
			Matrix1234InitializerNode();
			return;
		}

		static void pad_string(AssignmentIterator &ai, std::string s, unsigned level) {
			assert(level > 0);
			if(level == 1) {
				assert(s.length() == 2);
				ai.Current() = (s[0] == '1'), ai.Next();
				ai.Current() = 0, ai.Next();
				ai.Current() = (s[1] == '1'), ai.Next();
				ai.Current() = 0, ai.Next();
			}
			else {
				ai.Current() = 0, ai.Next();
				pad_string(ai,s.substr(0, s.length() / 2), level - 1);
				ai.Current() = 0, ai.Next();
				ai.Current() = 0, ai.Next();
				pad_string(ai,s.substr(s.length() / 2), level - 1);
				ai.Current() = 0, ai.Next();
			}
		}
		static Assignment index2assignment(std::string s, unsigned level) {
			Assignment a( (1 << (level + 2)) - 4 );
			AssignmentIterator ai(a);
			pad_string(ai, s, level);
			return a;
		}
		static std::string make_interleave(std::string s1, std::string s2) {
			assert(s1.length() == s2.length());
			std::string s;
			for(unsigned i = 0; i < s1.length(); ++i)
				s.push_back(s1[i]), s.push_back(s2[i]);
			return s;
		}
		void DumpMatrixTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n) {
			unsigned l = n -> level;
			assert(l > 0);
			if(l == 1) {
				Assignment asgn0 = index2assignment("00", 1);
				auto a00 = n -> Evaluate(asgn0);
				Assignment asgn1 = index2assignment("01", 1);
				auto a01 = n -> Evaluate(asgn1);
				Assignment asgn2 = index2assignment("10", 1);
				auto a10 = n -> Evaluate(asgn2);
				Assignment asgn3 = index2assignment("11", 1);
				auto a11 = n -> Evaluate(asgn3);
				std::cout << "[\n ";
				std::cout << "[" << a00 << ", " << a01 << "]\n ";
				std::cout << "[" << a10 << ", " << a11 << "]\n ";
				std::cout << "]\n";
				return;	
			}
			else {
				unsigned vars = 1 << l;
				std::cout << "[\n ";
				for(unsigned x = 0; x < (1 << (vars / 2)); ++x) {
					std::cout << "[";
					std::string s1;
					for(unsigned j = 0; j < vars / 2; ++j)
						if(x & (1 << j)) s1.push_back('1');
						else s1.push_back('0');
					for(unsigned y = 0; y < (1 << (vars / 2)); ++y) {
						std::string s2;
						for(unsigned j = 0; j < vars / 2; ++j)
							if(y & (1 << j)) s2.push_back('1');
							else s2.push_back('0');
						std::string s = make_interleave(s1, s2);
						Assignment a = index2assignment(s, l);
						auto r = n->Evaluate(a);
						std::cout << r << ", ";
					}
					std::cout << "]\n ";
				}
				std::cout << "]\n";
			}		
		}


		NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftToAConnectionTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c) {
			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			NWAOBDDNodeHandle tempHandle;

			tempHandle = MatrixShiftToAConnectionNode(*(c->rootConnection.entryPointHandle));
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, c->rootConnection.returnMapHandle);
			return v;
		}

		NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftToBConnectionTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c) {
			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			NWAOBDDNodeHandle tempHandle;

			tempHandle = MatrixShiftToBConnectionNode(*(c->rootConnection.entryPointHandle));
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, c->rootConnection.returnMapHandle);
			return v;
		}


	}

	namespace MatrixComplex {
        NWAOBDDTopNodeComplexFloatBoostRefPtr MkIdTop(unsigned int i) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkIdNode(i);

            ReturnMapHandle<BIG_COMPLEX_FLOAT> m10;
			m10.AddToEnd(1);
			m10.AddToEnd(0);
			m10.Canonicalize();

            NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m10);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkWalshTop(unsigned int i) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkWalshNode(i);

            ReturnMapHandle<BIG_COMPLEX_FLOAT> m10;
			auto val = boost::multiprecision::pow(sqrt(2), boost::multiprecision::pow(BIG_COMPLEX_FLOAT(2), i));
			
			m10.AddToEnd(1 / val);
			m10.AddToEnd(-1 / val);
			m10.Canonicalize();

            NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m10);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkNegationTop(unsigned int i) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkNegationNode(i);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m01;
			m01.AddToEnd(0);
			m01.AddToEnd(1);
			m01.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m01);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkPauliYTop(unsigned int i) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkPauliYNode(i);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;
			BIG_COMPLEX_FLOAT img_i(0, 1);
			m.AddToEnd(0);
			m.AddToEnd(- 1.0 * img_i );
			m.AddToEnd(1.0 * img_i);
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkPauliZTop(unsigned int i) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkPauliZNode(i);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;
			BIG_COMPLEX_FLOAT img_i(0, 1);
			m.AddToEnd(1);
			m.AddToEnd(0);
			m.AddToEnd(-1);
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkSTop(unsigned int i) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkSNode(i);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;
			m.AddToEnd(1);
			m.AddToEnd(0);
			m.AddToEnd(BIG_COMPLEX_FLOAT(0, 1));
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkPhaseShiftTop(unsigned int i, double theta) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkSNode(i);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;

			auto cos_v = boost::math::cos_pi(theta);
			auto sin_v = boost::math::sin_pi(theta);
			m.AddToEnd(1);
			m.AddToEnd(0);
			m.AddToEnd(BIG_COMPLEX_FLOAT(cos_v, sin_v));
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkCNotTop(unsigned int level, unsigned int n, long int controller, long int controlled) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkCNot2Node(level, n, controller, controlled);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;
			m.AddToEnd(1);
			m.AddToEnd(0);
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkCCNotTop(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkCCNotNode(level, n, controller1, controller2, controlled);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;
			m.AddToEnd(1);
			m.AddToEnd(0);
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkSwapTop(unsigned int level, long int c1, long int c2) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkSwapNode(level, c1, c2, -1);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;
			m.AddToEnd(1);
			m.AddToEnd(0);
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkiSwapTop(unsigned int level, long int c1, long int c2) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkiSwapNode(level, c1, c2, -1);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;
			m.AddToEnd(1);
			m.AddToEnd(0);
			m.AddToEnd(BIG_COMPLEX_FLOAT(0, 1));
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkCSwapTop(unsigned int level, long int c, long int i, long int j) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkCSwap2Node(level, c, i, j, -1);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;
			m.AddToEnd(1);
			m.AddToEnd(0);
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkCPTop(unsigned int level, long int c1, long int c2, double theta) {
			NWAOBDDNodeHandle tempHandle;
			tempHandle = MkCPNode(level, c1, c2);

			ReturnMapHandle<BIG_COMPLEX_FLOAT> m;
			double cos_v = boost::math::cos_pi(theta);
			double sin_v = boost::math::sin_pi(theta);
			BIG_COMPLEX_FLOAT val(cos_v, sin_v);
			m.AddToEnd(1);
			m.AddToEnd(0);
			m.AddToEnd(val);
			m.Canonicalize();

			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			v = new NWAOBDDTopNodeComplexFloatBoost(tempHandle, m);
			return v;
		}
	}
	namespace MatrixComplex {
		NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixMultiplyTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c1, NWAOBDDTopNodeComplexFloatBoostRefPtr c2) {
			std::unordered_map<ZeroValNodeInfo, ZeroIndicesMapHandle, ZeroValNodeInfo::ZeroValNodeInfoHash> hashMap;
			int c1_zero_index = -1, c2_zero_index = -1;
			c1_zero_index = c1->rootConnection.returnMapHandle.LookupInv(0);
			c2_zero_index = c2->rootConnection.returnMapHandle.LookupInv(0);
			NWAOBDDTopNodeMatMultMapRefPtr c = MatrixMultiplyNode
				(hashMap, *(c1->rootConnection.entryPointHandle), *(c2->rootConnection.entryPointHandle),
				c1_zero_index, c2_zero_index);
			ComplexFloatBoostReturnMapHandle v;
			boost::unordered_map<BIG_COMPLEX_FLOAT, unsigned int> reductionMap;
			ReductionMapHandle reductionMapHandle;
			for (unsigned int i = 0; i < c->rootConnection.returnMapHandle.Size(); i++){
				MatMultMapHandle r = c->rootConnection.returnMapHandle[i];
				BIG_COMPLEX_FLOAT val = 0;
				for (auto &j : r.mapContents->map){
					unsigned int index1 = j.first.first;
					unsigned int index2 = j.first.second;
					if (index1 != -1 && index2 != -1){
						auto factor = BIG_COMPLEX_FLOAT(j.second, 0);
						// auto factor = j.second.convert_to<BIG_COMPLEX_FLOAT>();
						val = val + (factor * (c1->rootConnection.returnMapHandle[index1] * c2->rootConnection.returnMapHandle[index2]));
					}
				}
				if (reductionMap.find(val) == reductionMap.end()){
					v.AddToEnd(val);
					reductionMap.insert(std::make_pair(val, v.Size() - 1));
					reductionMapHandle.AddToEnd(v.Size() - 1);
				}
				else{
					reductionMapHandle.AddToEnd(reductionMap[val]);
				}
			}

			v.Canonicalize();
			reductionMapHandle.Canonicalize();
			NWAOBDDNodeHandle tempHandle = *(c->rootConnection.entryPointHandle);
			// Perform reduction on tempHandle, with respect to the common elements that rmh maps together
			/*ReductionMapHandle inducedReductionMapHandle;
			FloatBoostReturnMapHandle inducedReturnMap;
			v.InducedReductionAndReturnMap(inducedReductionMapHandle, inducedReturnMap);
			NWAOBDDNodeHandle reduced_tempHandle = tempHandle.Reduce(inducedReductionMapHandle, inducedReturnMap.Size());*/
			NWAOBDDNodeHandle reduced_tempHandle = tempHandle.Reduce(reductionMapHandle, v.Size(), true);
			// Create and return NWAOBDDTopNode
			//return(new NWAOBDDTopNodeFloatBoost(reduced_tempHandle, inducedReturnMap));
			return(new NWAOBDDTopNodeComplexFloatBoost(reduced_tempHandle, v));
		}
	}
}

