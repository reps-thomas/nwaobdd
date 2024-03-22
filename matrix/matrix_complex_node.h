#ifndef MATRIX1234_NODE_GUARD
#define MATRIX1234_NODE_GUARD

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

#include "../nwaobdd_node.h"
#include <map>
#include <unordered_map>

namespace NWA_OBDD {

	typedef std::pair<long, long> INT_PAIR;
	enum VisitPosition { AVisit, BVisit, TopLevelVisit };
	extern int nodeNum;
	extern int cacheNodeNum;
	extern int notL1NodeNum;
	extern void clearMultMap();
	extern NWAOBDDNodeHandle MkIdRelationInterleavedNode(unsigned int level);
	// extern NWAOBDDNodeHandle MkWalshInterleavedNode(unsigned int i);
	// extern NWAOBDDNodeHandle MkInverseReedMullerInterleavedNode(unsigned int i);
	// extern NWAOBDDNodeHandle MkNegationMatrixInterleavedNode(unsigned int i);
	// extern NWAOBDDNodeHandle MkPauliYInterleavedNode(unsigned int i);
	// extern NWAOBDDNodeHandle MkPauliZInterleavedNode(unsigned int i);
	// extern NWAOBDDNodeHandle MkSGateInterleavedNode(unsigned int i);
	// extern NWAOBDDNodeHandle MkCNOTInterleavedNode(unsigned int i);
	// extern NWAOBDDNodeHandle MkExchangeInterleavedNode(unsigned int i);
	// extern NWAOBDDNodeHandle MkCNOTNode(unsigned int level, unsigned int n, long int controller, long int controlled);
	// extern NWAOBDDNodeHandle MkCNOT2Node(unsigned int level, unsigned int n, long int controller, long int controlled);
	// extern NWAOBDDNodeHandle MkCCNOTNode(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled);
	// extern NWAOBDDNodeHandle MkMCXNode(unsigned int level, unsigned int n, std::vector<long int>& controllers, long int controlled);
	// extern NWAOBDDNodeHandle MkCPGateNode(unsigned int level, long int controller, long int controlled);
	// extern NWAOBDDNodeHandle MkSwapGateNode(unsigned int level, long int controller, long int controlled, int case_num);
	// extern NWAOBDDNodeHandle MkiSwapGateNode(unsigned int level, long int controller, long int controlled, int case_num);
	// extern NWAOBDDNodeHandle MkCSwapGateNode(unsigned int level, long int controller, long int i, long int j, int case_num);
	// extern NWAOBDDNodeHandle MkCSwapGate2Node(unsigned int level, long int controller, long int i, long int j, int case_num);
	// extern NWAOBDDNodeHandle MkCCPNode(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled);
	// extern std::pair<NWAOBDDNodeHandle, int> MkRestrictNode(unsigned int level, std::string s);
	
	// Initialization routine that needs to be called before any call to MatrixProjectVoc23Node
	extern void Matrix1234InitializerNode();  // Empty for now

	// Matrix-related operations (on matrices with room for two extra vocabularies) ------------
	// extern NWAOBDDNodeHandle MkWalshVoc13Node(unsigned int i);
	// extern NWAOBDDNodeHandle MkWalshVoc12Node(unsigned int i);
	// extern NWAOBDDNodeHandle MatrixShiftVoc43Node(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
	// extern NWAOBDDNodeHandle MatrixShiftVoc42Node(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
	// extern NWAOBDDNodeHandle MatrixShiftVocs13To24Node(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
	// extern NWAOBDDNodeHandle MatrixShiftVocs12To34Node(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
	// extern NWAOBDDNodeHandle MkDetensorConstraintInterleavedNode(unsigned int i);
	// extern NWAOBDDTopNodeLinearMapRefPtr MatrixProjectVoc23Node(NWAOBDDLinearMapMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh, VisitPosition position); // Vocabulary projection
	// extern NWAOBDDNodeHandle ReverseColumnsNode(NWAOBDDNodeHandle nh);
	// extern std::pair<NWAOBDDNodeHandle, NWAOBDDReturnMapHandle>
	// 	MatrixTransposeNode(std::unordered_map<NWAOBDDNodeHandle, std::pair<NWAOBDDNodeHandle, NWAOBDDReturnMapHandle>, 
	// 	NWAOBDDNodeHandle::NWAOBDDNodeHandle_Hash>& hashMap,
	// 	NWAOBDDNodeHandle nh);

	extern NWAOBDDNodeHandle MatrixShiftToAConnectionNode(NWAOBDDNodeHandle c);
	extern NWAOBDDNodeHandle MatrixShiftToBConnectionNode(NWAOBDDNodeHandle c);

	// Subroutines for Discrete Fourier Transform
	// extern NWAOBDDNodeHandle MkNWAOBDDMatrixEqVoc14Node(unsigned int i);
	// extern NWAOBDDNodeHandle MkFourierDiagonalComponentNode(unsigned int i);
	// extern NWAOBDDNodeHandle PromoteInterleavedTo12Node(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
	// extern NWAOBDDNodeHandle Demote12ToInterleavedNode(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);
	// extern NWAOBDDNodeHandle PromoteInterleavedTo13Node(NWAOBDDNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nh);

	// extern NWAOBDDNodeHandle SMatrixNode(std::string s);
	// extern NWAOBDDNodeHandle MkDistinctionTwoVarsNode(int x, int y, unsigned int var_level, unsigned int matrix_level);

	// // extern NWAOBDD_GENERAL AddMatrixRowsNode(NWAOBDDGeneralMapNodeMemoTableRefPtr memoTable, NWAOBDDNodeHandle nhHandle);
	// extern std::pair<NWAOBDDNodeHandle, std::vector<std::vector<std::pair<int, int>>>> MultiplyOperationNode(NWAOBDDNodeHandle c);
	// std::pair<NWAOBDDNodeHandle, std::vector<std::vector<std::pair<int, int>>>> MultiplyOperationNodeInternal(NWAOBDDNodeHandle c, char position, unsigned int maxLevel, NWAOBDDReturnMapHandle cReturnMapHandle);
	// std::vector<std::vector<std::pair<int, int>>> multiplyGeneralMap(int multiple, std::vector<std::vector<std::pair<int, int>>> &tmpReturnMap);
	// std::pair<NWAOBDDNodeHandle, std::vector<std::vector<std::pair<int, int>>>> addNodes(NWAOBDDNodeHandle n1, NWAOBDDNodeHandle n2, std::vector<std::vector<std::pair<int, int>>> n1GeneralMap, std::vector<std::vector<std::pair<int, int>>> n2GeneralMap);
	// extern NWAOBDDNodeHandle MkMatrixMultiplyConstraintNode(unsigned int level);
	// NWAOBDDNodeHandle MkMatrixMultiplyConstraintNodeInternal(unsigned int level);
	// NWAOBDDNodeHandle MkRowWithOne(unsigned int bits, unsigned int rowNo, std::map<std::pair<int, int>, NWAOBDDNodeHandle>&  memoTable);
	// void changeGeneralMap(std::vector<std::vector<std::pair<int, int>>> &generalMap, NWAOBDDReturnMapHandle returnMapHandle);
	// extern NWAOBDDTopNodeMatMultMapRefPtr MatrixMultiplyV4Node(
	// 	std::unordered_map<MatMultPair, NWAOBDDTopNodeMatMultMapRefPtr, MatMultPair::MatMultPairHash>& hashMap,
	// 	NWAOBDDNodeHandle c1, NWAOBDDNodeHandle c2);
	// extern NWAOBDDTopNodeMatMultMapRefPtr MatrixMultiplyV4WithInfoNode(
	// 	std::unordered_map<ZeroValNodeInfo, ZeroIndicesMapHandle, ZeroValNodeInfo::ZeroValNodeInfoHash>& hashMap,
	// 	NWAOBDDNodeHandle c1, NWAOBDDNodeHandle c2, int c1_zero_index, int c2_zero_index);
 }

#endif

