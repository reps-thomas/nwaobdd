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

		void pad_string(AssignmentIterator &ai, std::string s, unsigned level) {
			assert(level > 0);
			if(level == 1) {
				assert(s.length() == 4);
				ai.Current() = (s[0] == '1'), ai.Next();
				ai.Current() = (s[1] == '1'), ai.Next();
				ai.Current() = (s[2] == '1'), ai.Next();
				ai.Current() = (s[3] == '1'), ai.Next();
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
		Assignment index2assignment(std::string s, unsigned level) {
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

			if(GetLevelTop(n) == 0) {
				Assignment asgn0 = index2assignment("0000", 1);
				auto a00 = n -> Evaluate(asgn0);
				Assignment asgn1 = index2assignment("0100", 1);
				auto a01 = n -> Evaluate(asgn1);
				Assignment asgn2 = index2assignment("1000", 1);
				auto a10 = n -> Evaluate(asgn2);
				Assignment asgn3 = index2assignment("1100", 1);
				auto a11 = n -> Evaluate(asgn3);
				std::cout << "[\n ";
				std::cout << "[" << a00 << ", " << a01 << "]\n ";
				std::cout << "[" << a10 << ", " << a11 << "]\n ";
				std::cout << "]\n";
				return;	
			}

			unsigned level = n -> level;
			unsigned vars = 1 << (level + 1);
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
					Assignment a = index2assignment(s, level);
					auto r = n->Evaluate(a);
					std::cout << r << ", ";
				}
				std::cout << "]\n ";
			}
			std::cout << "]\n";
		}
		unsigned GetLevelTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n) {
			unsigned l = n -> level;
			if(l > 1) return l;
			NWAOBDDNode* nh = n -> rootConnection.entryPointHandle->handleContents;
			NWAOBDDInternalNode *internal = dynamic_cast<NWAOBDDInternalNode*>(nh);
			assert(internal);
			for(unsigned i = 0; i < internal -> numBConnections; ++i) {
				ReturnMapHandle<intpair> rmh;
				rmh.AddToEnd(intpair(i, i));
				rmh.Canonicalize();
				if(internal -> BConnection[0][i].returnMapHandle != rmh)
					return 1u;
				if(internal -> BConnection[1][i].returnMapHandle != rmh)
					return 1u;
			}
			return 0u;
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

		NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftLevel0Top(NWAOBDDTopNodeComplexFloatBoostRefPtr c) {
			NWAOBDDTopNodeComplexFloatBoostRefPtr v;
			NWAOBDDNodeHandle tempHandle;

			tempHandle = MatrixShiftLevel0Node(*(c->rootConnection.entryPointHandle));
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

		


	}
}

