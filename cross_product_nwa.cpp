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
Hashset<PairProductMapBody> *PairProductMapBody::canonicalPairProductMapBodySet = new Hashset < PairProductMapBody > ;

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
	PairProductMapBodyIterator mi(*this);

	mi.Reset();
	while (!mi.AtEnd()) {
		hvalue = (hvalue + (unsigned int)mi.Current().First()
			+ (unsigned int)mi.Current().Second()) % modsize;
		mi.Next();
	}
	return hvalue;
}

namespace NWA_OBDD {
	std::ostream& operator<< (std::ostream & out, const PairProductMapBody &r)
	{
		out << (List<intpair>&)r;
		return(out);
	}
}

//***************************************************************
// PairProductMapHandle
//***************************************************************


// Default constructor
PairProductMapHandle::PairProductMapHandle()
	: mapContents(new PairProductMapBody)
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
	: mapContents(r.mapContents)
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
	return mapContents->Length();
}

void PairProductMapHandle::AddToEnd(intpair p)
{
	assert(mapContents->refCount <= 1);
	mapContents->AddToEnd(p);
}

bool PairProductMapHandle::Member(intpair p)
{
	PairProductMapBodyIterator mi(*mapContents);

	mi.Reset();
	while (!mi.AtEnd()) {
		if (mi.Current() == p) {
			return true;
		}
		mi.Next();
	}
	return false;
}

int PairProductMapHandle::Lookup(intpair p)
{
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
	std::cerr << "Failure in PairProductMapHandle::Lookup: " << p << " not found" << std::endl;
	return -1;
}

void PairProductMapHandle::Canonicalize()
{
	PairProductMapBody *answerContents;

	answerContents = PairProductMapBody::canonicalPairProductMapBodySet->Lookup(mapContents);
	if (answerContents == NULL) {
		PairProductMapBody::canonicalPairProductMapBodySet->Insert(mapContents);
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
	PairProductMapBodyIterator mi(*mapContents);
	PairProductMapHandle answer;
	intpair p;

	mi.Reset();
	while (!mi.AtEnd()) {
		answer.AddToEnd(intpair(mi.Current().Second(), mi.Current().First()));
		mi.Next();
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

static Hashtable<PairProductMapKey, PairProductMapHandle> *pairProductMapCache = NULL;
static Hashtable<PairProductMapKey, ReturnMapHandle<intpair>> *pairProductRetMapCache1 = NULL;
static Hashtable<PairProductMapKey, ReturnMapHandle<intpair>> *pairProductRetMapCache2 = NULL;

namespace NWA_OBDD {

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
			else {                                                                        // XX, XX
				PairProductMapHandle AMap;
				PairProductMapHandle AMap1;
				PairProductMapHandle AMap2;
				NWAOBDDInternalNode *n;
				int curANode = 0;
				int curExit;
				unsigned int j;
				int b1, b2;

				n = new NWAOBDDInternalNode(n1->level);

				// Perform the pair product of the 0-AConnections and the 1-AConnections and put the resulting
				// pair product return maps into AMap1 and AMap2
				auto A0_entry = PairProduct(
					*(n1->AConnection[0].entryPointHandle),
					*(n2->AConnection[0].entryPointHandle),
					AMap1
					);
				// n->AConnection[0].entryPointHandle = PairProduct(
				// 	*(n1->AConnection[0].entryPointHandle),
				// 	*(n2->AConnection[0].entryPointHandle),
				// 	AMap1
				// 	);
				auto A1_entry = PairProduct(
					*(n1->AConnection[1].entryPointHandle),
					*(n2->AConnection[1].entryPointHandle),
					AMap2
					);
				// n->AConnection[1].entryPointHandle = PairProduct(
				// 	*(n1->AConnection[1].entryPointHandle),
				// 	*(n2->AConnection[1].entryPointHandle),
				// 	AMap2
				// 	);

				ReturnMapHandle<intpair> A0ReturnMap, A1ReturnMap;
				// Fill in n->AConnection.returnMapHandle
				// For each element in AMap1, populate AMap with the intpair for the zero return then the one return
				// Then, for each element in AMap2, populate AMap with the intpair for the zero return then the one return
				PopulateReturnMaps(n1, n2, AMap1, AMap2, A0ReturnMap, A1ReturnMap, AMap, curANode, true, 0, 0);

				//Canonicalize the return maps.
				n->AConnection[0] = Connection(A0_entry, A0ReturnMap);
				n->AConnection[1] = Connection(A1_entry, A1ReturnMap);

				// Perform the appropriate cross products of the BConnections
				j = 0;
				curExit = 0;
				//There's 2 B-Connections for each point in AMap
				n->numBConnections = AMap.Size();
				n->BConnection[0] = new Connection[n->numBConnections];
				n->BConnection[1] = new Connection[n->numBConnections];
				PairProductMapBodyIterator AMapIterator(*AMap.mapContents);
				AMapIterator.Reset();
				//For each point in the AMap
				while (!AMapIterator.AtEnd()) {
					PairProductMapHandle BMap, BMap2;
					//Get the seperate midpoints for n1 and n2
					b1 = AMapIterator.Current().First();
					b2 = AMapIterator.Current().Second();
					//Perform a pair product on the 0th B-Connections that come from b1 and b2 in n1 and n2
					
					auto B0_entry = PairProduct(
						*(n1->BConnection[0][b1].entryPointHandle),
						*(n2->BConnection[0][b2].entryPointHandle),
						BMap
						);
					// n->BConnection[0][j].entryPointHandle = PairProduct(
					// 	*(n1->BConnection[0][b1].entryPointHandle),
					// 	*(n2->BConnection[0][b2].entryPointHandle),
					// 	BMap
					// 	);
					auto B1_entry = PairProduct(
						*(n1->BConnection[1][b1].entryPointHandle),
						*(n2->BConnection[1][b2].entryPointHandle),
						BMap2
						);
					// n->BConnection[1][j].entryPointHandle = PairProduct(
					// 	*(n1->BConnection[1][b1].entryPointHandle),
					// 	*(n2->BConnection[1][b2].entryPointHandle),
					// 	BMap2
					// 	);

					ReturnMapHandle<intpair> B0ReturnMap, B1ReturnMap;
					PopulateReturnMaps(n1, n2, BMap, BMap2, B0ReturnMap, B1ReturnMap, pairProductMapHandle, curExit, false, b1, b2);

					n->BConnection[0][j] = Connection(B0_entry, B0ReturnMap);
					n->BConnection[1][j] = Connection(B1_entry, B1ReturnMap);
					AMapIterator.Next();
					j++;
				}
				n->numExits = curExit;
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
			answer = PairProduct((NWAOBDDInternalNode *)n1.handleContents,
				(NWAOBDDInternalNode *)n2.handleContents,
				pairProductMapHandle
				);
		}
		else { /* n1.handleContents->NodeKind() == NWAOBDD_EPSILON  n2.handleContents->NodeKind() == NWAOBDD_EPSILON */
			// NWAOBDD_EPSILON, NWAOBDD_EPSILON
			pairProductMapHandle.AddToEnd(intpair(0, 0));
			pairProductMapHandle.Canonicalize();
			answer = n1;
		}
		pairProductCache->Insert(PairProductKey(n1,n2),
		 PairProductMemo(answer,pairProductMapHandle));
		answer.Canonicalize();
		return answer;
		}
	}

	void InitPairProductCache()
	{
		std::cout << "Initializing Cache\n";
		pairProductCache = new Hashtable<PairProductKey, PairProductMemo>(40000);
	}

	void DisposeOfPairProductCache()
	{
		pairProductCache = NULL;
	}

	void InitPairProductMapCaches()
	{
		pairProductMapCache = new Hashtable<PairProductMapKey, PairProductMapHandle>(40000);
		pairProductRetMapCache1 = new Hashtable<PairProductMapKey, ReturnMapHandle<intpair>>(40000);
		pairProductRetMapCache2 = new Hashtable<PairProductMapKey, ReturnMapHandle<intpair>>(40000);
	}

	void DisposeOfPairProductMapCaches()
	{
		pairProductMapCache = NULL;
		pairProductRetMapCache1 = NULL;
		pairProductRetMapCache2 = NULL;
	}
}
