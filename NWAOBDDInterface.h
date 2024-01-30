#ifndef RELATION_INTERFACE_NWAOBDD_GUARD
#define RELATION_INTERFACE_NWAOBDD_GUARD

#include "RelationInterface.h"
#include "nwaobdd.h"
#include "vocabulary_ordering.h"


/* An instance of RelationInterface for NWAODDs - takes a vocabulary ordering and executes the
* various operations based off of that ordering
*/
namespace NWA_OBDD {
class NWAOBDDInterface: public RelationInterface
{
	public:
		/* Functions that manipulate vocabulary placement in the NWAOBDD */
		//Construction
		//ETTODO - create a copy constructor
		NWAOBDDInterface(VocabularyOrdering * vOrdering, NWAOBDD n, bool isTensored);
		NWAOBDDInterface* Transpose();
		NWAOBDDInterface* Tensor(NWAOBDDInterface NI);
		NWAOBDDInterface* Detensor();
		NWAOBDDInterface* DetensorTranspose();
		NWAOBDDInterface* MoveBaseToTensor();
		NWAOBDDInterface* ExistentialQuantification(vocabulary_t vocabs[], int length);
		/*TODO: Operations on NWAOBDDs - Extend/Combine/One/Zero/OneT/ZeroT*/
		NWAOBDDInterface* Union(NWAOBDDInterface * RI);
		NWAOBDDInterface* Compose(NWAOBDDInterface * that);
	protected:
		/* Creates the various maps needed by AdjustedPathSummary*/
		bool isZero();
		bool isOne();
		void CreateTransposeMap(int newMap[12]);
		void CreateDetensorMap(int newMap[12]);
		void CreateDetensorTransposeMap(int newMap[12]);
		void CreateMoveBaseToTensorMap(int newMap[12]);
		void CreateExistentialMap(vocabulary_t * vocabs, int length, int newMap[12]);
		void CreateBaseRightShiftMap(int newMap[12]);
		void CreateTensorRightShiftMap(int newMap[12]);
		void CreateBaseRestoreMap(int newMap[12]);
		void CreateTensorRestoreMap(int newMap[12]);
		NWAOBDDTopNodeRefPtr GetWeight(int map[12], int vals[12], int vLocs[12], int c);
		NWAOBDDTopNodeRefPtr AdjustedPathSummaryRecursive(NWAOBDDInternalNode * n, int exit, int map[12], int offset);
		NWAOBDDTopNodeRefPtr AdjustedPathSummaryRecursive(NWAOBDDNodeHandle n, int exit, int map[12], int offset);
		NWAOBDDInterface* SetEqualVocabs(vocabulary_t v1, vocabulary_t v2);
		NWAOBDD AdjustedPathSummary(NWAOBDD n, int * map);
	private:
		NWAOBDD n;
		VocabularyOrdering * vOrdering;
		bool isTensored;
};
}

#endif //RELATION_INTERFACE_NWAOBB_GUARD