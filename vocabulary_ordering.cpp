#include "vocabulary_ordering.h"

/*The constructor - based on the ordering provided determines the locations of the vocabularies
* relative to their tiers.
*
* The locations are:
*0
*	1
*	2
*	3
*	4
*5
*6
*	7
*	8
*	9
*	10
*11
*/
VocabularyOrdering::VocabularyOrdering(ordering orderType){
	//Initialize the noDistinction and unchanged maps
	for (int i = 0; i < 12; i++)
	{
		noDistinctionMap[i] = NO_DISTINCTION;
		unchangedMap[i] = i;
	}

	switch (orderType)
	{
		/* This ordering is:
			TensorPre
				BasePre
				Base Post
				Base Extra
				*
			TensorPost
				*
				*
				*
				*
			TensorExtra
		*/
	case BASE_1ST_TENSOR_ROOT:
		tensorPreLoc = 0;
		basePreLoc = 1;
		basePostLoc = 2;
		baseExtraLoc = 3;
		tensorPostLoc = 5;
		tensorExtraLoc = 11;
		break;
		/*
			BasePost
				BasePre
				*
				*
				TensorPost
			TensorPre
			TensorExtra
				*
				*
				*
				*
			BaseExtra
		*/
	case BASE_TENSOR_NESTED:
		basePostLoc = 0;
		basePreLoc = 1;
		tensorPostLoc = 4;
		tensorPreLoc = 5;
		tensorExtraLoc = 6;
		baseExtraLoc = 11;
		break;
		/*
			BaseExtra
				TensorPost
				BasePre
				BasePost
				TensorPre
			TensorExtra
			*
				*
				*
				*
				*
			*
		*/
	case PRE_POST_1ST:
		baseExtraLoc = 0;
		tensorPostLoc = 1;
		basePreLoc = 2;
		basePostLoc = 3;
		tensorPreLoc = 4;
		tensorExtraLoc = 5;
		break;
		/*
			BasePre
				BaseExtra
				*
				*
				*
			BasePost
			TensorPre
				TensorExtra
				*
				*
				*
			TensorPost
		*/
	case SEPERATE_NESTED:
		basePreLoc = 0;
		baseExtraLoc = 1;
		basePostLoc = 5;
		tensorPreLoc = 6;
		tensorExtraLoc = 7;
		tensorPostLoc = 11;
		break;
		/*
			*
				BasePre
				BasePost
				BaseExtra
				*
			*
			*
				TensorPre
				TensorPost
				TensorExtra
				*
			*
		*/
	default:
		basePreLoc = 1;
		basePostLoc = 2;
		baseExtraLoc = 3;
		tensorPreLoc = 7;
		tensorPostLoc = 8;
		tensorExtraLoc = 9;
		break;
	}
	createBaseOneTop(orderType);
	createBaseZeroTop(orderType);
	createBaseTopTop(orderType);
	createTensorOneTop(orderType);
	createTensorZeroTop(orderType);
	createTensorTopTop(orderType);
}


void VocabularyOrdering::createBaseOneTop(ordering orderType)
{

	NWAOBDDTopNodeRefPtr v;
	NWAOBDDNodeHandle h;
	ReturnMapHandle<intpair> m;

	h = createBaseOne(NWAOBDDTopNode::maxLevel, orderType);

	m.AddToEnd(intpair(1,1));
	m.AddToEnd(intpair(0,0));
	m.Canonicalize();

	v = new NWAOBDDTopNode(h, m);
	
	baseOne = NWAOBDD(v);
}

void VocabularyOrdering::createBaseTopTop(ordering orderType)
{
	baseTop = MkTrue();
}

void VocabularyOrdering::createBaseZeroTop(ordering orderType)
{
	baseTop = MkFalse();
}

void VocabularyOrdering::createTensorOneTop(ordering orderType)
{
	NWAOBDDTopNodeRefPtr v;
	NWAOBDDNodeHandle h;
	ReturnMapHandle<intpair> m;

	h = createTensorOne(NWAOBDDTopNode::maxLevel, orderType);

	m.AddToEnd(intpair(1,1));
	m.AddToEnd(intpair(0,0));
	m.Canonicalize();

	v = new NWAOBDDTopNode(h, m);
	
	tensorOne = NWAOBDD(v);
}


void VocabularyOrdering::createTensorTopTop(ordering orderType)
{
	tensorTop = MkTrue();
}

void VocabularyOrdering::createTensorZeroTop(ordering orderType)
{
	tensorZero = MkFalse();
}

NWAOBDDNodeHandle VocabularyOrdering::createBaseOne(int level, ordering orderType)
{
	NWAOBDDInternalNode *nLower;
	switch (orderType)
	{
	/* This ordering is:
		TensorPre
			BasePre
			Base Post
			Base Extra
			*
		TensorPost
			*
			*
			*
			*
		TensorExtra
	*/
	case BASE_1ST_TENSOR_ROOT:
		if (level == 0)
		{
			return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
		} 
		else if (level == 2)
		{
			NWAOBDDInternalNode *nUA;
			nLower = new NWAOBDDInternalNode(level);
			nUA = new NWAOBDDInternalNode(level-1);
			ReturnMapHandle<intpair> m00, m01, m10, m11, m00_11;

			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m01.AddToEnd(intpair(0,1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1,0));
			m10.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();

			NWAOBDDNodeHandle temp = createBaseOne(level-2, orderType);
			nUA->AConnection[0] = Connection(temp,m01);
			nUA->AConnection[1] = Connection(temp,m10);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new Connection[nUA->numBConnections];
			nUA->BConnection[1] = new Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUA->numExits = 2;
			
			nLower->AConnection[0] = Connection(NWAOBDDNodeHandle(nUA), m00_11);
			nLower->AConnection[1] = Connection(NWAOBDDNodeHandle(nUA), m00_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new Connection[nLower->numBConnections];
			nLower->BConnection[1] = new Connection[nLower->numBConnections];

			nLower->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			nLower->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

			nLower->numExits = 2;
		} 
		else { //level > 2
			NWAOBDDInternalNode *nUA;
			NWAOBDDInternalNode *nUB;
			nLower = new NWAOBDDInternalNode(level);
			nUA = new NWAOBDDInternalNode(level-1);
			nUB = new NWAOBDDInternalNode(level-1);
			ReturnMapHandle<intpair> m11, m01_11, m10_11, m00_11;

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m01_11.AddToEnd(intpair(0,1));
			m01_11.AddToEnd(intpair(1,1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1,0));
			m10_11.AddToEnd(intpair(1,1));
			m10_11.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();

			NWAOBDDNodeHandle temp = createBaseOne(level-2, orderType);
			nUA->AConnection[0] = Connection(temp,m01_11);
			nUA->AConnection[1] = Connection(temp,m10_11);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new Connection[nUA->numBConnections];
			nUA->BConnection[1] = new Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = Connection(temp, m00_11);
			nUA->BConnection[1][0] = Connection(temp, m00_11);
			nUA->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUA->numExits = 2;
			
			nLower->AConnection[0] = Connection(NWAOBDDNodeHandle(nUA), m00_11);
			nLower->AConnection[1] = Connection(NWAOBDDNodeHandle(nUA), m00_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new Connection[nLower->numBConnections];
			nLower->BConnection[1] = new Connection[nLower->numBConnections];


			nUB->AConnection[0] = Connection(temp,m00_11);
			nUB->AConnection[1] = Connection(temp,m00_11);
			nUB->numBConnections = 2;
			
			nUB->BConnection[0] = new Connection[nUB->numBConnections];
			nUB->BConnection[1] = new Connection[nUB->numBConnections];

			nUB->BConnection[0][0] = Connection(temp, m00_11);
			nUB->BConnection[1][0] = Connection(temp, m00_11);
			nUB->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUB->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUB->numExits = 2;

			nLower->BConnection[0][0] = Connection(NWAOBDDNodeHandle(nUB),m00_11);
			nLower->BConnection[1][0] = Connection(NWAOBDDNodeHandle(nUB),m00_11);
			nLower->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			nLower->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

			nLower->numExits = 2;
		}
		break;
		/*
			BasePost
				BasePre
				*
				*
				TensorPost
			TensorPre
			TensorExtra
				*
				*
				*
				*
			BaseExtra
		*/
	case BASE_TENSOR_NESTED:
		if (level == 0) {
			return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
		}
		else if (level == 2) {
			NWAOBDDInternalNode *nUA;
			nLower = new NWAOBDDInternalNode(level);
			nUA = new NWAOBDDInternalNode(level-1);

			ReturnMapHandle<intpair> m00, m11, m00_11, m11_00;

			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();

			m11_00.AddToEnd(intpair(1,1));
			m11_00.AddToEnd(intpair(0,0));
			m11_00.Canonicalize();

			NWAOBDDNodeHandle temp = createBaseOne(level-2, orderType);
			nUA->AConnection[0] = Connection(temp,m00);
			nUA->AConnection[1] = Connection(temp,m11);

			nUA->numBConnections = 2;

			nUA->BConnection[0] = new Connection[nUA->numBConnections];
			nUA->BConnection[1] = new Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUA->numExits = 2;
			
			nLower->AConnection[0] = Connection(NWAOBDDNodeHandle(nUA), m00_11);
			nLower->AConnection[1] = Connection(NWAOBDDNodeHandle(nUA), m11_00);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new Connection[nLower->numBConnections];
			nLower->BConnection[1] = new Connection[nLower->numBConnections];

			nLower->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			nLower->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

			nLower->numExits = 2;
		}
		else { //level > 2;
			NWAOBDDInternalNode *nUA0;
			NWAOBDDInternalNode *nUA1;
			NWAOBDDInternalNode *nUB;
			nLower = new NWAOBDDInternalNode(level);
			nUA0 = new NWAOBDDInternalNode(level-1);
			nUA1 = new NWAOBDDInternalNode(level-1);
			nUB = new NWAOBDDInternalNode(level-1);

			ReturnMapHandle<intpair> m00, m11, m00_11, m11_00;

			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();

			m11_00.AddToEnd(intpair(1,1));
			m11_00.AddToEnd(intpair(0,0));
			m11_00.Canonicalize();

			NWAOBDDNodeHandle temp = createBaseOne(level-2, orderType);
			nUA0->AConnection[0] = Connection(temp,m00_11);
			nUA0->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA1->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA1->AConnection[1] = Connection(temp,m11_00);

			nUA0->numBConnections = 2;
			nUA1->numBConnections = 2;

			nUA0->BConnection[0] = new Connection[nUA0->numBConnections];
			nUA0->BConnection[1] = new Connection[nUA0->numBConnections];
			nUA1->BConnection[0] = new Connection[nUA1->numBConnections];
			nUA1->BConnection[1] = new Connection[nUA1->numBConnections];

			nUA0->BConnection[0][0] = Connection(temp,m00_11);
			nUA0->BConnection[1][0] = Connection(temp,m00_11);
			nUA0->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA0->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUA1->BConnection[0][1] = Connection(temp,m11_00);
			nUA1->BConnection[1][1] = Connection(temp,m11_00);
			nUA1->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA1->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);

			nUA0->numExits = 2;
			nUA1->numExits = 2;
			
			nLower->AConnection[0] = Connection(NWAOBDDNodeHandle(nUA0), m00_11);
			nLower->AConnection[1] = Connection(NWAOBDDNodeHandle(nUA1), m11_00);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new Connection[nLower->numBConnections];
			nLower->BConnection[1] = new Connection[nLower->numBConnections];

			nUB->AConnection[0] = Connection(temp,m00_11);
			nUB->AConnection[1] = Connection(temp,m00_11);
			nUB->numBConnections = 2;

			nUB->BConnection[0] = new Connection[nUB->numBConnections];
			nUB->BConnection[1] = new Connection[nUB->numBConnections];

			nUB->BConnection[0][0] = Connection(temp, m00_11);
			nUB->BConnection[1][0] = Connection(temp, m00_11);
			nUB->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUB->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUB->numExits = 2;

			nLower->BConnection[0][0] = Connection(nUB, m00_11);
			nLower->BConnection[1][0] = Connection(nUB, m00_11);
			nLower->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			nLower->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

			nLower->numExits = 2;
		}
		break;
		/*
			BaseExtra
				TensorPost
				BasePre
				BasePost
				TensorPre
			TensorExtra
			*
				*
				*
				*
				*
			*
		*/
	case PRE_POST_1ST:
		baseExtraLoc = 0;
		tensorPostLoc = 1;
		basePreLoc = 2;
		basePostLoc = 3;
		tensorPreLoc = 4;
		tensorExtraLoc = 5;
		break;
		/*
			BasePre
				BaseExtra
				*
				*
				*
			BasePost
			TensorPre
				TensorExtra
				*
				*
				*
			TensorPost
		*/
	case SEPERATE_NESTED:
		basePreLoc = 0;
		baseExtraLoc = 1;
		basePostLoc = 5;
		tensorPreLoc = 6;
		tensorExtraLoc = 7;
		tensorPostLoc = 11;
		break;
		/*
			*
				BasePre
				BasePost
				BaseExtra
				*
			*
			*
				TensorPre
				TensorPost
				TensorExtra
				*
			*
		*/
	default:
		basePreLoc = 1;
		basePostLoc = 2;
		baseExtraLoc = 3;
		tensorPreLoc = 7;
		tensorPostLoc = 8;
		tensorExtraLoc = 9;
		break;
	}
}
// The various get functions

NWAOBDDNodeHandle VocabularyOrdering::createTensorOne(int level, ordering orderType)
{
	NWAOBDDInternalNode *nLower;
	switch (orderType)
	{
	/* This ordering is:
		TensorPre
			BasePre
			Base Post
			Base Extra
			*
		TensorPost
			*
			*
			*
			*
		TensorExtra
	*/
	case BASE_1ST_TENSOR_ROOT:
		if (level == 0)
		{
			return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
		} 
		else if (level == 2)
		{
			NWAOBDDInternalNode *nUA;
			nLower = new NWAOBDDInternalNode(level);
			nUA = new NWAOBDDInternalNode(level-1);
			ReturnMapHandle<intpair> m00, m01, m10, m11, m01_11, m10_11;

			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m01.AddToEnd(intpair(0,1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1,0));
			m10.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m01_11.AddToEnd(intpair(0,1));
			m01_11.AddToEnd(intpair(1,1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1,0));
			m10_11.AddToEnd(intpair(1,1));
			m10_11.Canonicalize();

			NWAOBDDNodeHandle temp = createTensorOne(level-2, orderType);
			nUA->AConnection[0] = Connection(temp,m01);
			nUA->AConnection[1] = Connection(temp,m10);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new Connection[nUA->numBConnections];
			nUA->BConnection[1] = new Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUA->numExits = 2;
			
			nLower->AConnection[0] = Connection(NWAOBDDNodeHandle(nUA), m01_11);
			nLower->AConnection[1] = Connection(NWAOBDDNodeHandle(nUA), m10_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new Connection[nLower->numBConnections];
			nLower->BConnection[1] = new Connection[nLower->numBConnections];

			nLower->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			nLower->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

			nLower->numExits = 2;
		} 
		else { //level > 2
			NWAOBDDInternalNode *nUA;
			NWAOBDDInternalNode *nUB;
			nLower = new NWAOBDDInternalNode(level);
			nUA = new NWAOBDDInternalNode(level-1);
			nUB = new NWAOBDDInternalNode(level-1);
			ReturnMapHandle<intpair> m11, m01_11, m10_11, m00_11;

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m01_11.AddToEnd(intpair(0,1));
			m01_11.AddToEnd(intpair(1,1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1,0));
			m10_11.AddToEnd(intpair(1,1));
			m10_11.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();

			NWAOBDDNodeHandle temp = createTensorOne(level-2, orderType);
			nUA->AConnection[0] = Connection(temp,m01_11);
			nUA->AConnection[1] = Connection(temp,m10_11);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new Connection[nUA->numBConnections];
			nUA->BConnection[1] = new Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = Connection(temp, m00_11);
			nUA->BConnection[1][0] = Connection(temp, m00_11);
			nUA->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUA->numExits = 2;
			
			nLower->AConnection[0] = Connection(NWAOBDDNodeHandle(nUA), m01_11);
			nLower->AConnection[1] = Connection(NWAOBDDNodeHandle(nUA), m10_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new Connection[nLower->numBConnections];
			nLower->BConnection[1] = new Connection[nLower->numBConnections];


			nUB->AConnection[0] = Connection(temp,m00_11);
			nUB->AConnection[1] = Connection(temp,m00_11);
			nUB->numBConnections = 2;
			
			nUB->BConnection[0] = new Connection[nUB->numBConnections];
			nUB->BConnection[1] = new Connection[nUB->numBConnections];

			nUB->BConnection[0][0] = Connection(temp, m00_11);
			nUB->BConnection[1][0] = Connection(temp, m00_11);
			nUB->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUB->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUB->numExits = 2;

			nLower->BConnection[0][0] = Connection(NWAOBDDNodeHandle(nUB),m00_11);
			nLower->BConnection[1][0] = Connection(NWAOBDDNodeHandle(nUB),m00_11);
			nLower->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			nLower->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

			nLower->numExits = 2;
		}
		break;
	}
}

NWAOBDD VocabularyOrdering::getBaseZero()
{
	return baseZero;
}

NWAOBDD VocabularyOrdering::getTensorZero()
{
	return tensorZero;
}

NWAOBDD VocabularyOrdering::getBaseOne()
{
	return baseOne;
}

NWAOBDD VocabularyOrdering::getTensorOne()
{
	return tensorOne;
}

void VocabularyOrdering::getUnchangedMap(int destinationMap[12]){
	destinationMap = unchangedMap;
}

void VocabularyOrdering::getNoDistinctionMap(int destinationMap[12]){
	destinationMap = noDistinctionMap;
}

int VocabularyOrdering::getVLoc(vocabulary_t v) {
	switch (v){
	case BASE_PRE:
		return basePreLoc;
		break;
	case BASE_POST:
		return basePostLoc;
		break;
	case BASE_EXTRA:
		return baseExtraLoc;
		break;
	case TENSOR_PRE:
		return tensorPreLoc;
		break;
	case TENSOR_POST:
		return tensorPostLoc;
		break;
	case TENSOR_EXTRA:
		return tensorExtraLoc;
		break;
	}
}

int VocabularyOrdering::getBasePreLoc() {return basePreLoc;}

int VocabularyOrdering::getBasePostLoc() {return basePostLoc;}

int VocabularyOrdering::getBaseExtraLoc() {return baseExtraLoc;}

int VocabularyOrdering::getTensorPreLoc() {return tensorPreLoc;}

int VocabularyOrdering::getTensorPostLoc() {return tensorPostLoc;}

int VocabularyOrdering::getTensorExtraLoc() {return tensorExtraLoc;}
