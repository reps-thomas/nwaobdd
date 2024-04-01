//
//    Copyright (c) 1999 Thomas W. Reps
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
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unordered_map>
#include "nwaobdd_node.h"
#include "infra/list_T.h"
#include "infra/list_TPtr.h"
#include "infra/intpair.h"
#include "infra/inttriple.h"
#include "cross_product_nwa.h"
//#include "hash_functions.h"

using namespace NWA_OBDD;
// ********************************************************************
// 2-Way Cross Product
// ********************************************************************

//***************************************************************
// PairProductMapBody
//***************************************************************

// Initializations of static members ---------------------------------
Hashset<PairProductMapBody> *PairProductMapBody::canonicalPairProductMapBodySet = new Hashset<PairProductMapBody>(HASHSET_NUM_BUCKETS);

// Constructor
PairProductMapBody::PairProductMapBody()
  : refCount(0), isCanonical(false)
{
}

void PairProductMapBody::IncrRef()
{
  refCount++;    // Warning: Saturation not checked
}

void PairProductMapBody::DecrRef()
{
  if (--refCount == 0) {    // Warning: Saturation not checked
    if (isCanonical) {
      PairProductMapBody::canonicalPairProductMapBodySet->DeleteEq(this);
    }
    delete this;
  }
}

unsigned int PairProductMapBody::Hash(unsigned int modsize)
{
  unsigned int hvalue = 0;

  /*
  PairProductMapBodyIterator mi(*this);

  mi.Reset();
  while (!mi.AtEnd()) {
    hvalue = (997 * (997 * hvalue + (unsigned int)mi.Current().First())
                                  + (unsigned int)mi.Current().Second() ) % modsize;
    mi.Next();
  }
  */

  for (unsigned int i = 0; i < mapArray.size(); i++){
	  hvalue = (997*hvalue + (unsigned int)97*mapArray[i].First() + (unsigned int)mapArray[i].Second()) % modsize;
  }

  return hvalue;
}

void PairProductMapBody::setHashCheck()
{
	unsigned int hvalue = 0;

	for (auto &i : mapArray) {
		hvalue = (117 * (hvalue + 1) + (int)(97 * i.First()) + i.Second());
	}
	hashCheck = hvalue;
}

void PairProductMapBody::AddToEnd(const intpair& y)
{
	mapArray.push_back(y);
}

bool PairProductMapBody::operator==(const PairProductMapBody &o) const
{
	if (mapArray.size() != o.mapArray.size())
		return false;

	for (unsigned int i = 0; i < mapArray.size(); i++){
		if (mapArray[i] != o.mapArray[i])
			return false;
	}
	return true;
}
intpair& PairProductMapBody::operator[](unsigned int i){                       // Overloaded []
	return mapArray[i];
}

unsigned int PairProductMapBody::Size(){
	return (unsigned int)mapArray.size();
}


namespace NWA_OBDD {
std::ostream& operator<< (std::ostream & out, const PairProductMapBody &r)
{
  //out << (List<int>&)r;
	for (unsigned int i = 0; i < r.mapArray.size(); i++)
	{
		out << r.mapArray[i] << " ";
	}
  return(out);
}
}
//***************************************************************
// PairProductMapHandle
//***************************************************************


// Default constructor
PairProductMapHandle::PairProductMapHandle()
  :  mapContents(new PairProductMapBody)
{
  mapContents->IncrRef();
}

// Destructor
PairProductMapHandle::~PairProductMapHandle()
{
  mapContents->DecrRef();
}

// Copy constructor
PairProductMapHandle::PairProductMapHandle(const PairProductMapHandle &r)
  :  mapContents(r.mapContents)
{
  mapContents->IncrRef();
}

// Overloaded assignment
PairProductMapHandle& PairProductMapHandle::operator= (const PairProductMapHandle &r)
{
  if (this != &r)      // don't assign to self!
  {
    PairProductMapBody *temp = mapContents;
    mapContents = r.mapContents;
    mapContents->IncrRef();
    temp->DecrRef();
  }
  return *this;        
}

// Overloaded !=
bool PairProductMapHandle::operator!=(const PairProductMapHandle &r)
{
  return (mapContents != r.mapContents);
}

// Overloaded ==
bool PairProductMapHandle::operator==(const PairProductMapHandle &r)
{
  return (mapContents == r.mapContents);
}

std::ostream& operator<< (std::ostream & out, const PairProductMapHandle &r)
{
  out << "[" << *r.mapContents << "]";
  return(out);
}

unsigned int PairProductMapHandle::Hash(unsigned int modsize)
{
  return ((unsigned int) reinterpret_cast<uintptr_t>(mapContents) >> 2) % modsize;
}

unsigned int PairProductMapHandle::Size()
{
  return mapContents->Size();
}

intpair& PairProductMapHandle::operator[](unsigned int i)
{
	return mapContents->mapArray[i];
}

void PairProductMapHandle::AddToEnd(const intpair& p)
{
  assert(mapContents->refCount <= 1);
  mapContents->AddToEnd(p);
}

bool PairProductMapHandle::Member(intpair& p)
{
	/*
  PairProductMapBodyIterator mi(*mapContents);

  mi.Reset();
  while (!mi.AtEnd()) {
    if (mi.Current() == p) {
      return true;
    }
    mi.Next();
  }
  return false;
  */
	for (auto& i : mapContents->mapArray){
		if (i == p)
			return true;
	}
	return false;
}

int PairProductMapHandle::Lookup(intpair& p)
{
	/*  
	int index = 0;
  PairProductMapBodyIterator mi(*mapContents);

  mi.Reset();
  while (!mi.AtEnd()) {
    if (mi.Current() == p) {
      return index;
    }
    mi.Next();
    index++;
  }
  //std::cerr << "Failure in PairProductMapHandle::Lookup: " << p << " not found" << std::endl;
  std::cout << "Failure in PairProductMapHandle::Lookup: " << p << " not found" << std::endl;
  */
	for (unsigned int i = 0; i < mapContents->mapArray.size(); i++){
		if (mapContents->mapArray[i] == p)
			return i;
	}
  return -1;
}

void PairProductMapHandle::Canonicalize()
{
  PairProductMapBody *answerContents;
  unsigned int hash = PairProductMapBody::canonicalPairProductMapBodySet->GetHash(mapContents);
  answerContents = PairProductMapBody::canonicalPairProductMapBodySet->Lookup(mapContents, hash);
  if (answerContents == NULL) {
    PairProductMapBody::canonicalPairProductMapBodySet->Insert(mapContents, hash);
    mapContents->isCanonical = true;
  }
  else {
    answerContents->IncrRef();
    mapContents->DecrRef();
    mapContents = answerContents;
  }
}

// Create map with reversed entries
PairProductMapHandle PairProductMapHandle::Flip()
{
  //PairProductMapBodyIterator mi(*mapContents);
  PairProductMapHandle answer;
  /*intpair p;

  mi.Reset();
  while (!mi.AtEnd()) {
    answer.AddToEnd(intpair(mi.Current().Second(), mi.Current().First()));
    mi.Next();
  }
  */

  for (auto& i : mapContents->mapArray){
	  intpair p(i.Second(), i.First());
	  answer.AddToEnd(p);
  }
  return answer;
}

//***************************************************************
// PairProductKey
//***************************************************************

// Constructor
PairProductKey::PairProductKey(NWAOBDDNodeHandle nodeHandle1, NWAOBDDNodeHandle nodeHandle2)
	: nodeHandle1(nodeHandle1), nodeHandle2(nodeHandle2)
{
}

// Hash
unsigned int PairProductKey::Hash(unsigned int modsize)
{
	unsigned int hvalue = 0;
	hvalue = (997 * nodeHandle1.Hash(modsize) + nodeHandle2.Hash(modsize)) % modsize;
	return hvalue;
}

// print
std::ostream& PairProductKey::print(std::ostream & out) const
{
	out << "(" << nodeHandle1 << ", " << nodeHandle2 << ")";
	return out;
}

std::ostream& operator<< (std::ostream & out, const PairProductKey &p)
{
	p.print(out);
	return(out);
}


PairProductKey& PairProductKey::operator= (const PairProductKey& i)
{
	if (this != &i)      // don't assign to self!
	{
		nodeHandle1 = i.nodeHandle1;
		nodeHandle2 = i.nodeHandle2;
	}
	return *this;
}

// Overloaded !=
bool PairProductKey::operator!=(const PairProductKey& p)
{
	return (nodeHandle1 != p.nodeHandle1) || (nodeHandle2 != p.nodeHandle2);
}

// Overloaded ==
bool PairProductKey::operator==(const PairProductKey& p)
{
	return (nodeHandle1 == p.nodeHandle1) && (nodeHandle2 == p.nodeHandle2);
}


//***************************************************************
// PairProductMapKey
//***************************************************************

// Constructor
PairProductMapKey::PairProductMapKey(NWAOBDDNodeHandle nodeHandle1, NWAOBDDNodeHandle nodeHandle2, PairProductMapHandle m1, PairProductMapHandle m2, int curANode, int b1, int b2, bool A, PairProductMapHandle PPMap)
	: nodeHandle1(nodeHandle1), nodeHandle2(nodeHandle2), m1(m1), m2(m2), curANode(curANode), b1(b1), b2(b2), A(A), PPMap(PPMap)
{
}

// Hash
unsigned int PairProductMapKey::Hash(unsigned int modsize)
{
	unsigned int hvalue = 0;
	hvalue = (997 * nodeHandle1.Hash(modsize) + nodeHandle2.Hash(modsize) + m1.Hash(modsize) + m2.Hash(modsize) + curANode * 997 + b1 + b2 + PPMap.Hash(modsize)) % modsize;
	return hvalue;
}

// print
std::ostream& PairProductMapKey::print(std::ostream & out) const
{
	out << "(" << nodeHandle1 << ", " << nodeHandle2 << ")";
	return out;
}

std::ostream& operator<< (std::ostream & out, const PairProductMapKey &p)
{
	p.print(out);
	return(out);
}


PairProductMapKey& PairProductMapKey::operator= (const PairProductMapKey& i)
{
	if (this != &i)      // don't assign to self!
	{
		nodeHandle1 = i.nodeHandle1;
		nodeHandle2 = i.nodeHandle2;
		m1 = i.m1;
		m2 = i.m2;
		PPMap = i.PPMap;
		b1 = i.b1;
		b2 = i.b2;
		curANode = i.curANode;
		A = i.A;
	}
	return *this;
}

// Overloaded !=
bool PairProductMapKey::operator!=(const PairProductMapKey& p)
{
	return (nodeHandle1 != p.nodeHandle1) || (nodeHandle2 != p.nodeHandle2) || (m1 != p.m1) || (m2 != p.m2) || (b1 != p.b1) || (b2 != p.b2) || (curANode != p.curANode) || (A != p.A) || (PPMap != p.PPMap);
}

// Overloaded ==
bool PairProductMapKey::operator==(const PairProductMapKey& p)
{
	return (nodeHandle1 == p.nodeHandle1) && (nodeHandle2 == p.nodeHandle2) && (m1 == p.m1) && (m2 == p.m2) && (b1 == p.b1) && (b2 == p.b2) && (curANode == p.curANode) && (A == p.A) && (PPMap == p.PPMap);
}

//***************************************************************
// PairProductMemo
//***************************************************************

// Default constructor
PairProductMemo::PairProductMemo()
	: nodeHandle(NWAOBDDNodeHandle()), pairProductMapHandle(PairProductMapHandle())
{
}

// Constructor
PairProductMemo::PairProductMemo(NWAOBDDNodeHandle nodeHandle, PairProductMapHandle pairProductMapHandle)
	: nodeHandle(nodeHandle), pairProductMapHandle(pairProductMapHandle)
{
}

std::ostream& operator<< (std::ostream & out, const PairProductMemo &p)
{
	out << "(" << p.nodeHandle << ", " << p.pairProductMapHandle << ")";
	return(out);
}

PairProductMemo& PairProductMemo::operator= (const PairProductMemo& i)
{
	if (this != &i)      // don't assign to self!
	{
		nodeHandle = i.nodeHandle;
		pairProductMapHandle = i.pairProductMapHandle;
	}
	return *this;
}

// Overloaded !=
bool PairProductMemo::operator!=(const PairProductMemo& p)
{
	return (nodeHandle != p.nodeHandle) || (pairProductMapHandle != p.pairProductMapHandle);
}

// Overloaded ==
bool PairProductMemo::operator==(const PairProductMemo& p)
{
	return (nodeHandle == p.nodeHandle) && (pairProductMapHandle == p.pairProductMapHandle);
}

// --------------------------------------------------------------------
// PairProduct
//
// Returns a new NWAOBDDNodeHandle, and (in pairProductMapHandle) a descriptor of the
// node's exits
// --------------------------------------------------------------------

static Hashtable<PairProductKey, PairProductMemo> *pairProductCache = NULL;

namespace NWA_OBDD {
	/*
	void PopulateReturnMaps(NWAOBDDInternalNode * n1, NWAOBDDInternalNode * n2, PairProductMapHandle m1, PairProductMapHandle m2, ReturnMapHandle<intpair> & r1, ReturnMapHandle<intpair> & r2, PairProductMapHandle & PPMap, int & curANode, bool A, int b1, int b2)
	{
		PairProductMapHandle cachedPPHandle;
		bool isCached = pairProductMapCache->Fetch(PairProductMapKey(n1, n2, m1, m2, curANode, b1, b2, A, PPMap), cachedPPHandle);
		if (isCached) {
			pairProductRetMapCache1->Fetch(PairProductMapKey(n1, n2, m1, m2, curANode, b1, b2, A, PPMap), r1);
			pairProductRetMapCache2->Fetch(PairProductMapKey(n1, n2, m1, m2, curANode, b1, b2, A, PPMap), r2);
			// std::cout << "Hit : " << handleContents->Level() << std::endl;
			PPMap = cachedPPHandle;
		}
		else {
			PairProductMapBodyIterator PPIt(*PPMap.mapContents);
			PPIt.Reset();
			while (!PPIt.AtEnd())
			{
				cachedPPHandle.AddToEnd(PPIt.Current());
				PPIt.Next();
			}
			PairProductMapBodyIterator MapIterator1(*m1.mapContents);
			MapIterator1.Reset();
			while (!MapIterator1.AtEnd()){
				int c1, c2, d1, d2;
				int index0, index1;
				//Get the returns for the n1 exit point and the n2 exit point
				intpair n1Pts, n2Pts;
				if (A)
				{
					n1Pts = n1->AConnection[0].returnMapHandle.Lookup(MapIterator1.Current().First());
					n2Pts = n2->AConnection[0].returnMapHandle.Lookup(MapIterator1.Current().Second());
				}
				else
				{
					n1Pts = n1->BConnection[0][b1].returnMapHandle.Lookup(MapIterator1.Current().First());
					n2Pts = n2->BConnection[0][b2].returnMapHandle.Lookup(MapIterator1.Current().Second());
				}
				//Get the return map values for the 0 returns and 1 returns for the paired return points
				c1 = n1Pts.First();
				c2 = n2Pts.First();
				d1 = n1Pts.Second();
				d2 = n2Pts.Second();
				//If the combines point exists in AMap, add that point to the new AConnection's return map
				if (cachedPPHandle.Member(intpair(c1, c2))){
					index0 = cachedPPHandle.Lookup(intpair(c1, c2));
				}
				else { //Create a new A-return point
					cachedPPHandle.AddToEnd(intpair(c1, c2));
					index0 = curANode;
					curANode++;
				} //Repeat the steps for the 1 return for the current return pair
				if (cachedPPHandle.Member(intpair(d1, d2))){
					index1 = cachedPPHandle.Lookup(intpair(d1, d2));
				}
				else {
					cachedPPHandle.AddToEnd(intpair(d1, d2));
					index1 = curANode;
					curANode++;
				}
				//Now that we've determined the new 0 and 1 returns for the 0th AConnections
				r1.AddToEnd(intpair(index0, index1));
				MapIterator1.Next();
			}

			//Now continue to populate AMap with return points for the 1-AConnection if 
			//those points don't all ready exist and add them to the new 1-AConnection's return map 
			PairProductMapBodyIterator MapIterator2(*m2.mapContents);
			MapIterator2.Reset();
			while (!MapIterator2.AtEnd()){
				int c1, c2, d1, d2;
				int index0, index1;
				intpair n1Pts, n2Pts;
				//Get the returns for the n1 exit point and the n2 exit point
				if (A)
				{
					n1Pts = n1->AConnection[1].returnMapHandle.Lookup(MapIterator2.Current().First());
					n2Pts = n2->AConnection[1].returnMapHandle.Lookup(MapIterator2.Current().Second());
				}
				else
				{
					n1Pts = n1->BConnection[1][b1].returnMapHandle.Lookup(MapIterator2.Current().First());
					n2Pts = n2->BConnection[1][b2].returnMapHandle.Lookup(MapIterator2.Current().Second());
				}
				//Get the return map values for the 0 returns and 1 returns for the paired return points
				c1 = n1Pts.First();
				c2 = n2Pts.First();
				d1 = n1Pts.Second();
				d2 = n2Pts.Second();
				//If the combines point exists in AMap, add that point to the new AConnection's return map
				if (cachedPPHandle.Member(intpair(c1, c2))){
					index0 = cachedPPHandle.Lookup(intpair(c1, c2));
				}
				else { //Create a new A-return point
					cachedPPHandle.AddToEnd(intpair(c1, c2));
					index0 = curANode;
					curANode++;
				} //Repeat the steps for the 1 return for the current return pair
				if (cachedPPHandle.Member(intpair(d1, d2))){
					index1 = cachedPPHandle.Lookup(intpair(d1, d2));
				}
				else {
					cachedPPHandle.AddToEnd(intpair(d1, d2));
					index1 = curANode;
					curANode++;
				}
				//Now that we've determined the new 0 and 1 returns for the 1th AConnections
				r2.AddToEnd(intpair(index0, index1));
				MapIterator2.Next();
			}

			cachedPPHandle.Canonicalize();
			r1.Canonicalize();
			r2.Canonicalize();

			pairProductMapCache->Insert(PairProductMapKey(n1, n2, m1, m2, curANode, b1, b2, A, PPMap), cachedPPHandle);
			pairProductRetMapCache1->Insert(PairProductMapKey(n1, n2, m1, m2, curANode, b1, b2, A, PPMap), r1);
			pairProductRetMapCache2->Insert(PairProductMapKey(n1, n2, m1, m2, curANode, b1, b2, A, PPMap), r2);
			//Canonicalize the return maps.
			PPMap = cachedPPHandle;
		}
	}

	*/

	//ETTODO - fix returnmap stuff
	NWAOBDDNodeHandle PairProduct(NWAOBDDInternalNode *n1,
		NWAOBDDInternalNode *n2,
		PairProductMapHandle &pairProductMapHandle
		)
	{ //If both are no distinction nodes, then the pair product is a no distinction node
		if (n1 == NWAOBDDNodeHandle::NoDistinctionNode[n1->level].handleContents) {
			if (n2 == NWAOBDDNodeHandle::NoDistinctionNode[n2->level].handleContents) {   // ND, ND
				pairProductMapHandle.AddToEnd(intpair(0, 0));
				pairProductMapHandle.Canonicalize();
				return NWAOBDDNodeHandle(n1);
			}
			else { //If one is a no distinction node, the pair product is the other node with 
				//endpoints paired with the endpoint of the no distinction node
				for (unsigned int kk = 0; kk < n2->numExits; kk++) {
					pairProductMapHandle.AddToEnd(intpair(0, kk));
				}
				pairProductMapHandle.Canonicalize();
				return NWAOBDDNodeHandle(n2);
			}
		}
		else {
			if (n2 == NWAOBDDNodeHandle::NoDistinctionNode[n2->level].handleContents) {   // XX, ND
				for (unsigned int kk = 0; kk < n1->numExits; kk++) {
					pairProductMapHandle.AddToEnd(intpair(kk, 0));
				}
				pairProductMapHandle.Canonicalize();
				return NWAOBDDNodeHandle(n1);
			}
			else {                                                                   // XX, XX
				
				NWAOBDDInternalNode *n;
				n = new NWAOBDDInternalNode(n1->level);

				// Perform the pair product of the 0-AConnections and the 1-AConnections and put the resulting
				// pair product return maps into ARet0 and ARet1
				PairProductMapHandle ARet0;
				NWAOBDDNodeHandle A0_entry = PairProduct(
					*(n1->AConnection[0].entryPointHandle),
					*(n2->AConnection[0].entryPointHandle),
					ARet0 );
				PairProductMapHandle ARet1;
				NWAOBDDNodeHandle A1_entry = PairProduct(
					*(n1->AConnection[1].entryPointHandle),
					*(n2->AConnection[1].entryPointHandle),
					ARet1 );

				ReturnMapHandle<intpair> A0ReturnMap, A1ReturnMap;
				PairProductMapHandle AMap;
				// Fill in n->AConnection.returnMapHandle
				// For each element in ARet0, populate AMap with the intpair for the zero return then the one return
				// Then, for each element in ARet1, populate AMap with the intpair for the zero return then the one return

				// PopulateReturnMaps(n1, n2, ARet0, ARet1, A0ReturnMap, A1ReturnMap, AMap, curANode, true, 0, 0);
				std::unordered_map<intpair, unsigned, intpair::intpair_hash> pair_to_index_a;
				std::vector<intpair> middle_merge; // how we should merge the BConnection part
				unsigned cur_index_a = 0;
				
				// work with AConnection[0] of the result
				for(unsigned i = 0; i < ARet0.Size(); ++i) {
					unsigned fst = ARet0[i].First();
					unsigned snd = ARet0[i].Second();
					unsigned fst_ans, snd_ans;

					unsigned fst0 = n1->AConnection[0].returnMapHandle[fst].First();
					unsigned snd0 = n2->AConnection[0].returnMapHandle[snd].First();
					if(pair_to_index_a.find(intpair(fst0, snd0)) == pair_to_index_a.end()) {
						pair_to_index_a[intpair(fst0, snd0)] = cur_index_a;
						middle_merge.push_back(intpair(fst0, snd0));
						fst_ans = cur_index_a++;
					}
					else fst_ans = pair_to_index_a[intpair(fst0, snd0)];

					unsigned fst1 = n1->AConnection[0].returnMapHandle[fst].Second();
					unsigned snd1 = n2->AConnection[0].returnMapHandle[snd].Second();
					if(pair_to_index_a.find(intpair(fst1, snd1)) == pair_to_index_a.end()) {
						pair_to_index_a[intpair(fst1, snd1)] = cur_index_a;
						middle_merge.push_back(intpair(fst1, snd1));
						snd_ans = cur_index_a++;
					}
					else snd_ans = pair_to_index_a[intpair(fst1, snd1)];

					A0ReturnMap.AddToEnd(intpair(fst_ans, snd_ans));
				}
				// work with AConnection[1] of the result
				for(unsigned i = 0; i < ARet1.Size(); ++i) {
					unsigned fst = ARet1[i].First();
					unsigned snd = ARet1[i].Second();
					unsigned fst_ans, snd_ans;

					unsigned fst0 = n1->AConnection[1].returnMapHandle[fst].First();
					unsigned snd0 = n2->AConnection[1].returnMapHandle[snd].First();
					if(pair_to_index_a.find(intpair(fst0, snd0)) == pair_to_index_a.end()) {
						pair_to_index_a[intpair(fst0, snd0)] = cur_index_a;
						middle_merge.push_back(intpair(fst0, snd0));
						fst_ans = cur_index_a++;
					}
					else fst_ans = pair_to_index_a[intpair(fst0, snd0)];

					unsigned fst1 = n1->AConnection[1].returnMapHandle[fst].Second();
					unsigned snd1 = n2->AConnection[1].returnMapHandle[snd].Second();
					if(pair_to_index_a.find(intpair(fst1, snd1)) == pair_to_index_a.end()) {
						pair_to_index_a[intpair(fst1, snd1)] = cur_index_a;
						middle_merge.push_back(intpair(fst1, snd1));
						snd_ans = cur_index_a++;
					}
					else snd_ans = pair_to_index_a[intpair(fst1, snd1)];

					A1ReturnMap.AddToEnd(intpair(fst_ans, snd_ans));
				}

				//Canonicalize the return maps.
				A0ReturnMap.Canonicalize();
				A1ReturnMap.Canonicalize();
				n->AConnection[0] = Connection(A0_entry, A0ReturnMap);
				n->AConnection[1] = Connection(A1_entry, A1ReturnMap);

				n -> numBConnections = cur_index_a;
				n->BConnection[0] = new Connection[n->numBConnections];
				n->BConnection[1] = new Connection[n->numBConnections];

				std::unordered_map<intpair, unsigned, intpair::intpair_hash> pair_to_index_b;
				unsigned cur_index_b = 0;
				for(unsigned id = 0; id < middle_merge.size(); ++id) {
					intpair ip = middle_merge[id];
					PairProductMapHandle BRet0;
					NWAOBDDNodeHandle B0_entry = PairProduct(
						*(n1->BConnection[0][ip.First()].entryPointHandle),
						*(n2->BConnection[0][ip.Second()].entryPointHandle),
						BRet0 );
				
					PairProductMapHandle BRet1;
					NWAOBDDNodeHandle B1_entry = PairProduct(
						*(n1->BConnection[1][ip.First()].entryPointHandle),
						*(n2->BConnection[1][ip.Second()].entryPointHandle),
						BRet1 );

					ReturnMapHandle<intpair> B0ReturnMap, B1ReturnMap;
					// work with BConnection[0]["ip"] of the result
					for(unsigned i = 0; i < BRet0.Size(); ++i) {
						unsigned fst = BRet0[i].First();
						unsigned snd = BRet0[i].Second();
						unsigned fst_ans, snd_ans;

						unsigned fst0 = n1->BConnection[0][ip.First()].returnMapHandle[fst].First();
						unsigned snd0 = n2->BConnection[0][ip.Second()].returnMapHandle[snd].First();
						if(pair_to_index_b.find(intpair(fst0, snd0)) == pair_to_index_b.end()) {
							pair_to_index_b[intpair(fst0, snd0)] = cur_index_b;
							pairProductMapHandle.AddToEnd(intpair(fst0, snd0));
							fst_ans = cur_index_b++;
						}
						else fst_ans = pair_to_index_b[intpair(fst0, snd0)];

						unsigned fst1 = n1->BConnection[0][ip.First()].returnMapHandle[fst].Second();
						unsigned snd1 = n2->BConnection[0][ip.Second()].returnMapHandle[snd].Second();
						if(pair_to_index_b.find(intpair(fst1, snd1)) == pair_to_index_b.end()) {
							pair_to_index_b[intpair(fst1, snd1)] = cur_index_b;
							pairProductMapHandle.AddToEnd(intpair(fst1, snd1));
							snd_ans = cur_index_b++;
						}
						else snd_ans = pair_to_index_b[intpair(fst1, snd1)];

						B0ReturnMap.AddToEnd(intpair(fst_ans, snd_ans));
					}
					// work with BConnection[1]["ip"] of the result
					for(unsigned i = 0; i < BRet1.Size(); ++i) {
						unsigned fst = BRet1[i].First();
						unsigned snd = BRet1[i].Second();
						unsigned fst_ans, snd_ans;

						unsigned fst0 = n1->BConnection[1][ip.First()].returnMapHandle[fst].First();
						unsigned snd0 = n2->BConnection[1][ip.Second()].returnMapHandle[snd].First();
						if(pair_to_index_b.find(intpair(fst0, snd0)) == pair_to_index_b.end()) {
							pair_to_index_b[intpair(fst0, snd0)] = cur_index_b;
							pairProductMapHandle.AddToEnd(intpair(fst0, snd0));
							fst_ans = cur_index_b++;
						}
						else fst_ans = pair_to_index_b[intpair(fst0, snd0)];

						unsigned fst1 = n1->BConnection[1][ip.First()].returnMapHandle[fst].Second();
						unsigned snd1 = n2->BConnection[1][ip.Second()].returnMapHandle[snd].Second();
						if(pair_to_index_b.find(intpair(fst1, snd1)) == pair_to_index_b.end()) {
							pair_to_index_b[intpair(fst1, snd1)] = cur_index_b;
							pairProductMapHandle.AddToEnd(intpair(fst1, snd1));
							snd_ans = cur_index_b++;
						}
						else snd_ans = pair_to_index_b[intpair(fst1, snd1)];

						B1ReturnMap.AddToEnd(intpair(fst_ans, snd_ans));
					}

					//Canonicalize the return maps.

					B0ReturnMap.Canonicalize();
					B1ReturnMap.Canonicalize();

					n->BConnection[0][id] = Connection(B0_entry, B0ReturnMap);
					n->BConnection[1][id] = Connection(B1_entry, B1ReturnMap);
				}
				
				n->numExits = cur_index_b;
#ifdef PATH_COUNTING_ENABLED
				n->InstallPathCounts();
#endif
				pairProductMapHandle.Canonicalize();
				return NWAOBDDNodeHandle(n);
			}
		}
	}

	NWAOBDDNodeHandle PairProduct(NWAOBDDNodeHandle n1,
		NWAOBDDNodeHandle n2,
		PairProductMapHandle &pairProductMapHandle
		)
	{
		PairProductMemo cachedPairProductMemo;
		bool isCached = pairProductCache->Fetch(PairProductKey(n1,n2), cachedPairProductMemo);
		if (isCached) {
		pairProductMapHandle = cachedPairProductMemo.pairProductMapHandle;
		return cachedPairProductMemo.nodeHandle;
		}
		else if (pairProductCache->Fetch(PairProductKey(n2,n1), cachedPairProductMemo)) {
		pairProductMapHandle = cachedPairProductMemo.pairProductMapHandle.Flip();
		return cachedPairProductMemo.nodeHandle;
		}
		else {
			NWAOBDDNodeHandle answer;
			if (n1.handleContents->NodeKind() == NWAOBDD_INTERNAL) {
				// Internal, Internal
				answer = PairProduct((NWAOBDDInternalNode *)n1.handleContents,
					(NWAOBDDInternalNode *)n2.handleContents,
					pairProductMapHandle
					);
			}
			else {
				// Epsilon, Epsilon
				pairProductMapHandle.AddToEnd(intpair(0, 0));
				pairProductMapHandle.Canonicalize();
				answer = n1;
			}
			pairProductCache->Insert(PairProductKey(n1,n2), PairProductMemo(answer,pairProductMapHandle));
			// answer.Canonicalize();
			return answer;
		}
	}

	void InitPairProductCache()
	{
		// std::cout << "Initializing Cache\n";
		pairProductCache = new Hashtable<PairProductKey, PairProductMemo>(40000);
	}

	void DisposeOfPairProductCache()
	{
		delete pairProductCache;
		pairProductCache = NULL;
	}
}
