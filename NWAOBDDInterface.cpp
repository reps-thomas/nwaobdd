#include "NWAOBDDInterface.h"
#include "nwaobdd.h"

using namespace NWA_OBDD;

/* Initialized the NWAOBDDInterface with the NWAOBDD and the vocabulary Ordering the
* NWAOBDDs are using.
*/
NWAOBDDInterface::NWAOBDDInterface(VocabularyOrdering * vOrdering, NWAOBDD n, bool isTensored)
{
	this->n = n;
	this->vOrdering = vOrdering;
	this->isTensored = isTensored;
}

/* Performs a tensor operation on the NWAOBDD and NI's NWAOBDD */
NWAOBDDInterface * NWAOBDDInterface::Tensor(NWAOBDDInterface NI)
{
	NWAOBDD g;
	NWAOBDDInterface * NI = MoveBaseToTensor(); //ETTODO - make sure this works
	g = MkAnd(this->n,NI.n);
	this->n = g;
	this->isTensored = true;
	return this;
}

/* Performs a transpose of v1 and v2 */
NWAOBDDInterface * NWAOBDDInterface::Transpose(){
	NWAOBDD g;
	int TransposeMap[12];
	CreateTransposeMap(TransposeMap);
	g = AdjustedPathSummary(this->n, TransposeMap);
	this->n = g;
	return this;
}

/* Performs Detensor*/
NWAOBDDInterface * NWAOBDDInterface::Detensor()
{
	NWAOBDD g;
	NWAOBDDInterface * NI = this->SetEqualVocabs(BASE_POST,TENSOR_PRE);
	int DetensorMap[12];
	CreateDetensorMap(DetensorMap);
	g = AdjustedPathSummary(NI->n, DetensorMap);
	this->n = g;
	this->isTensored = false;
	return this;
}

/*Performs DetensorTranspose */
NWAOBDDInterface * NWAOBDDInterface::DetensorTranspose()
{
	NWAOBDD g;
	NWAOBDDInterface * NI = this->SetEqualVocabs(BASE_POST,TENSOR_POST);
	int DetensorTransMap[12];
	CreateDetensorTransposeMap(DetensorTransMap);
	g = AdjustedPathSummary(NI->n, DetensorTransMap);
	this->n = g;
	this->isTensored = false;
	return this;
}

/* Moves the values in BasePre/BasePost/BaseExtra to TensorPre/TensorPost/TensorExtra
* Base vocabularies are empty aftewards
*/
NWAOBDDInterface * NWAOBDDInterface::MoveBaseToTensor()
{
	NWAOBDD g;
	int MoveBaseToTensorMap[12];
	CreateMoveBaseToTensorMap(MoveBaseToTensorMap);
	g = AdjustedPathSummary(this->n, MoveBaseToTensorMap);
	this->n = g;
	return this;
}


NWAOBDDInterface * NWAOBDDInterface::Union(NWAOBDDInterface * that)
{
	if (this->isZero())
		return that;
	if (that->isZero())
		return new NWAOBDDInterface(*this);

	NWAOBDDInterface * ret = new NWAOBDDInterface(this->vOrdering, MkOr(this->n,that->n), this->isTensored);

	return ret;
}

NWAOBDDInterface * NWAOBDDInterface::Compose(NWAOBDDInterface * that)
{
	if (this->isZero() || that->isZero())
	{
		return new NWAOBDDInterface(this->vOrdering, vOrdering->getBaseZero(), false);
	}
	if (this->isOne())
	{
		return that;
	}
	if (that->isOne())
	{
		return new NWAOBDDInterface(*this);
	}

	NWAOBDD c;
	if (!this->isTensored) {
		int BaseRightShiftMap[12];
		CreateBaseRightShiftMap(BaseRightShiftMap);
		NWAOBDD temp1 = AdjustedPathSummary(that->n, BaseRightShiftMap);
		NWAOBDD temp2 = MkAnd(this->n, temp1);
		int BaseRestoreMap[12];
		CreateBaseRestoreMap(BaseRestoreMap);
		c = AdjustedPathSummary(temp2, BaseRestoreMap);
	} else
	{
		int TensorRightShiftMap[12];
		CreateTensorRightShiftMap(TensorRightShiftMap);
		NWAOBDD temp1 = AdjustedPathSummary(that->n, TensorRightShiftMap);
		NWAOBDD temp2 = MkAnd(this->n, temp1);
		int TensorRestoreMap[12];
		CreateTensorRestoreMap(TensorRestoreMap);
		c = AdjustedPathSummary(temp2, TensorRestoreMap);
	}

	NWAOBDDInterface * ret = new NWAOBDDInterface(vOrdering,c,this->isTensored);
	return ret;
}

bool NWAOBDDInterface::isZero()
{
	if (!isTensored)
	{
		return (this->n == this->vOrdering->getBaseZero());
	}
	else
	{
		return (this->n == this->vOrdering->getTensorZero());
	}
}

bool NWAOBDDInterface::isOne()
{
	if (!isTensored)
	{
		return (this->n == this->vOrdering->getBaseOne());
	}
	else
	{
		return (this->n == this->vOrdering->getTensorOne());
	}
}




/*Performs existential quantifications on a list of vocabularies */
NWAOBDDInterface * NWAOBDDInterface::ExistentialQuantification(vocabulary_t vocabs[], int length)
{
	NWAOBDD g;
	int ExistentialMap[12];
	CreateExistentialMap(vocabs,length,ExistentialMap);
	g = AdjustedPathSummary(this->n, ExistentialMap);
	this->n = g;
	return this;
}

//ETTODO
NWAOBDDInterface * NWAOBDDInterface::SetEqualVocabs(vocabulary_t t1, vocabulary_t t2)
{
	return this;
}

/*Creates the map needed for AdjustedPathSummary to perform a transpose operation */
void NWAOBDDInterface::CreateTransposeMap(int newMap[12])
{
	vOrdering->getUnchangedMap(newMap);
	int v1Loc = vOrdering->getBasePreLoc();
	int v2Loc = vOrdering->getBasePostLoc();
	newMap[v1Loc] = v2Loc;
	newMap[v2Loc] = v1Loc;
}

/*Creates the map needed for AdjustedPathSummary to perform a detensor operation */
void NWAOBDDInterface::CreateDetensorMap(int newMap[12])
{
	vOrdering->getNoDistinctionMap(newMap);
	int basePreLoc = vOrdering->getBasePreLoc();
	int basePostLoc = vOrdering->getBasePostLoc();
	int tensorPostLoc = vOrdering->getTensorPostLoc();
	newMap[basePreLoc] = basePreLoc;
	newMap[basePostLoc] = tensorPostLoc;
}

/*Create a DetensorTransposeMap */
void NWAOBDDInterface::CreateDetensorTransposeMap(int newMap[12])
{
	vOrdering->getNoDistinctionMap(newMap);
	int basePreLoc = vOrdering->getBasePreLoc();
	int basePostLoc = vOrdering->getBasePostLoc();
	int tensorPreLoc = vOrdering->getTensorPreLoc();
	newMap[basePreLoc] = basePreLoc;
	newMap[basePostLoc] = tensorPreLoc;
}

/*Create a map to move values in the base vocabularies into the tensored vocabulary*/
void NWAOBDDInterface::CreateMoveBaseToTensorMap(int newMap[12])
{
	vOrdering->getNoDistinctionMap(newMap);
	int basePreLoc = vOrdering->getBasePreLoc();
	int basePostLoc = vOrdering->getBasePostLoc();
	int tensorPreLoc = vOrdering->getTensorPreLoc();
	int tensorPostLoc = vOrdering->getTensorPostLoc();
	newMap[tensorPreLoc] = basePreLoc;
	newMap[tensorPostLoc] = basePostLoc;
}

void NWAOBDDInterface::CreateBaseRightShiftMap(int newMap[12])
{
	vOrdering->getNoDistinctionMap(newMap);
	int basePreLoc = vOrdering->getBasePreLoc();
	int basePostLoc = vOrdering->getBasePostLoc();
	int baseExtraLoc = vOrdering->getBaseExtraLoc();

	newMap[basePostLoc] = basePreLoc;
	newMap[baseExtraLoc] = basePostLoc;
}

void NWAOBDDInterface::CreateTensorRightShiftMap(int newMap[12])
{
	vOrdering->getNoDistinctionMap(newMap);
	int basePreLoc = vOrdering->getBasePreLoc();
	int basePostLoc = vOrdering->getBasePostLoc();
	int baseExtraLoc = vOrdering->getBaseExtraLoc();

	int tensorPreLoc = vOrdering->getTensorPreLoc();
	int tensorPostLoc = vOrdering->getTensorPostLoc();
	int tensorExtraLoc = vOrdering->getTensorExtraLoc();

	newMap[basePostLoc] = basePreLoc;
	newMap[baseExtraLoc] = basePostLoc;

	newMap[tensorPostLoc] = tensorPreLoc;
	newMap[tensorExtraLoc] = tensorPostLoc;
}

void NWAOBDDInterface::CreateBaseRestoreMap(int newMap[12])
{
	vOrdering->getNoDistinctionMap(newMap);
	int basePostLoc = vOrdering->getBasePostLoc();
	int baseExtraLoc = vOrdering->getBaseExtraLoc();

	newMap[basePostLoc] = baseExtraLoc;
}

void NWAOBDDInterface::CreateTensorRestoreMap(int newMap[12])
{
	vOrdering->getNoDistinctionMap(newMap);
	int basePostLoc = vOrdering->getBasePostLoc();
	int baseExtraLoc = vOrdering->getBaseExtraLoc();

	int tensorPostLoc = vOrdering->getTensorPostLoc();
	int tensorExtraLoc = vOrdering->getTensorExtraLoc();

	newMap[basePostLoc] = baseExtraLoc;
	newMap[tensorPostLoc] = tensorExtraLoc;
}

/*Creates a map for existential quantification*/
void NWAOBDDInterface::CreateExistentialMap(vocabulary_t vocabs[], int length, int newMap[12])
{
	vOrdering->getUnchangedMap(newMap);

	for (int i = 0; i < length; i++)
	{
		int cLoc = vOrdering->getVLoc(vocabs[i]);
		newMap[cLoc] = NO_DISTINCTION;
	}
}

	
/* The nonrecursive entry to the AdjustedPathSummary function */
NWAOBDD NWAOBDDInterface::AdjustedPathSummary(NWAOBDD n, int map[12])
{
	NWAOBDD g = AdjustedPathSummaryRecursive(n.root->rootConnection.entryPointHandle, 1, map,0);
	return g;
}

NWAOBDDTopNodeRefPtr NWAOBDDInterface::GetWeight(int map[12], int vals[12], int vLocs[12], int c)
{
	NWAOBDDTopNodeRefPtr n;
	if (map[c] == NO_DISTINCTION)
	{
		n = MkTrueTop();
	}
	else
	{
		int pos = map[c];
		if (vals[pos])
		{
			n = MkDistinction(vLocs[pos]);
		} else {
			n = MkNot(MkDistinction(vLocs[pos]));
		}
	}
	return n;
}


/*
*  This is a way of adjusting the path summary of other 
*
*
*
*
*
*
*
*
*
*
*/
NWAOBDDTopNodeRefPtr NWAOBDDInterface::AdjustedPathSummaryRecursive(NWAOBDDInternalNode * n, int exit, int map[12], int offset)
{
	NWAOBDDTopNodeRefPtr current = MkFalseTop();  //f stores the new NWAOBDD as we build it up, starts as false as it will be a product of Ors  
	if (n->level >= 1)
	{
		int vals[12];  //An array to hold the values of the variables on the incoming and outgoing AConnection and BConnection edges
		int vLocs[12]; //An array to hold the values of the actual location of the variables in the larger NWAOBDD
		unsigned int size = ((unsigned int)((((unsigned int)1) << (n->level + 2)) - (unsigned int)4));//The number of variables in this level of NWAOBDD
		unsigned int size2 = (size - 4)/2; //The size of upper level in the tier
		//The locations in the overall NWAOBDD of the variables in this tier

		//The locations of all the variables in relation to the overall NWAOBDD structure
		vLocs[0] = offset;
		vLocs[1] = offset+1;
		vLocs[2] = offset+(size2/2);
		vLocs[3] = offset+1+(size2/2);
		vLocs[4] = offset-2 + (size/2);
		vLocs[5] = offset-1 + (size/2);
		vLocs[6] = offset + (size/2);
		vLocs[7] = offset + 1 + (size/2);
		vLocs[8] = offset+(size2/2);
		vLocs[9] = offset+1+(size2/2);
		vLocs[10] = offset + size - 2;
		vLocs[11] = offset + size - 1;
		
		//The goal is to find a path from the entry of the node to the given exit of the node
		//First:  Find all the B-Connections of this node which have returns to the given exit

		//Iterate through the B-Connections - For every middle node, there are two B-Connections 
		for (unsigned int i = 0; i < n->numBConnections; i++)  //For every middle node
		{
			for (int j = 0; j < 2; j++)  //Look at the 0 Connection, then the 1 Connection
			{
				vals[6] = j; //We are looking at the path in which the outgoing B-Connection variable is assigned the value of "j"
				ReturnMapHandle<intpair> BMap = n->BConnection[j][i].returnMapHandle; //Get the Return Map of the B connection
				unsigned BMapSize = BMap.mapContents->mapArray.size();
				int bExitVal = 0; //This is the current exit of the B-Connection we are looking at
				for (unsigned sBI = 0; sBI < BMapSize; sBI++)
				{
					intpair bExit = BMap.mapContents->mapArray[sBI];
					if(exit == bExit.First())  //The 0 return of on of the BConnections exits connects to the exit we are looking for
					{
						vals[11] = 0; //Since the 0-return connected, put that value into the array
						//Now we have found one of the upper tier nodes on the path - we must find the values on
						//the path from the entrance to bExitVal - the exit that connects to the first given exit.  These are
						//the 7th through 10th values in the vals array
						NWAOBDDInternalNode * tempB = (NWAOBDDInternalNode*)n->BConnection[j][i].entryPointHandle.handleContents;
						for (unsigned int k = 0; k < tempB->numBConnections; k++) //Now look at each of the B-Connections of the current node
						{
							for (int jB = 0; jB < 2; jB++) //look at the 0 Connection, then the 1 connection
							{
								vals[9] = jB; //Forcing the value of vals[9]
								ReturnMapHandle<intpair> BBMap = tempB->BConnection[jB][k].returnMapHandle; //Get the Return Map of the B connection
								unsigned BBMapSize = BBMap.mapContents->mapArray.size();
								int bbExitVal = 0; //This is the current exit of the B-Connection we are looking at
								for (unsigned sBBI = 0; sBBI < BBMapSize; sBBI++)
								{
									intpair bbExit = BBMap.mapContents->mapArray[sBBI];
									if(bExitVal == bbExit.First())  //The 0 return of on of the BConnections exits connects to the Upper B node's exit we are looking for
									{
										vals[10] = 0;
										//We have enough information to do a recursive call on the B-Connection of this node
										NWAOBDDTopNodeRefPtr BBVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->BConnection[jB][k].entryPointHandle,bbExitVal,map,vLocs[9]+1);
										//We've found a path through a B Connection of the Upper B tier, now we must find all paths from the entrance of the B-Connection to
										//the start of the found B-Connection - which is the kth midpoint
										for (int lB = 0; lB < 2; lB++) //Look at the two A-connections
										{
											vals[7] = lB;
											ReturnMapHandle<intpair> BAMap = tempB->AConnection[lB].returnMapHandle;
											unsigned BAMapSize = BAMap.mapContents->mapArray.size();
											int baExitVal = 0; //This is the current exit of the B-Connection we are looking at
											for (unsigned sBAI = 0; sBAI < BAMapSize; sBAI++)
											{
												intpair baExit = BAMap.mapContents->mapArray[sBAI];
												if (k == baExit.First()) //The 0 reutrn of an exit of the given AConnection connects to the kth midpoint
												{
													vals[8] = 0;
													//This gives us enough information to make the next recursive call on the AConnection of the upper B Node
													NWAOBDDTopNodeRefPtr BAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle,baExitVal,map,vLocs[7]+1);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for(int l = 0; l < 2; l++)
													{
														vals[0] = l;
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														int aExitVal = 0; //This is the current exit of the B-Connection we are looking at
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															if (i == aExit.First()) //The 0 return of an exit of the given A Connection connects to the ith midpoint
															{
																vals[5] = 0;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		int abExitVal = 0; //This is the current exit of the B-Connection we are looking at
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					int aaExitVal = 0; //This is the current exit of the B-Connection we are looking at
																					for (unsigned sAAI = 0; sAAI < AMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					int aaExitVal = 0; //This is the current exit of the B-Connection we are looking at
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															if (i == aExit.Second())
															{
																vals[5] = 1;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		int abExitVal = 0; //This is the current exit of the B-Connection we are looking at
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					int aaExitVal = 0; //This is the current exit of the B-Connection we are looking at
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					int aaExitVal = 0; //This is the current exit of the A-Connection we are looking at
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															aExitVal++;
														}
													}
												}
												if (k == baExit.Second())
												{
													vals[8] = 1;
													//This gives us enough information to make the next recursive call on the AConnection of the upper B Node
													NWAOBDDTopNodeRefPtr BAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle,baExitVal,map,vLocs[7]+1);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for(int l = 0; l < 2; l++)
													{
														vals[0] = l;
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														int aExitVal = 0; //This is the current exit of the A-Connection we are looking at
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															if (i == aExit.First()) //The 0 return of an exit of the given A Connection connects to the ith midpoint
															{
																vals[5] = 0;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		int abExitVal = 0; //This is the current exit of the A-Connection we are looking at
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															if (i == aExit.Second())
															{
																vals[5] = 1;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															
															aExitVal++;
														}
													}
												}
												
												baExitVal++;
											}
										}
									}
									if(bExitVal == bbExit.Second())
									{
										vals[10] = 1;
										NWAOBDDTopNodeRefPtr BBVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->BConnection[jB][k].entryPointHandle,bbExitVal,map,vLocs[9]+1);
																				//We've found a path through a B Connection of the Upper B tier, now we must find all paths from the entrance of the B-Connection to
										//the start of the found B-Connection - which is the kth midpoint
										for (int lB = 0; lB < 2; lB++) //Look at the two A-connections
										{
											vals[7] = lB;
											ReturnMapHandle<intpair> BAMap = tempB->AConnection[lB].returnMapHandle;
											unsigned BAMapSize = BAMap.mapContents->mapArray.size();
											
											int baExitVal = 0;
											for (unsigned sBAI = 0; sBAI < BAMapSize; sBAI++) //Like the previous steps, iterate through the return map of the given A-Connection
											{
												intpair baExit = BAMap.mapContents->mapArray[sBAI];
												if (k == baExit.First()) //The 0 reutrn of an exit of the given AConnection connects to the kth midpoint
												{
													vals[8] = 0;
													//This gives us enough information to make the next recursive call on the AConnection of the upper B Node
													NWAOBDDTopNodeRefPtr BAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle,baExitVal,map,vLocs[7]+1);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for(int l = 0; l < 2; l++)
													{
														vals[0] = l;
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														
														int aExitVal = 0;
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															if (i == aExit.First()) //The 0 return of an exit of the given A Connection connects to the ith midpoint
															{
																vals[5] = 0;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															if (i == aExit.Second())
															{
																vals[5] = 1;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															;
															aExitVal++;
														}
													}
												}
												if (k == baExit.Second())
												{
													vals[8] = 1;
													//This gives us enough information to make the next recursive call on the AConnection of the upper B Node
													NWAOBDDTopNodeRefPtr BAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle,baExitVal,map,vLocs[7]+1);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for(int l = 0; l < 2; l++)
													{
														vals[0] = l;
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														
														int aExitVal = 0;
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															if (i == aExit.First()) //The 0 return of an exit of the given A Connection connects to the ith midpoint
															{
																vals[5] = 0;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															if (i == aExit.Second())
															{
																vals[5] = 1;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															;
															aExitVal++;
														}
													}
												}
												
												baExitVal++;
											}
										}
									}
									
									bbExitVal++;
								}
							}
						}
							//Now look at the AConnections and see which ones have exits into given BConnection node
					}
					if (exit == bExit.Second())
					{
						vals[11] = 1; //Since the 0-return connected, put that value into the array
						//Now we have found one of the upper tier nodes on the path - we must find the values on
						//the path from the entrance to bExitVal - the exit that connects to the first given exit.  These are
						//the 7th through 10th values in the vals array
						NWAOBDDInternalNode * tempB = (NWAOBDDInternalNode*)n->BConnection[j][i].entryPointHandle.handleContents;
						for (unsigned int k = 0; k < tempB->numBConnections; k++) //Now look at each of the B-Connections of the current node
						{
							for (int jB = 0; jB < 2; jB++) //look at the 0 Connection, then the 1 connection
							{
								vals[9] = jB; //Forcing the value of vals[9]
								ReturnMapHandle<intpair> BBMap = tempB->BConnection[jB][k].returnMapHandle; //Get the Return Map of the B connection
								unsigned BBMapSize = BBMap.mapContents->mapArray.size();
								int bbExitVal = 0;
								for (unsigned sBBI = 0; sBBI < BBMapSize; sBBI++)  //Iterate through the return map
								{
									intpair bbExit = BBMap.mapContents->mapArray[sBBI];
									if(bExitVal == bbExit.First())  //The 0 return of on of the BConnections exits connects to the Upper B node's exit we are looking for
									{
										vals[10] = 0;
										//We have enough information to do a recursive call on the B-Connection of this node
										NWAOBDDTopNodeRefPtr BBVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->BConnection[jB][k].entryPointHandle,bbExitVal,map,vLocs[9]+1);
										//We've found a path through a B Connection of the Upper B tier, now we must find all paths from the entrance of the B-Connection to
										//the start of the found B-Connection - which is the kth midpoint
										for (int lB = 0; lB < 2; lB++) //Look at the two A-connections
										{
											vals[7] = lB;
											ReturnMapHandle<intpair> BAMap = tempB->AConnection[lB].returnMapHandle;
											unsigned BAMapSize = BAMap.mapContents->mapArray.size();
											
											int baExitVal = 0;
											for (unsigned sBAI = 0; sBAI < BAMapSize; sBAI++) //Like the previous steps, iterate through the return map of the given A-Connection
											{
												intpair baExit = BAMap.mapContents->mapArray[sBAI];
												if (k == baExit.First()) //The 0 reutrn of an exit of the given AConnection connects to the kth midpoint
												{
													vals[8] = 0;
													//This gives us enough information to make the next recursive call on the AConnection of the upper B Node
													NWAOBDDTopNodeRefPtr BAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle,baExitVal,map,vLocs[7]+1);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for(int l = 0; l < 2; l++)
													{
														vals[0] = l;
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														
														int aExitVal = 0;
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															if (i == aExit.First()) //The 0 return of an exit of the given A Connection connects to the ith midpoint
															{
																vals[5] = 0;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															if (i == aExit.Second())
															{
																vals[5] = 1;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															;
															aExitVal++;
														}
													}
												}
												if (k == baExit.Second())
												{
													vals[8] = 1;
													//This gives us enough information to make the next recursive call on the AConnection of the upper B Node
													NWAOBDDTopNodeRefPtr BAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle,baExitVal,map,vLocs[7]+1);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for(int l = 0; l < 2; l++)
													{
														vals[0] = l;
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														
														int aExitVal = 0;
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															if (i == aExit.First()) //The 0 return of an exit of the given A Connection connects to the ith midpoint
															{
																vals[5] = 0;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															if (i == aExit.Second())
															{
																vals[5] = 1;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															;
															aExitVal++;
														}
													}
												}
												
												baExitVal++;
											}
										}
									}
									if(bExitVal == bbExit.Second())
									{
										vals[10] = 1;
										NWAOBDDTopNodeRefPtr BBVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->BConnection[jB][k].entryPointHandle,bbExitVal,map,vLocs[9]+1);
																				//We've found a path through a B Connection of the Upper B tier, now we must find all paths from the entrance of the B-Connection to
										//the start of the found B-Connection - which is the kth midpoint
										for (int lB = 0; lB < 2; lB++) //Look at the two A-connections
										{
											vals[7] = lB;
											ReturnMapHandle<intpair> BAMap = tempB->AConnection[lB].returnMapHandle;
											unsigned BAMapSize = BAMap.mapContents->mapArray.size();
											
											int baExitVal = 0;
											for (unsigned sBAI = 0; sBAI < BAMapSize; sBAI++) //Like the previous steps, iterate through the return map of the given A-Connection
											{
												intpair baExit = BAMap.mapContents->mapArray[sBAI];
												if (k == baExit.First()) //The 0 reutrn of an exit of the given AConnection connects to the kth midpoint
												{
													vals[8] = 0;
													//This gives us enough information to make the next recursive call on the AConnection of the upper B Node
													NWAOBDDTopNodeRefPtr BAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle,baExitVal,map,vLocs[7]+1);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for(int l = 0; l < 2; l++)
													{
														vals[0] = l;
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														
														int aExitVal = 0;
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															if (i == aExit.First()) //The 0 return of an exit of the given A Connection connects to the ith midpoint
															{
																vals[5] = 0;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															if (i == aExit.Second())
															{
																vals[5] = 1;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															;
															aExitVal++;
														}
													}
												}
												if (k == baExit.Second())
												{
													vals[8] = 1;
													//This gives us enough information to make the next recursive call on the AConnection of the upper B Node
													NWAOBDDTopNodeRefPtr BAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle,baExitVal,map,vLocs[7]+1);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for(int l = 0; l < 2; l++)
													{
														vals[0] = l;
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														
														int aExitVal = 0;
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															if (i == aExit.First()) //The 0 return of an exit of the given A Connection connects to the ith midpoint
															{
																vals[5] = 0;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															if (i == aExit.Second())
															{
																vals[5] = 1;
																//Now we know an A-Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWAOBDDInternalNode * tempA = (NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		vals[3] = jA;
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		
																		int abExitVal = 0;
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			if (aExitVal == abExit.First())
																			{
																				vals[4] = 0;
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//Now check the 1 return of this exit node
																			if (aExitVal == abExit.Second())
																			{
																				vals[4] = 1; //This is now set to a 1-value
																				NWAOBDDTopNodeRefPtr ABVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; k < 2; l++)
																				{
																					vals[1] = lA;
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					
																					int aaExitVal = 0;
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						if (f == aaExit.First()) //The A connection has a 0 return onto f
																						{
																							vals[2] = 0;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Now check if the A connection has a 1 return onto f
																						if (f == aaExit.Second())
																						{
																							vals[2] = 1;
																							//Perform another recursive call
																							NWAOBDDTopNodeRefPtr AAVal = NWAOBDDInterface::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A connection
																			}
																			//increment the iterator
																			
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A connections middle nodes
															}
															;
															aExitVal++;
														}
													}
												}
												
												baExitVal++;
											}
										}
									}
									
									bbExitVal++;
								}
							}
						}
							//Now look at the AConnections and see which ones have exits into given BConnection node
					}
					//The BConnection's 1 return connects to the given exit, so repeat the above with the adjusted value
					;
					bExitVal++;
				}
			}
		}
		return current;
	}
	return MkTrueTop();
}


NWAOBDDTopNodeRefPtr NWAOBDDInterface::AdjustedPathSummaryRecursive(NWAOBDDNodeHandle n, int exit, int map[12], int offset)
{
		NWAOBDDTopNodeRefPtr topNodeMemo;
		//ETTODO insert Memoization
			NWAOBDDTopNodeRefPtr answer = NWAOBDDInterface::AdjustedPathSummaryRecursive((NWAOBDDInternalNode*)n.handleContents, exit, map, offset);
			return answer;
}