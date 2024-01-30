#ifndef VOCABULARY_ORDERING_GUARD
#define VOCABULARY_ORDERING_GUARD

/*
* The Vocabulary Ordering class - This class keeps track of the locations of the vocabularies in the
* NWAOBDD.
*/

#include "nwaobdd.h"

using namespace NWA_OBDD;

// An enumeration of the 6 vocabularies
enum vocabulary_t {BASE_PRE, BASE_POST, BASE_EXTRA, TENSOR_PRE, TENSOR_POST, TENSOR_EXTRA};
#define NO_DISTINCTION 12

// An enumeration of the possible variable orderings
enum ordering {BASE_1ST_TENSOR_ROOT, PRE_POST_1ST, BASE_TENSOR_NESTED, SEPERATE_NESTED};

class VocabularyOrdering{

public:
	// Constructor
	VocabularyOrdering(ordering orderType);

	//Get the two maps
	void getUnchangedMap(int destinationMap[12]);
	void getNoDistinctionMap(int destinationMap[12]);

	//Get the locations of the vocabularies
	int getVLoc(vocabulary_t v);
	int getBasePreLoc();
	int getBasePostLoc();
	int getBaseExtraLoc();
	int getTensorPreLoc();
	int getTensorPostLoc();
	int getTensorExtraLoc();
	NWAOBDD getBaseOne();
	NWAOBDD getBaseZero();
	NWAOBDD getBaseTop();
	NWAOBDD getTensorOne();
	NWAOBDD getTensorZero();
	NWAOBDD getTensorTop();

private:
	void createBaseOneTop(ordering orderType);
	void createBaseZeroTop(ordering orderType);
	void createBaseTopTop(ordering orderType);
	void createTensorOneTop(ordering orderType);
	void createTensorZeroTop(ordering orderType);
	void createTensorTopTop(ordering orderType);
	NWAOBDDNodeHandle createBaseOne(int level, ordering orderType);
	NWAOBDDNodeHandle createBaseZero(int level, ordering orderType);
	NWAOBDDNodeHandle createBaseTop(int level, ordering orderType);
	NWAOBDDNodeHandle createTensorOne(int level, ordering orderType);
	NWAOBDDNodeHandle createTensorZero(int level, ordering orderType);
	NWAOBDDNodeHandle createTensorTop(int level, ordering orderType);
	int unchangedMap[12];
	int noDistinctionMap[12];
	int basePreLoc;
	int basePostLoc;
	int baseExtraLoc;
	int tensorPreLoc;
	int tensorPostLoc;
	int tensorExtraLoc;
	NWAOBDD baseOne;
	NWAOBDD baseZero;
	NWAOBDD baseTop;
	NWAOBDD tensorOne;
	NWAOBDD tensorZero;
	NWAOBDD tensorTop;
};

#endif //VOCABULARY_ORDERING_GUARD