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
#include <cstdarg>
#include "nwaobdd_node.h"
#include "infra/list_T.h"
#include "infra/list_TPtr.h"
#include "infra/intpair.h"
#include "infra/conscell.h"
#include "assignment.h"
#include "bool_op.h"
#include "return_map_T.h"
#include "reduction_map.h"
#include "cross_product_nwa.h"
#include "traverse_state_nwa.h"
#include "infra/hash.h"
#include "infra/hashset.h"
#include "nwaobdd_top_node.h"
#include <map>
#include <algorithm>
#include <unordered_map>

using namespace NWA_OBDD;

//********************************************************************
// NWAOBDDNodeHandle
//
// Contains a canonical NWAOBDDNode*
//********************************************************************

// Initializations of static members ---------------------------------

Hashset<NWAOBDDNode> *NWAOBDDNodeHandle::canonicalNodeTable = new Hashset<NWAOBDDNode>(HASHSET_NUM_BUCKETS);
NWAOBDDNodeHandle *NWAOBDDNodeHandle::NoDistinctionNode = NULL;
NWAOBDDNodeHandle NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;

// InitNoDistinctionTable
//
// Create and record the "NoDistinctionNodes" (one per level).
// As a side effect, these nodes are entered in the canonicalNodeTable.
//

void NWAOBDDNodeHandle::InitNoDistinctionTable()
{
  NoDistinctionNode = new NWAOBDDNodeHandle [NWAOBDDMaxLevel+1];
  for (unsigned int i = 0; i <= NWAOBDDMaxLevel; i++) {
    if (i == 0) {
      NoDistinctionNode[0] = NWAOBDDNodeHandle(new NWAOBDDEpsilonNode());
    }
    else {
      NWAOBDDInternalNode *n;
      ReturnMapHandle<intpair> m1;

      n = new NWAOBDDInternalNode(i);
      n->AConnection[0].entryPointHandle = &(NoDistinctionNode[i-1]);
      n->AConnection[1].entryPointHandle = &(NoDistinctionNode[i-1]);
      m1.AddToEnd(intpair(0,0));
      m1.Canonicalize();
      n->AConnection[0].returnMapHandle = m1;
      n->AConnection[1].returnMapHandle = m1;
  
      n->numBConnections = 1;
	  n->BConnection[0] = new Connection[n->numBConnections];
	  n->BConnection[1] = new Connection[n->numBConnections];
      n->BConnection[0][0].entryPointHandle = &(NoDistinctionNode[i-1]);
      n->BConnection[1][0].entryPointHandle = &(NoDistinctionNode[i-1]);
      n->BConnection[0][0].returnMapHandle = m1;
      n->BConnection[1][0].returnMapHandle = m1;
      n->numExits = 1;
#ifdef PATH_COUNTING_ENABLED
      n->InstallPathCounts();
#endif
      NoDistinctionNode[i] = NWAOBDDNodeHandle(n);
    }
  }

  NWAOBDDEpsilonNodeHandle = NoDistinctionNode[0];

  return;
} // InitNoDistinctionTable


// Constructors/Destructor -------------------------------------------

// Default constructor
NWAOBDDNodeHandle::NWAOBDDNodeHandle()
  : handleContents(NULL)
{
}

// Constructor
//
// Construct and canonicalize
//
NWAOBDDNodeHandle::NWAOBDDNodeHandle(NWAOBDDNode *n)
  : handleContents(n)
{
  assert(n != NULL);
  handleContents->IncrRef();
  Canonicalize();
}

// Copy constructor
NWAOBDDNodeHandle::NWAOBDDNodeHandle(const NWAOBDDNodeHandle &c)
{
  handleContents = c.handleContents;
  if (handleContents != NULL) {
    handleContents->IncrRef();
  }
}

NWAOBDDNodeHandle::~NWAOBDDNodeHandle()
{
  if (handleContents != NULL) {
    handleContents->DecrRef();
  }
}

// Hash
unsigned int NWAOBDDNodeHandle::Hash(unsigned int modsize)
{
  return ((unsigned int) reinterpret_cast<uintptr_t>(handleContents) >> 2) % modsize;
}

// Overloaded !=
bool NWAOBDDNodeHandle::operator!= (const NWAOBDDNodeHandle & C)
{
  return handleContents != C.handleContents;
}

// Overloaded ==
bool NWAOBDDNodeHandle::operator== (const NWAOBDDNodeHandle & C)
{
  return handleContents == C.handleContents;
}

// Overloaded assignment
NWAOBDDNodeHandle & NWAOBDDNodeHandle::operator= (const NWAOBDDNodeHandle &c)
{
  if (this != &c)      // don't assign to self!
  {
    NWAOBDDNode *temp = handleContents;
    handleContents = c.handleContents;
    if (handleContents != NULL) {
      handleContents->IncrRef();
    }
    if (temp != NULL) {
      temp->DecrRef();
    }
  }
  return *this;        
}

// Reduce and its associated cache ----------------------------------

static Hashtable<NWAReduceKey, NWAOBDDNodeHandle> *reduceCache = NULL;


NWAOBDDNodeHandle NWAOBDDNodeHandle::Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits, bool forceReduce)
{
  if (replacementNumExits == 1 && !forceReduce) {
    return NWAOBDDNodeHandle::NoDistinctionNode[handleContents->Level()];
  }

  if (redMapHandle.mapContents->isIdentityMap && !forceReduce) {
    return *this;
  }

  NWAOBDDNodeHandle cachedNodeHandle;
  bool isCached = reduceCache->Fetch(NWAReduceKey(*this, redMapHandle), cachedNodeHandle);
  if (isCached) {
    // std::cout << "Hit : " << handleContents->Level() << std::endl;
    return cachedNodeHandle;
  }
  else {
    // std::cout << "Miss: " << handleContents->Level() << std::endl;
    NWAOBDDNodeHandle temp = handleContents->Reduce(redMapHandle, replacementNumExits, forceReduce);
    reduceCache->Insert(NWAReduceKey(*this, redMapHandle), temp);
    return temp;
  }
}

void NWAOBDDNodeHandle::InitReduceCache()
{
  reduceCache = new Hashtable<NWAReduceKey, NWAOBDDNodeHandle>(HASHSET_NUM_BUCKETS);
}

void NWAOBDDNodeHandle::DisposeOfReduceCache()
{
  reduceCache = NULL;
}


// Canonicalization --------------------------------------------
void NWAOBDDNodeHandle::Canonicalize()
{
  NWAOBDDNode *answerContents;

  if (!handleContents->isCanonical) {
    answerContents = canonicalNodeTable->Lookup(handleContents);
    if (answerContents == NULL) {
      canonicalNodeTable->Insert(handleContents);
      handleContents->isCanonical = true;
    }
    else {
      answerContents->IncrRef();
      handleContents->DecrRef();
      handleContents = answerContents;
    }
  }
}

// print
std::ostream& NWAOBDDNodeHandle::print(std::ostream & out) const
{
  out << *handleContents << std::endl;
  return out;
}

namespace NWA_OBDD {

std::ostream& operator<< (std::ostream & out, const NWAOBDDNodeHandle &d)
{
  d.print(out);
  return(out);
}

NWAOBDDNodeHandle MkDistinction(unsigned int level, unsigned int i)
{
	if (level == 0) {
		return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
	}
	else {
		NWAOBDDInternalNode *n = new NWAOBDDInternalNode(level);
		ReturnMapHandle<intpair> m00, m11, m01, m00_11;
		unsigned int size = ((unsigned int)((((unsigned int)1) << (level + 2)) - (unsigned int)4));
		if(i == 0) {
			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			n->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
			n->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);

			n->numBConnections = 2;
			n->BConnection[0] = new Connection[n->numBConnections];
			n->BConnection[1] = new Connection[n->numBConnections];
			n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
			n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
			n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);
			n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);
			n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
            n->InstallPathCounts();
#endif
		} else if (i == ((size/2) - 1)){
			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m01.AddToEnd(intpair(0,1));
			m01.Canonicalize();

			n->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m01);
			n->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m01);

			n->numBConnections = 2;
			n->BConnection[0] = new Connection[n->numBConnections];
			n->BConnection[1] = new Connection[n->numBConnections];
			n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
			n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
			n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);
			n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);
			n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
            n->InstallPathCounts();
#endif
		} else if (i == (size/2)) {
			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			n->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
			n->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);

			n->numBConnections = 1;
			n->BConnection[0] = new Connection[n->numBConnections];
			n->BConnection[1] = new Connection[n->numBConnections];
			n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
			n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);
			n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
            n->InstallPathCounts();
#endif
		} else if (i == (size - 1)) {
			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m01.AddToEnd(intpair(0,1));
			m01.Canonicalize();

			n->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
			n->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);

			n->numBConnections = 1;
			n->BConnection[0] = new Connection[n->numBConnections];
			n->BConnection[1] = new Connection[n->numBConnections];
			n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m01);
			n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m01);
			n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
            n->InstallPathCounts();
#endif
		} else if (i > (size/2)){
			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();

			n->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
			n->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);

			n->numBConnections = 1;

			NWAOBDDNodeHandle temp = MkDistinction(level-1, (i-((size/2) + 1)));
			n->BConnection[0] = new Connection[n->numBConnections];
			n->BConnection[1] = new Connection[n->numBConnections];
			n->BConnection[0][0] = Connection(temp, m00_11);
			n->BConnection[1][0] = Connection(temp, m00_11);
			n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
            n->InstallPathCounts();
#endif
		} else {
			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();

			NWAOBDDNodeHandle temp = MkDistinction(level-1, (i - 1));

			n->AConnection[0] = Connection(temp, m00_11);
			n->AConnection[1] = Connection(temp, m00_11);

			n->numBConnections = 2;
			n->BConnection[0] = new Connection[n->numBConnections];
			n->BConnection[1] = new Connection[n->numBConnections];

			n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
      n->InstallPathCounts();
#endif
		}
		return NWAOBDDNodeHandle(n);
	}
}

NWAOBDDNodeHandle MkIdRelationNested(unsigned int level)
{
    NWAOBDDInternalNode *n;

    if (level == 0) {
        return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
        n->numExits = 1;
    } else if (level == 1) {
        n = new NWAOBDDInternalNode(level);
        ReturnMapHandle<intpair> m01, m10, m11;
        m01.AddToEnd(intpair(0,1));
		m01.Canonicalize();
        
		m10.AddToEnd(intpair(1,0));
        m10.Canonicalize();

		m11.AddToEnd(intpair(1,1));
		m11.Canonicalize();

        NWAOBDDNodeHandle temp = MkIdRelationNested(level-1);

        n->AConnection[0] = Connection(temp,m01); //return map 0: 0; 1: 1
        n->AConnection[1] = Connection(temp,m10); //return map 0: 1; 1: 0
        
        n->numBConnections = 2;
        n->BConnection[0] = new Connection[n->numBConnections];
        n->BConnection[1] = new Connection[n->numBConnections];

        n->BConnection[0][0] = Connection(temp,m01);
        n->BConnection[1][0] = Connection(temp,m10);
        n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
        n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
        
        n->numExits = 2;
    } else { //level > 1
        n = new NWAOBDDInternalNode(level);
        ReturnMapHandle<intpair> m01_11, m10_11, m11;
        m01_11.AddToEnd(intpair(0,1));
		m01_11.AddToEnd(intpair(1,1));
        m01_11.Canonicalize();

		m10_11.AddToEnd(intpair(1,0));
		m10_11.AddToEnd(intpair(1,1));
        m10_11.Canonicalize();

		m11.AddToEnd(intpair(1,1));

        NWAOBDDNodeHandle temp = MkIdRelationNested(level-1);

        n->AConnection[0] = Connection(temp,m01_11);
        n->AConnection[1] = Connection(temp,m10_11);

        n->numBConnections = 2;

        n->BConnection[0] = new Connection[n->numBConnections];
        n->BConnection[1] = new Connection[n->numBConnections];

        n->BConnection[0][0] = Connection(temp,m01_11);
        n->BConnection[1][0] = Connection(temp,m10_11);
        n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
        n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

        n->numExits = 2;
    }

#ifdef PATH_COUNTING_ENABLED
  n->InstallPathCounts();
#endif
  return NWAOBDDNodeHandle(n);
}

//***************************************************************
// PathSummaryKey
//***************************************************************

// Constructor
PathSummaryKey::PathSummaryKey(NWAOBDDNodeHandle nodeHandle, int exit, int offset)
	: nodeHandle(nodeHandle), exit(exit), offset(offset)
{
}

// Hash
unsigned int PathSummaryKey::Hash(unsigned int modsize)
{
  unsigned int hvalue = 0;
  hvalue = nodeHandle.Hash(modsize) + exit % modsize;
  return hvalue;
}

// print
std::ostream& PathSummaryKey::print(std::ostream & out) const
{
  out << "(" << nodeHandle << ", " << exit << ")";
  return out;
}

std::ostream& operator<< (std::ostream & out, const PathSummaryKey &p)
{
  p.print(out);
  return(out);
}


PathSummaryKey& PathSummaryKey::operator= (const PathSummaryKey& i)
{
  if (this != &i)      // don't assign to self!
  {
    nodeHandle = i.nodeHandle;
    exit = i.exit;
  }
  return *this;        
}

// Overloaded !=
bool PathSummaryKey::operator!=(const PathSummaryKey& p)
{
  return (nodeHandle != p.nodeHandle) || (exit != p.exit);
}

// Overloaded ==
bool PathSummaryKey::operator==(const PathSummaryKey& p)
{
  return (nodeHandle == p.nodeHandle) && (exit == p.exit);
}


/* Return an NWAOBDD Top Node representing the weight on a certain edge
 * The arguments are a Schema, the values of the four outgoing/incoming
 * variables at the current variable, the locations of the variables in
 * the overall cflobdd stucture, and which variable in particular that
 * the weight is being retrieved for.
 */
NWAOBDDTopNodeRefPtr GetWeight(int s[], int vals[], int vLocs[], int c)
{
	NWAOBDDTopNodeRefPtr n;
	if (s[c] = 5) //ETTODO Change this to an enumeration -- Warning "s[c] = 5" is an assignment; should this be "=="?
	{
		n = MkTrueTop();
	}
	else
	{
		int pos = s[c];
		if (vals[pos])
		{
			n = MkDistinction(vLocs[pos]);
		} else {
			n = MkNot(MkDistinction(vLocs[pos]));
		}
	}
	return n;
}

static Hashtable<PathSummaryKey, NWAOBDDTopNodeRefPtr> *pathSummaryCache = NULL;


/*
 * A method which does the memoization step of adjusting the schema of the nwaobdd
 */
NWAOBDDTopNodeRefPtr NWAOBDDNodeHandle::SchemaAdjust(NWAOBDDNodeHandle n, int exit, int s[4], int offset)
{
	NWAOBDDTopNodeRefPtr topNodeMemo;
  bool isCached = pathSummaryCache->Fetch(PathSummaryKey(n,exit,offset), topNodeMemo);
  if (isCached) {
	  return topNodeMemo;
  } else  {
		NWAOBDDTopNodeRefPtr answer = NWAOBDDInternalNode::SchemaAdjust((NWAOBDDInternalNode*)n.handleContents, exit, s, offset);
		pathSummaryCache->Insert(PathSummaryKey(n,exit, offset), answer);
		return answer;
	}
}
NWAOBDDTopNodeRefPtr NWAOBDDNodeHandle::SchemaAdjust(NWAOBDDNodeHandle *n, int exit, int s[4], int offset)
{
	return SchemaAdjust(*n, exit, s, offset);
}

NWAOBDDTopNodeRefPtr NWAOBDDNodeHandle::PathSummary(NWAOBDDNodeHandle n, int exit, int offset)
{
	NWAOBDDTopNodeRefPtr topNodeMemo;
	bool isCached = pathSummaryCache->Fetch(PathSummaryKey(n, exit, offset), topNodeMemo);
	if (isCached) {
		return topNodeMemo;
	}
	else  {
		NWAOBDDTopNodeRefPtr answer = NWAOBDDInternalNode::PathSummary((NWAOBDDInternalNode*)n.handleContents, exit, offset);
		pathSummaryCache->Insert(PathSummaryKey(n, exit, offset), answer);
		return answer;
	}
}
NWAOBDDTopNodeRefPtr NWAOBDDNodeHandle::PathSummary(NWAOBDDNodeHandle *n, int exit, int offset)
{
	return PathSummary(*n, exit, offset);
}

void InitPathSummaryCache()
{
  pathSummaryCache = new Hashtable<PathSummaryKey, NWAOBDDTopNodeRefPtr>(40000);
}

void DisposeOfPathSummaryCache()
{
  pathSummaryCache = NULL;
}

/*
 * A method changing the order of the variables based in a given schema, represented as
 * a integer array of length 4.  It does so by computing a path summary value from entry
 * to exit.
 */
NWAOBDDTopNodeRefPtr NWAOBDDInternalNode::SchemaAdjust(NWAOBDDInternalNode * n, int exit, int s[4], int offset) {
	NWAOBDDTopNodeRefPtr f = MkFalseTop();  //f stores the new NWAOBDD as we build it up, starts as false as it will be a product of Ors 
	if (n->level >= 1) {// n is not an epsilon node 
		int vals[4];  //An array to hold the values of the variables on the incoming and outgoing AConnection and BConnection edges
		int vLocs[4]; //An array to hold the values of the actual location of the variables in the larger NWAOBDD
		unsigned int size = ((unsigned int)((((unsigned int)1) << (n->level + 2)) - (unsigned int)4));  //The number of variables in this level of NWAOBDD
		vLocs[0] = offset;
		vLocs[1] = offset-1 + (size/2);
		vLocs[2] = offset + (size/2);
		vLocs[3] = offset + size - 1;
		//For every B Connection of this node, look for ones that have a return connection into the given exit node
		for (unsigned int i = 0; i < n->numBConnections; i++)  //For every B connection
		{
			for (int j = 0; j < 2; j++)  //Look at the 0 Connection, then the 1 Connection
			{
				vals[2] = j;
				ReturnMapHandle<intpair> BMap = n->BConnection[j][i].returnMapHandle; //Get the Return Map of the B connection
				unsigned bMapSize = BMap.mapContents->mapArray.size();
				for (unsigned sBI = 0; sBI < bMapSize; sBI++)  //Iterator through the return map
				{
					intpair bExit = BMap.mapContents->mapArray[sBI];
					if(exit == bExit.First())  //The 0 return of on of the BConnections exits connects to the given exit
					{
						vals[3] = 0;
						//Now look at the AConnections and see which ones have exits into given BConnection node
						for (int k = 0; k < 2; k++) //Look at the 0 and 1 AConnection
						{
							vals[0] = k;
							ReturnMapHandle<intpair> AMap = n->AConnection[k].returnMapHandle;
							unsigned aMapSize = AMap.mapContents->mapArray.size();
							for (unsigned sAI = 0; sAI < aMapSize; sAI++)
							{
								intpair aExit = AMap.mapContents->mapArray[sAI];
								if (i == aExit.First()) //The 0 return of an exit of the given A connection connects to the given BConnection node
								{
									vals[1] = 0;
									//Get the NWAOBDDS representing the weights of the edges found given by the schema
									NWAOBDDTopNodeRefPtr AEnt = GetWeight(s,vals,vLocs,0);
									NWAOBDDTopNodeRefPtr AExit = GetWeight(s,vals,vLocs,1);
									NWAOBDDTopNodeRefPtr BEnt = GetWeight(s,vals,vLocs,2);
									NWAOBDDTopNodeRefPtr BExit = GetWeight(s,vals,vLocs,3);
									//Get the weights of the AConnection and BConnection found along with their exits
									NWAOBDDTopNodeRefPtr AWeight = NWAOBDDNodeHandle::SchemaAdjust(n->AConnection[k].entryPointHandle,aExit.First(),s,offset+1);
									NWAOBDDTopNodeRefPtr BWeight = NWAOBDDNodeHandle::SchemaAdjust(n->BConnection[j][i].entryPointHandle,bExit.First(),s,offset + size/2 + 1);
																		NWAOBDDTopNodeRefPtr path = MkAnd(MkAnd(AEnt, BEnt),
                      MkAnd(MkAnd(AWeight, BWeight), 
                      MkAnd(AExit, BExit)
                      )
                    );
									f = MkOr(path,f); //Or the found weight with current final weight that's been built up
								}
								// Repeat the above if the 1 return of the current exit of the given A connection connects to the given BConnection node
								if (i == aExit.Second())
								{
									vals[1] = 1;
									NWAOBDDTopNodeRefPtr AEnt = GetWeight(s,vals,vLocs,0);
									NWAOBDDTopNodeRefPtr AExit = GetWeight(s,vals,vLocs,1);
									NWAOBDDTopNodeRefPtr BEnt = GetWeight(s,vals,vLocs,2);
									NWAOBDDTopNodeRefPtr BExit = GetWeight(s,vals,vLocs,3);
									NWAOBDDTopNodeRefPtr AWeight = NWAOBDDNodeHandle::SchemaAdjust(n->AConnection[k].entryPointHandle,aExit.Second(),s,offset+1);
									NWAOBDDTopNodeRefPtr BWeight = NWAOBDDNodeHandle::SchemaAdjust(n->BConnection[j][i].entryPointHandle,bExit.First(),s,offset + size/2 + 1);
									NWAOBDDTopNodeRefPtr path = MkAnd(MkAnd(AEnt, BEnt),
                      MkAnd(MkAnd(AWeight, BWeight), 
                      MkAnd(AExit, BExit)
                      )
                    );
                  //  MkAnd(6, AEnt,AWeight,AExit,BEnt,BWeight,BExit);
									f = MkOr(path,f);
								}
							}
						}
					}
					//The BConnection's 1 return connects to the given exit, so repeat the above with the adjusted value
					if(exit == bExit.Second())
					{
						vals[3] = 1;
						for (int k = 0; k < 2; k++)
						{
							vals[0] = k;
							ReturnMapHandle<intpair> AMap = n->AConnection[k].returnMapHandle;
							unsigned aMapSize = AMap.mapContents->mapArray.size();
							for (unsigned sAI = 0; sAI < aMapSize; sAI++)
							{
								intpair aExit = AMap.mapContents->mapArray[sAI];
								if (i == aExit.First())
								{
									vals[1] = 0;
									NWAOBDDTopNodeRefPtr AEnt = GetWeight(s,vals,vLocs,0);
									NWAOBDDTopNodeRefPtr AExit = GetWeight(s,vals,vLocs,1);
									NWAOBDDTopNodeRefPtr BEnt = GetWeight(s,vals,vLocs,2);
									NWAOBDDTopNodeRefPtr BExit = GetWeight(s,vals,vLocs,3);
									NWAOBDDTopNodeRefPtr AWeight = NWAOBDDNodeHandle::SchemaAdjust(n->AConnection[k].entryPointHandle,aExit.First(),s,offset+1);
									NWAOBDDTopNodeRefPtr BWeight = NWAOBDDNodeHandle::SchemaAdjust(n->BConnection[j][i].entryPointHandle,bExit.Second(),s,offset + size/2 + 1);
									NWAOBDDTopNodeRefPtr path = MkAnd(MkAnd(AEnt, BEnt),
                      MkAnd(MkAnd(AWeight, BWeight), 
                      MkAnd(AExit, BExit)
                      )
                    );
									f = MkOr(path,f);
								}
								if (i == aExit.Second())
								{
									vals[1] = 1;
									NWAOBDDTopNodeRefPtr AEnt = GetWeight(s,vals,vLocs,0);
									NWAOBDDTopNodeRefPtr AExit = GetWeight(s,vals,vLocs,1);
									NWAOBDDTopNodeRefPtr BEnt = GetWeight(s,vals,vLocs,2);
									NWAOBDDTopNodeRefPtr BExit = GetWeight(s,vals,vLocs,3);
									NWAOBDDTopNodeRefPtr AWeight = NWAOBDDNodeHandle::SchemaAdjust(n->AConnection[k].entryPointHandle,aExit.Second(),s,offset+1);
									NWAOBDDTopNodeRefPtr BWeight = NWAOBDDNodeHandle::SchemaAdjust(n->BConnection[j][i].entryPointHandle,bExit.Second(),s,offset + size/2 + 1);
									NWAOBDDTopNodeRefPtr path = MkAnd(MkAnd(AEnt, BEnt),
                      MkAnd(MkAnd(AWeight, BWeight), 
                      MkAnd(AExit, BExit)
                      )
                    );
									f = MkOr(path,f);
								}
							}
						}
					}
				}
			}
		}
		return f;
	} else  { //level = 0, we are at an epsilong node
		return MkTrueTop();
	}
}


/*
*  Performs a path summary over the NWAOBDD for a given node, n, and exit point.
*/
NWAOBDDTopNodeRefPtr NWAOBDDInternalNode::PathSummary(NWAOBDDInternalNode * n, int exit, int offset) {
	NWAOBDDTopNodeRefPtr f = MkFalseTop();  //f stores the new NWAOBDD as we build it up, starts as false as it will be a product of Ors 
	if (n->level >= 1) {// n is not an epsilon node 
		int vals[4];  //An array to hold the values of the variables on the incoming and outgoing AConnection and BConnection edges
		int vLocs[4];
		unsigned int size = ((unsigned int)((((unsigned int)1) << (n->level + 2)) - (unsigned int)4));  //The number of variables in this level of NWAOBDD
		vLocs[0] = offset;
		vLocs[1] = offset - 1 + (size / 2);
		vLocs[2] = offset + (size / 2);
		vLocs[3] = offset + size - 1;
		//For every B Connection of this node, look for ones that have a return connection into the given exit node
		for (unsigned int i = 0; i < n->numBConnections; i++)  //For every B connection
		{
			for (int j = 0; j < 2; j++)  //Look at the 0 Connection, then the 1 Connection
			{
				vals[2] = j;
				ReturnMapHandle<intpair> BMap = n->BConnection[j][i].returnMapHandle; //Get the Return Map of the B connection
				unsigned bMapSize = BMap.mapContents->mapArray.size();
				for (unsigned sBI = 0; sBI < bMapSize; sBI++)  //Iterator through the return map
				{
					bool hit = false;
					intpair bExit = BMap.mapContents->mapArray[sBI];
					if (exit == bExit.First())  //The 0 return of on of the BConnections exits connects to the given exit
					{
						hit = true;
						if (exit == bExit.Second())
						{
							vals[3] = -1;
						}
						else
						{
							vals[3] = 0;
						}
					}
					else if (exit == bExit.Second())
					{
						hit = true;
						vals[3] = 1;
					}
					if (hit)
					{
						//Now look at the AConnections and see which ones have exits into given BConnection node
						for (int k = 0; k < 2; k++) //Look at the 0 and 1 AConnection
						{
							vals[0] = k;
							ReturnMapHandle<intpair> AMap = n->AConnection[k].returnMapHandle;
							unsigned aMapSize = AMap.mapContents->mapArray.size();
							for (unsigned sAI = 0; sAI < aMapSize; sAI++)
							{
								bool hit2 = false;
								intpair aExit = AMap.mapContents->mapArray[sAI];
								if (i == aExit.First()) //The 0 return of an exit of the given A connection connects to the given BConnection node
								{
									hit2 = true;
									if (i == aExit.Second())
									{
										vals[1] = -1;
									}
									else
									{
										vals[1] = 0;
									}
								}
								else if (i == aExit.Second())
								{
									hit2 = true;
									vals[1] = 1;
								}
								if (hit2)
								{
									//Get the NWAOBDDS representing the weights of the edges found given by the schema
									NWAOBDDTopNodeRefPtr path = MkTrueTop();
									if (vals[0] == 0)
										path = MkAnd(path,MkNot(MkDistinction(vLocs[0])));
									else
										path = MkAnd(path,MkDistinction(vLocs[0]));
									
									if (vals[1] == 0)
										path = MkAnd(path, MkNot(MkDistinction(vLocs[1])));
									else if (vals[1] == 1)
										path = MkAnd(path, MkDistinction(vLocs[1]));
									
									if (vals[2] == 0)
										path = MkAnd(path, MkNot(MkDistinction(vLocs[2])));
									else
										path = MkAnd(path, MkDistinction(vLocs[2]));
									
									if (vals[3] == 0)
										path = MkAnd(path, MkNot(MkDistinction(vLocs[3])));
									else if (vals[3] == 1)
										path = MkAnd(path, MkDistinction(vLocs[3]));
									
									path = MkAnd(path, NWAOBDDNodeHandle::PathSummary(n->AConnection[k].entryPointHandle, sAI, vLocs[0] + 1));
									path = MkAnd(path, NWAOBDDNodeHandle::PathSummary(n->BConnection[j][i].entryPointHandle, sBI, vLocs[2] + 1));
									f = MkOr(path, f); //Or the found weight with current final weight that's been built up
								}
							}
						}
					}
				}
			}
		}
		return f;
	}
	else  { //level = 0, we are at an epsilon node
		return MkTrueTop();
	}
}
/*ETTODO

NWAOBDDNodeHandle MkDetensorConstraintInterleaved(unsigned int level)
{
  NWAOBDDInternalNode *n;

  assert(level >= 2);
  if (level == 2) {
    ReturnMapHandle m1, m2, m3, m4;
    m1.AddToEnd(0);
    m1.AddToEnd(1);
    m1.Canonicalize();
    m2.AddToEnd(0);
    m2.Canonicalize();
    m3.AddToEnd(1);
    m3.Canonicalize();
    m4.AddToEnd(1);
    m4.AddToEnd(0);
    m4.Canonicalize();

    NWAOBDDInternalNode *n1 = new NWAOBDDInternalNode(1);
    n1->AConnection = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m2);
    n1->numBConnections = 1;
    n1->BConnection = new Connection[n1->numBConnections];
    n1->BConnection[0] = Connection(NWAOBDDNodeHandle::NWAOBDDForkNodeHandle, m1);
    n1->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
    n1->InstallPathCounts();
#endif
    NWAOBDDNodeHandle nh1(n1);
    
    NWAOBDDInternalNode *n2 = new NWAOBDDInternalNode(1);
    n2->AConnection =  Connection(NWAOBDDNodeHandle::NWAOBDDForkNodeHandle, m1);
    n2->numBConnections = 2;
    n2->BConnection = new Connection[n2->numBConnections];
    n2->BConnection[0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m2);
    n2->BConnection[1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m3);
    n2->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
    n2->InstallPathCounts();
#endif
    NWAOBDDNodeHandle nh2(n2);
    
    n = new NWAOBDDInternalNode(2);
    n->AConnection = Connection(nh1, m1);
    n->numBConnections = 2;
    n->BConnection = new Connection[n->numBConnections];
    n->BConnection[0] = Connection(nh2, m4);    // TWR 12/12/2017: violates the greedy left-to-right invariant for Connections - the 0th connection has to use the identity return map <0,1>, not <1,0>
    n->BConnection[1] = Connection(nh2, m1);
  }
  else {  // Create an appropriate NWAOBDDInternalNode
    n = new NWAOBDDInternalNode(level);
    ReturnMapHandle m1, m2;
    m1.AddToEnd(0);
    m1.AddToEnd(1);
    m1.Canonicalize();
    NWAOBDDNodeHandle temp = MkDetensorConstraintInterleaved(level-1);
    n->AConnection = Connection(temp, m1);

    n->numBConnections = 2;
    n->BConnection = new Connection[n->numBConnections];
    m2.AddToEnd(0);
    m2.Canonicalize();
    n->BConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m2);
    n->BConnection[1] = Connection(temp, m1);
  }
  n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
  n->InstallPathCounts();
#endif
  return NWAOBDDNodeHandle(n);
} // MkDetensorConstraintInterleaved*/

NWAOBDDNodeHandle MkParity(unsigned int level)
{
  if (level == 0) {
    return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
  }
  else if (level == 1) {
	  NWAOBDDInternalNode *n = new NWAOBDDInternalNode(level);
	  ReturnMapHandle<intpair> m01, m10;
	  m01.AddToEnd(intpair(0,1));
	  m10.AddToEnd(intpair(1,0));
	  m01.Canonicalize();
	  m10.Canonicalize();
	  NWAOBDDNodeHandle temp = MkParity(level-1);


	  n->AConnection[0] = Connection(temp,m01);
	  n->AConnection[1] = Connection(temp,m10);

	  n->numBConnections = 2;
	  n->BConnection[0] = new Connection[n->numBConnections];
	  n->BConnection[1] = new Connection[n->numBConnections];

	  n->BConnection[0][0] = Connection(temp,m01);
	  n->BConnection[1][0] = Connection(temp,m10);
	  n->BConnection[0][1] = Connection(temp,m10);
	  n->BConnection[1][1] = Connection(temp,m01);

	  n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
    n->InstallPathCounts();
#endif
    return NWAOBDDNodeHandle(n);
  }
  else {  // Create an appropriate NWAOBDDInternalNode
    NWAOBDDInternalNode *n = new NWAOBDDInternalNode(level);
    ReturnMapHandle<intpair> m01_10, m10_01;
    m01_10.AddToEnd(intpair(0,1));
    m01_10.AddToEnd(intpair(1,0));
    m01_10.Canonicalize();
	m10_01.AddToEnd(intpair(1,0));
	m10_01.AddToEnd(intpair(0,1));
	m10_01.Canonicalize();

    NWAOBDDNodeHandle temp = MkParity(level-1);
    n->AConnection[0] = Connection(temp, m01_10);
	n->AConnection[1] = Connection(temp, m10_01);

    n->numBConnections = 2;
    n->BConnection[0] = new Connection[n->numBConnections];
	n->BConnection[1] = new Connection[n->numBConnections];
    n->BConnection[0][0] = Connection(temp, m01_10);
	n->BConnection[1][0] = Connection(temp, m10_01);
	n->BConnection[0][1] = Connection(temp, m10_01);
	n->BConnection[1][1] = Connection(temp, m01_10);

    n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
    n->InstallPathCounts();
#endif
    return NWAOBDDNodeHandle(n);
  }
} // MkParity
/*
NWAOBDDNodeHandle MkWalshInterleavedNode(unsigned int i)
{
  assert(i >= 1);
  NWAOBDDInternalNode *n = new NWAOBDDInternalNode(i);
  if (i == 1) {  // Base case
    ReturnMapHandle m1, m2;

    m1.AddToEnd(0);
    m1.AddToEnd(1);
    m1.Canonicalize();
    n->AConnection = Connection(NWAOBDDNodeHandle::NWAOBDDForkNodeHandle, m1);

    n->numBConnections = 2;
    n->BConnection = new Connection[n->numBConnections];
    m2.AddToEnd(0);
    m2.Canonicalize();
    n->BConnection[0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m2);
    n->BConnection[1] = Connection(NWAOBDDNodeHandle::NWAOBDDForkNodeHandle, m1);
  }
  else {
    ReturnMapHandle m1, m2;

    m1.AddToEnd(0);
    m1.AddToEnd(1);
    m1.Canonicalize();
    NWAOBDDNodeHandle temp = MkWalshInterleavedNode(i-1);
    n->AConnection = Connection(temp, m1);

    n->numBConnections = 2;
    n->BConnection = new Connection[n->numBConnections];
    n->BConnection[0] = Connection(temp, m1);
    m2.AddToEnd(1);
    m2.AddToEnd(0);
    m2.Canonicalize();
    n->BConnection[1] = Connection(temp, m2);
  }
  n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
  n->InstallPathCounts();
#endif
  return NWAOBDDNodeHandle(n);
} // MkWalshInterleavedNode


NWAOBDDNodeHandle MkInverseReedMullerInterleavedNode(unsigned int i)
{
  assert(i >= 1);
  NWAOBDDInternalNode *n = new NWAOBDDInternalNode(i);
  if (i == 1) {  // Base case
    ReturnMapHandle m1, m2;

    m1.AddToEnd(0);
    m1.AddToEnd(1);
    m1.Canonicalize();
    n->AConnection = Connection(NWAOBDDNodeHandle::NWAOBDDForkNodeHandle, m1);

    n->numBConnections = 2;
    n->BConnection = new Connection[n->numBConnections];
    m2.AddToEnd(2);
    m2.AddToEnd(0);
    m2.Canonicalize();
    n->BConnection[0] = Connection(NWAOBDDNodeHandle::NWAOBDDForkNodeHandle, m1);
    n->BConnection[1] = Connection(NWAOBDDNodeHandle::NWAOBDDForkNodeHandle, m2);
  }
  else {
    ReturnMapHandle m1, m2, m3;

    m1.AddToEnd(0);
    m1.AddToEnd(1);
    m1.AddToEnd(2);
    m1.Canonicalize();
    NWAOBDDNodeHandle temp = MkInverseReedMullerInterleavedNode(i-1);
    n->AConnection = Connection(temp, m1);

    n->numBConnections = 3;
    n->BConnection = new Connection[n->numBConnections];
    n->BConnection[0] = Connection(temp, m1);

    m2.AddToEnd(1);
    m2.Canonicalize();
    n->BConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[i-1], m2);

    m3.AddToEnd(2);
    m3.AddToEnd(1);
    m3.AddToEnd(0);
    m3.Canonicalize();
    n->BConnection[2] = Connection(temp, m3);
  }
  n->numExits = 3;
#ifdef PATH_COUNTING_ENABLED
  n->InstallPathCounts();
#endif
  return NWAOBDDNodeHandle(n);
} // MkInverseReedMullerInterleavedNode


NWAOBDDNodeHandle MkStepOneFourth(unsigned int level)
{
  assert(level >= 1);

  NWAOBDDInternalNode *n = new NWAOBDDInternalNode(level);
  if (level == 1) {
    ReturnMapHandle m1, m2;
    m1.AddToEnd(0);
    m1.AddToEnd(1);
    m1.Canonicalize();
    n->AConnection = Connection(NWAOBDDNodeHandle::NWAOBDDForkNodeHandle, m1);

    n->numBConnections = 2;
    n->BConnection = new Connection[n->numBConnections];
    n->BConnection[0] = Connection(NWAOBDDNodeHandle::NWAOBDDForkNodeHandle, m1);
    m2.AddToEnd(1);
    m2.Canonicalize();
    n->BConnection[1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m2);
  }
  else {  // Create an appropriate NWAOBDDInternalNode
    ReturnMapHandle m1, m2, m3;
    m1.AddToEnd(0);
    m1.AddToEnd(1);
    m1.Canonicalize();
    NWAOBDDNodeHandle temp = MkStepOneFourth(level-1);
    n->AConnection = Connection(temp, m1);

    n->numBConnections = 2;
    n->BConnection = new Connection[n->numBConnections];
    m2.AddToEnd(0);
    m2.Canonicalize();
    n->BConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m2);
    m3.AddToEnd(1);
    m3.Canonicalize();
    n->BConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m3);
  }
  n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
  n->InstallPathCounts();
#endif
  return NWAOBDDNodeHandle(n);
} // MkStepOneFourth
*/
}

//ETTODO
#ifdef ARBITRARY_STEP_FUNCTIONS
NWAOBDDNodeHandle MkStepNode(unsigned int level, unsigned int left, unsigned int middle, unsigned int right)
{
  assert(level <= 5);    // Need LONG ints
  assert(middle == 0 || middle == 1);

  if (level == 0) {
    if (left == 2 || right == 2) {
      return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
    }
    else {
      return NWAOBDDNodeHandle::NWAOBDDForkNodeHandle;
    }
  }
  else {  // Create an appropriate NWAOBDDInternalNode
    NWAOBDDInternalNode *n = new NWAOBDDInternalNode(level);
    int numberOfExits = (left != 0) + (middle != 0) + (right != 0);
    unsigned int a = binQuotient(left, level-1);
    unsigned int b = binQuotient(binRemainder(left, level-1) + (middle * (1<<(1<<(level-1)))) + binRemainder(right, level-1), level-1);
    unsigned int c = binQuotient(right, level-1);

    // Create AConnection -----------------------------------
       int numberOfAExits = (a != 0) + (b != 0) + (c != 0);
       assert(numberOfAExits != 0);
   
       ReturnMapHandle m1;
       for (int i = 0; i < numberOfAExits; i++) {
         m1.AddToEnd(i);
       }
       m1.Canonicalize();
       NWAOBDDNodeHandle temp = MkStepNode(level-1, a, b, c);
       n->AConnection = Connection(temp, m1);

    // Create the BConnections ------------------------------
       n->numBConnections = numberOfAExits;
       n->BConnection = new Connection[n->numBConnections];
       int curConnection = 0;
       if (a != 0) {
         ReturnMapHandle m2;
         m2.AddToEnd(0);  // Connect to first exit
         m2.Canonicalize();
         n->BConnection[curConnection++] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m2);
       }
       if (b != 0) {
         unsigned int aa = binRemainder(left, level-1);
         unsigned int cc = binRemainder(right, level-1);
         unsigned int bb = middle;
         ReturnMapHandle m3;
         if (aa != 0) {
           m3.AddToEnd(0);  // Connect to first exit
         }
         if (bb != 0) {
           if (left != 0) {
             m3.AddToEnd(1);
           }
           else {
             m3.AddToEnd(0);
           }
         }
         if (cc != 0) {
           m3.AddToEnd(numberOfExits-1);  // Connect to last exit
         }
         m3.Canonicalize();
         NWAOBDDNodeHandle temp = MkStepNode(level-1, aa, bb, cc);
         n->BConnection[curConnection++] = Connection(temp, m3);
       }
       if (c != 0) {
         ReturnMapHandle m4;
         m4.AddToEnd(numberOfExits-1);  // Connect to last exit
         m4.Canonicalize();
         n->BConnection[curConnection++] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m4);
       }
       n->numExits = numberOfExits;
#ifdef PATH_COUNTING_ENABLED
       n->InstallPathCounts();
#endif
    return NWAOBDDNodeHandle(n);
  }
} // MkStepNode
#endif

//********************************************************************
// NWAReduceKey
//********************************************************************

// Constructor
NWAReduceKey::NWAReduceKey(NWAOBDDNodeHandle nodeHandle, ReductionMapHandle redMapHandle)
  :  nodeHandle(nodeHandle), redMapHandle(redMapHandle)
{
}

//ETTODO good hash functions
// Hash
unsigned int NWAReduceKey::Hash(unsigned int modsize)
{
  unsigned int hvalue = 0;
  hvalue = (997 * nodeHandle.Hash(modsize) + redMapHandle.Hash(modsize)) % modsize;
  return hvalue;
}

// print
std::ostream& NWAReduceKey::print(std::ostream & out) const
{
  out << "(" << nodeHandle << ", " << redMapHandle << ")";
  return out;
}

std::ostream& operator<< (std::ostream & out, const NWAReduceKey &p)
{
  p.print(out);
  return(out);
}

NWAReduceKey& NWAReduceKey::operator= (const NWAReduceKey& i)
{
  if (this != &i)      // don't assign to self!
  {
    nodeHandle = i.nodeHandle;
    redMapHandle = i.redMapHandle;
  }
  return *this;        
}

// Overloaded !=
bool NWAReduceKey::operator!=(const NWAReduceKey& p)
{
  return (nodeHandle != p.nodeHandle) || (redMapHandle != p.redMapHandle);
}

// Overloaded ==
bool NWAReduceKey::operator==(const NWAReduceKey& p)
{
  return (nodeHandle == p.nodeHandle) && (redMapHandle == p.redMapHandle);
}


//********************************************************************
// NWAOBDDNode
//********************************************************************

// Initializations of static members ---------------------------------

unsigned int const NWAOBDDNode::maxLevel = NWAOBDDMaxLevel;

// Constructors/Destructor -------------------------------------------

// Default constructor
NWAOBDDNode::NWAOBDDNode()
  :  level(maxLevel), refCount(0), isCanonical(false)
{
}

// Constructor
NWAOBDDNode::NWAOBDDNode(const unsigned int l)
  :  level(l), refCount(0), isCanonical(false)
{
}

NWAOBDDNode::~NWAOBDDNode()
{
}

// print
namespace NWA_OBDD {
std::ostream& operator<< (std::ostream & out, const NWAOBDDNode &n)
{
  n.print(out);
  return(out);
}
}

//********************************************************************
// NWAOBDDInternalNode
//********************************************************************

// Constructors/Destructor -------------------------------------------

NWAOBDDInternalNode::NWAOBDDInternalNode(const unsigned int l)
  :  NWAOBDDNode(l)
{
}

NWAOBDDInternalNode::~NWAOBDDInternalNode()
{
  delete [] BConnection[0];
  delete [] BConnection[1];
  #ifdef PATH_COUNTING_ENABLED
  if(numPathsToExit)
    delete [] numPathsToExit;
  if(numPathsToMiddle)
    delete [] numPathsToMiddle;
  #endif
}

// print
std::ostream& NWAOBDDInternalNode::print(std::ostream & out) const
{

  unsigned int i, j;
  for (i = level; i < maxLevel; i++) {
    out << "  ";
  }
  out << "A0: " << std::endl;
  out << AConnection[0].entryPointHandle;
  for (i = level; i < maxLevel; i++) {
    out << "  ";
  }
  out << AConnection[0].returnMapHandle;

  for (i = level; i < maxLevel; i++) {
    out << "  ";
  }
  out << "A1: " << std::endl;
  out << AConnection[1].entryPointHandle;
  for (i = level; i < maxLevel; i++) {
    out << "  ";
  }
  out << AConnection[1].returnMapHandle;


  for (j = 0; j < numBConnections; j++) {
    for (i = level; i < maxLevel; i++) {
      out << "  ";
    }
    out << "B[0][" << j << "]:" << std::endl;
    out << BConnection[0][j].entryPointHandle;
    for (i = level; i < maxLevel; i++) {
      out << "  ";
    }
    out << BConnection[0][j].returnMapHandle;

    for (i = level; i < maxLevel; i++) {
      out << "  ";
    }
    out << "B[1][" << j << "]:" << std::endl;
    out << BConnection[1][j].entryPointHandle;
    for (i = level; i < maxLevel; i++) {
      out << "  ";
    }
    out << BConnection[1][j].returnMapHandle;
  }
  return out;
}


void NWAOBDDInternalNode::FillSatisfyingAssignment(unsigned int exitNumber, SH_OBDD::Assignment &assignment, unsigned int &index)
{
  //For each exit of the 0 A-connection
  for (unsigned int i = 0; i < AConnection[0].entryPointHandle->handleContents->numExits; i++) {
	  int ival = AConnection[0].returnMapHandle.Lookup(i).First(); //for each 0 return from that exit
    //For each exit of the 0 B-Connection from that current point
    for (unsigned int j = 0; j < BConnection[0][ival].entryPointHandle->handleContents->numExits; j++) {
		  unsigned int k = BConnection[0][ival].returnMapHandle.Lookup(j).First(); //Get the 0 return from that exit
      if (k == exitNumber) { // Found it
        index--;
        assignment[index] = 0;
            BConnection[0][ival].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
        index--;
        assignment[index] = 0;
        index--;
        assignment[index] = 0;
            AConnection[0].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
        index--;
        assignment[index] = 0;
        return;
      }
	    k = BConnection[0][ival].returnMapHandle.Lookup(j).Second(); //else get the 1 return from that exit
      if (k == exitNumber) { 
        index--;
        assignment[index] = 1;
            BConnection[0][i].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
        index--;
        assignment[index] = 0;
        index--;
        assignment[index] = 0;
            AConnection[0].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
        index--;
        assignment[index] = 0;
        return;
      }
    }
	//Look at the 1-BConnection from our midpoint
    for (unsigned int j = 0; j < BConnection[1][ival].entryPointHandle->handleContents->numExits; j++) {
		unsigned int k = BConnection[1][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[1][ival].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 0;
        AConnection[0].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
	  k = BConnection[1][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[1][i].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 0;
        AConnection[0].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
    }
	ival = AConnection[0].returnMapHandle.Lookup(i).Second();
    for (unsigned int j = 0; j < BConnection[0][ival].entryPointHandle->handleContents->numExits; j++) {
      unsigned int k = BConnection[0][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[0][ival].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 1;
        AConnection[0].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
	  k = BConnection[0][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[0][i].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 1;
        AConnection[0].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
    }
    for (unsigned int j = 0; j < BConnection[1][ival].entryPointHandle->handleContents->numExits; j++) {
      unsigned int k = BConnection[1][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[1][ival].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 1;
        AConnection[0].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
      k = BConnection[1][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[1][i].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 1;
        AConnection[0].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
    }
  }
  for (unsigned int i = 0; i < AConnection[1].entryPointHandle->handleContents->numExits; i++) {
	  int ival = AConnection[1].returnMapHandle.Lookup(i).First(); //for each 0 return
    //For each 0 BConnection coming from a 0 a connection
    for (unsigned int j = 0; j < BConnection[0][ival].entryPointHandle->handleContents->numExits; j++) {
		unsigned int k = BConnection[0][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[0][ival].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 0;
        AConnection[1].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
      k = BConnection[0][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[0][i].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 0;
        AConnection[1].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
    }
    for (unsigned int j = 0; j < BConnection[1][ival].entryPointHandle->handleContents->numExits; j++) {
      unsigned int k = BConnection[1][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[1][ival].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 0;
        AConnection[1].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
      k = BConnection[1][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[1][i].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 0;
        AConnection[1].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
      
        return;
      }
    }
	ival = AConnection[1].returnMapHandle.Lookup(i).Second(); //for each 1 return
    //For each 0 BConnection coming from a 1 a connection
    for (unsigned int j = 0; j < BConnection[0][ival].entryPointHandle->handleContents->numExits; j++) {
		unsigned int k = BConnection[0][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[0][ival].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 1;
        AConnection[1].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
      k = BConnection[0][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[0][i].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 1;
        AConnection[1].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
    }
    for (unsigned int j = 0; j < BConnection[1][ival].entryPointHandle->handleContents->numExits; j++) {
		unsigned int k = BConnection[1][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[1][ival].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 1;
        AConnection[1].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
      k = BConnection[1][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[1][i].entryPointHandle->handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 1;
        AConnection[1].entryPointHandle->handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
    }
  }
  std::cerr << "Failure in NWAOBDDInternalNode::FillSatisfyingAssignment:" << std::endl;
  std::cerr << "  exitNumber = " << exitNumber << std::endl;
  //std::cerr << "  assignment = " << assignment << std::endl; //ETTODO Fix assignment printout
  std::cerr << "  index = " << index << std::endl;
  abort();
}

int NWAOBDDInternalNode::Traverse(SH_OBDD::AssignmentIterator &ai)
{
  int ans;

  if (this == NWAOBDDNodeHandle::NoDistinctionNode[level].handleContents) {
	unsigned int skipLevel = ((unsigned int)((((unsigned int)1) << (level + 2)) - (unsigned int)4));
    ai.Advance(skipLevel);
    ans = 0;
  }
  else {
	bool val = ai.Current();
	ai.Next();
    int i, j, k;
    i = AConnection[val].entryPointHandle->handleContents->Traverse(ai);
	bool val1 = ai.Current();
	ai.Next();
	if(!val1)
		j = AConnection[val].returnMapHandle.Lookup(i).First();
	else
		j = AConnection[val].returnMapHandle.Lookup(i).Second();

	bool val2 = ai.Current();
	ai.Next();
	k = BConnection[val2][j].entryPointHandle->handleContents->Traverse(ai);
	
	bool val3 = ai.Current();
	ai.Next();
    if(!val3)
		ans = BConnection[val2][j].returnMapHandle.Lookup(k).First();
	else
		ans = BConnection[val2][j].returnMapHandle.Lookup(k).Second();
  }
  return ans;
}

ReturnMapHandle<intpair> ComposeAndReduce(ReturnMapHandle<intpair>& mapHandle, ReductionMapHandle& redMapHandle, ReductionMapHandle& inducedRedMapHandle) 
{
  if(redMapHandle.mapContents->isIdentityMap) {
    inducedRedMapHandle = redMapHandle;
    return mapHandle;
  }
  ReturnMapHandle<intpair> answer;
  unsigned siz = mapHandle.mapContents->mapArray.size();

  std::unordered_map<intpair, unsigned, intpair::intpair_hash>reductionMap(siz);
  unsigned cur_idx = 0;
  for(unsigned i = 0; i < siz; ++i) {
    intpair ip = mapHandle[i];
    int fst = redMapHandle.Lookup(ip.First());
    int snd = redMapHandle.Lookup(ip.Second());
    intpair ip_induced(fst, snd);
    if(reductionMap.find(ip_induced) == reductionMap.end()) {
      answer.AddToEnd(ip_induced);
      reductionMap.emplace(ip_induced, cur_idx);
      inducedRedMapHandle.AddToEnd(cur_idx++);
    }
    else {
      inducedRedMapHandle.AddToEnd(reductionMap[ip_induced]);
    }
  }
  answer.Canonicalize();
  inducedRedMapHandle.Canonicalize();
  return answer;
}

struct ConnectionPair {
  const Connection &c1;
  const Connection &c2;
  ConnectionPair(const Connection &cc1, const Connection &cc2)
    : c1(cc1), c2(cc2) {}
  bool operator== (const ConnectionPair &other) const {
    return c1.entryPointHandle->handleContents == other.c1.entryPointHandle->handleContents
        && c1.returnMapHandle.mapContents == other.c1.returnMapHandle.mapContents
        && c2.entryPointHandle->handleContents == other.c2.entryPointHandle->handleContents
        && c2.returnMapHandle.mapContents == other.c2.returnMapHandle.mapContents;
  }
  struct ConnectionPairHash {
    size_t operator() (const ConnectionPair cp) const {
      size_t a1 = reinterpret_cast<size_t>(cp.c1.entryPointHandle->handleContents) >> 3;
      size_t a2 = reinterpret_cast<size_t>(cp.c1.returnMapHandle.mapContents) >> 3;
      size_t a3 = reinterpret_cast<size_t>(cp.c2.entryPointHandle->handleContents) >> 3;
      size_t a4 = reinterpret_cast<size_t>(cp.c2.returnMapHandle.mapContents) >> 3;
      return (a1 + a2 * 997 + a3 * 997 * 997 + a4 * 997 * 997 * 997) % 998244353;
    }
    
  };
};

NWAOBDDNodeHandle NWAOBDDInternalNode::Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits, bool forceReduce)
{
  NWAOBDDInternalNode *n = new NWAOBDDInternalNode(level);
  ReductionMapHandle AReductionMapHandle;        // To record duplicate BConnections

  // Reduce the B connections
  
  n->BConnection[0] = new Connection[numBConnections];   // May create shorter version later
	n->BConnection[1] = new Connection[numBConnections];

  n->numBConnections = 0;
  std::unordered_map<ConnectionPair, unsigned, ConnectionPair::ConnectionPairHash>bconn_table(numBConnections);

  for(unsigned i = 0; i < numBConnections; ++i) {
    ReductionMapHandle inducedReductionMapHandle0(redMapHandle.Size());
    ReductionMapHandle inducedReductionMapHandle1(redMapHandle.Size());

    ReturnMapHandle<intpair> inducedReturnMap0;
    ReturnMapHandle<intpair> inducedReturnMap1;

    inducedReturnMap0 = ComposeAndReduce(BConnection[0][i].returnMapHandle, redMapHandle, inducedReductionMapHandle0);
    NWAOBDDNodeHandle temp0 = BConnection[0][i].entryPointHandle->Reduce(inducedReductionMapHandle0, inducedReturnMap0.Size(), forceReduce);
    inducedReturnMap1 = ComposeAndReduce(BConnection[1][i].returnMapHandle, redMapHandle, inducedReductionMapHandle1);
    NWAOBDDNodeHandle temp1 = BConnection[1][i].entryPointHandle->Reduce(inducedReductionMapHandle1, inducedReturnMap1.Size(), forceReduce);

    Connection c0(temp0, inducedReturnMap0);
    Connection c1(temp1, inducedReturnMap1);
    // unsigned int position = n->InsertBConnection(n->numBConnections, c0, c1);
    unsigned position;
    ConnectionPair cp(c0, c1);
    if(bconn_table.find(cp) == bconn_table.end()) {
      bconn_table[cp] = n->numBConnections;
      position = (n->numBConnections)++;
      n->BConnection[0][position] = c0;
      n->BConnection[1][position] = c1;
    }
    else position = bconn_table[cp];
    
    AReductionMapHandle.AddToEnd(position);
  }
  AReductionMapHandle.Canonicalize();
  if (n->numBConnections < numBConnections) {  // Shorten
    Connection *ntemp0 = n->BConnection[0];
    Connection *ntemp1 = n->BConnection[1];
    n->BConnection[0] = new Connection[n->numBConnections];
    n->BConnection[1] = new Connection[n->numBConnections];
    for (unsigned int j = 0; j < n->numBConnections; j++) {
      n->BConnection[0][j] = ntemp0[j];
      n->BConnection[1][j] = ntemp1[j];
    }
    delete [] ntemp0;
    delete [] ntemp1;
  }
  ReductionMapHandle inducedA0ReductionMapHandle;
  ReductionMapHandle inducedA1ReductionMapHandle;
  ReturnMapHandle<intpair> inducedA0ReturnMap;
  ReturnMapHandle<intpair> inducedA1ReturnMap;

  inducedA0ReturnMap = ComposeAndReduce(AConnection[0].returnMapHandle, AReductionMapHandle, inducedA0ReductionMapHandle);
  inducedA1ReturnMap = ComposeAndReduce(AConnection[1].returnMapHandle, AReductionMapHandle, inducedA1ReductionMapHandle);
  NWAOBDDNodeHandle tempHandle0 = AConnection[0].entryPointHandle->Reduce(inducedA0ReductionMapHandle, inducedA0ReturnMap.Size(), forceReduce);
  NWAOBDDNodeHandle tempHandle1 = AConnection[1].entryPointHandle->Reduce(inducedA1ReductionMapHandle, inducedA1ReturnMap.Size(), forceReduce);

  n -> AConnection[0] = Connection(tempHandle0, inducedA0ReturnMap);
  n -> AConnection[1] = Connection(tempHandle1, inducedA1ReturnMap);

  n->numExits = replacementNumExits;
#ifdef PATH_COUNTING_ENABLED
  n->InstallPathCounts();
#endif 
  return NWAOBDDNodeHandle(n);
    /*
     for (unsigned int i = 0; i < numBConnections; i++) {
        
        ReductionMapHandle inducedReductionMapHandle0(redMapHandle.Size());
        ReductionMapHandle inducedReductionMapHandle1(redMapHandle.Size());
        

        ReturnMapHandle<intpair> reducedReturnMap0 = BConnection[0][i].returnMapHandle.Compose(redMapHandle);
		    ReturnMapHandle<intpair> reducedReturnMap1 = BConnection[1][i].returnMapHandle.Compose(redMapHandle);

        reducedReturnMap0.InducedReductionAndReturnMap(inducedReductionMapHandle0, inducedReturnMap0);
		    reducedReturnMap1.InducedReductionAndReturnMap(inducedReductionMapHandle1, inducedReturnMap1);
        NWAOBDDNodeHandle temp0 = BConnection[0][i].entryPointHandle->Reduce(inducedReductionMapHandle0, inducedReturnMap0.Size());
		    NWAOBDDNodeHandle temp1 = BConnection[1][i].entryPointHandle->Reduce(inducedReductionMapHandle1, inducedReturnMap1.Size());
        
        Connection c0(temp0, inducedReturnMap0);
		    Connection c1(temp1, inducedReturnMap1);
        unsigned int position = n->InsertBConnection(n->numBConnections, c0, c1);
        AReductionMapHandle.AddToEnd(position);
     }
     AReductionMapHandle.Canonicalize();
     if (n->numBConnections < numBConnections) {  // Shorten
       Connection *ntemp0 = n->BConnection[0];
	   Connection *ntemp1 = n->BConnection[1];
       n->BConnection[0] = new Connection[n->numBConnections];
	   n->BConnection[1] = new Connection[n->numBConnections];
       for (unsigned int j = 0; j < n->numBConnections; j++) {
         n->BConnection[0][j] = ntemp0[j];
		 n->BConnection[1][j] = ntemp1[j];
       }
       delete [] ntemp0;
	   delete [] ntemp1;
     }

  // Reduce the A connection
     ReductionMapHandle inducedA0ReductionMapHandle;
	 ReductionMapHandle inducedA1ReductionMapHandle;
     ReturnMapHandle<intpair> inducedA0ReturnMap;
	 ReturnMapHandle<intpair> inducedA1ReturnMap;
     ReturnMapHandle<intpair> reducedAReturnMap = AConnection[0].returnMapHandle.Compose(AReductionMapHandle);
	 reducedAReturnMap.InducedReductionAndReturnMap(inducedA0ReductionMapHandle, inducedA0ReturnMap);
     NWAOBDDNodeHandle tempHandle = AConnection[0].entryPointHandle->Reduce(inducedA0ReductionMapHandle, inducedA0ReturnMap.Size());
     n->AConnection[0] = Connection(tempHandle, inducedA0ReturnMap);

	 reducedAReturnMap = AConnection[1].returnMapHandle.Compose(AReductionMapHandle);
	 reducedAReturnMap.InducedReductionAndReturnMap(inducedA1ReductionMapHandle, inducedA1ReturnMap);
     NWAOBDDNodeHandle tempHandle1 = AConnection[1].entryPointHandle->Reduce(inducedA1ReductionMapHandle, inducedA1ReturnMap.Size());
     n->AConnection[1] = Connection(tempHandle1, inducedA1ReturnMap);

  // Other material that has to be filled in
     n->numExits = replacementNumExits;
#ifdef PATH_COUNTING_ENABLED
     n->InstallPathCounts();
#endif
     
  return NWAOBDDNodeHandle(n);
  */
} // NWAOBDDInternalNode::Reduce

//ETTODO better hash functions
unsigned int NWAOBDDInternalNode::Hash(unsigned int modsize)
{
    unsigned int hvalue = AConnection[0].Hash(modsize) + AConnection[1].Hash(modsize);
  for (unsigned int j = 0; j < numBConnections; j++) {
      hvalue = ((hvalue + BConnection[1][j].Hash(modsize) + BConnection[0][j].Hash(modsize))) % modsize;
  }
  return hvalue;
}

void NWAOBDDInternalNode::DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out /* = std::cout */)
{

  if (visited->Lookup(this) == NULL) {
    unsigned int i;
    visited->Insert(this);
    AConnection[0].entryPointHandle->handleContents->DumpConnections(visited, out);
    AConnection[1].entryPointHandle->handleContents->DumpConnections(visited, out);
    for (i = 0; i < numBConnections; i++) {
      BConnection[0][i].entryPointHandle->handleContents->DumpConnections(visited, out);
      BConnection[1][i].entryPointHandle->handleContents->DumpConnections(visited, out);
    }
    out << AConnection[0] << std::endl;
	out << AConnection[1] << std::endl;
    for (i = 0; i < numBConnections; i++) {
      out << BConnection[0][i] << std::endl;
      out << BConnection[1][i] << std::endl;
    }
  }
}

//CountNodesAndEdges
void NWAOBDDInternalNode::CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount)
{
  if (visitedNodes->Lookup(this) == NULL) {
    visitedNodes->Insert(this);
    nodeCount++;
    edgeCount += 2 + 2*numBConnections;
    if (visitedEdges->Lookup(AConnection[0].returnMapHandle.mapContents) == NULL) {
      visitedEdges->Insert(AConnection[0].returnMapHandle.mapContents);
      edgeCount += 2*AConnection[0].returnMapHandle.Size();
    }
    AConnection[0].entryPointHandle->handleContents->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
	if (visitedEdges->Lookup(AConnection[1].returnMapHandle.mapContents) == NULL) {
	  visitedEdges->Insert(AConnection[1].returnMapHandle.mapContents);
	  edgeCount += 2*AConnection[1].returnMapHandle.Size();
	}
    for (unsigned int i = 0; i < numBConnections; i++) {
      BConnection[0][i].entryPointHandle->handleContents->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
      if (visitedEdges->Lookup(BConnection[0][i].returnMapHandle.mapContents) == NULL) {
        visitedEdges->Insert(BConnection[0][i].returnMapHandle.mapContents);
        edgeCount += 2*BConnection[0][i].returnMapHandle.Size();
      }
	  BConnection[1][i].entryPointHandle->handleContents->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
	  if (visitedEdges->Lookup(BConnection[1][i].returnMapHandle.mapContents) == NULL) {
	    visitedEdges->Insert(BConnection[1][i].returnMapHandle.mapContents);
		edgeCount += 2*BConnection[1][i].returnMapHandle.Size();
	  }
    }
  }
}


// Overloaded !=
bool NWAOBDDInternalNode::operator!= (const NWAOBDDNode & n)
{
  return !(*this == n);
}

// Overloaded ==
bool NWAOBDDInternalNode::operator== (const NWAOBDDNode & n)
{
  if (n.NodeKind() != NWAOBDD_INTERNAL)
    return false;
  NWAOBDDInternalNode &m = (NWAOBDDInternalNode &)n;
  if (level != m.level)
    return false;
  if (numExits != m.numExits)
    return false;
  if (numBConnections != m.numBConnections)
    return false;
  if (AConnection[0] != m.AConnection[0])
    return false;
  if (AConnection[1] != m.AConnection[1])
    return false;
  //for (unsigned int j = numBConnections-1; j >= 0; j--) {
  for (unsigned int j = 0; j < numBConnections; j++) {
    if (BConnection[0][j] != m.BConnection[0][j])
      return false;
    if (BConnection[1][j] != m.BConnection[1][j])
      return false;
  }
  return true;
}

void NWAOBDDInternalNode::IncrRef()
{
  refCount++;    // Warning: Saturation not checked
}

void NWAOBDDInternalNode::DecrRef()
{
  if (--refCount == 0) {    // Warning: Saturation not checked
    if (isCanonical) {
      NWAOBDDNodeHandle::canonicalNodeTable->DeleteEq(this);
    }
    delete this;
  }
}

// Insert Connection c at BConnection position [j], 0 or 1  but only if c does not
// duplicate an existing BConnection.
// Return the position at which c was found or inserted.
unsigned int NWAOBDDInternalNode::InsertBConnection(unsigned int &j, Connection &c0, Connection &c1)
{
  for (unsigned int i = 0; i < j; i++) {
    if ((BConnection[0][i] == c0) && BConnection[1][i] == c1) {
      return i;
    }
  }
  BConnection[0][j] = c0;
  BConnection[1][j] = c1;
  j++;
  return j-1;
}




#ifdef PATH_COUNTING_ENABLED
// InstallPathCounts

long double addNumPathsToExit(const std::vector<long double>& logOfPaths){
  // assert(logOfPaths.size() > 0);
	if (logOfPaths.size() == 1)
		return logOfPaths.back();
	long double sum = 0.0;
	for (unsigned i = 0; i + 1 < logOfPaths.size(); i++){
		if (logOfPaths[i] != -1.0 * std::numeric_limits<long double>::infinity())
			sum += pow(2, logOfPaths[i] - logOfPaths.back());
	}
	long double logOfSum = logOfPaths.back() + log1p(sum)/((long double)log(2));
	return logOfSum;
}

void NWAOBDDInternalNode::InstallPathCounts()
{
  numPathsToMiddle = new long double[numBConnections];
  numPathsToExit = new long double[numExits];
  isNumPathsMemAllocated = true;
  for(int i = 0; i < numBConnections; ++i) numPathsToMiddle[i] = 0;
  for(int i = 0; i < numExits; ++i) numPathsToExit[i] = 0;

  std::map<unsigned int, std::vector<long double>> storingNumPathsToMiddle;
  for(unsigned b = 0; b <= 1; ++b)
  {
    for(unsigned j = 0; j < AConnection[b].entryPointHandle->handleContents->numExits; ++j) {
      unsigned k0 = AConnection[b].returnMapHandle.Lookup(j).First();
      unsigned k1 = AConnection[b].returnMapHandle.Lookup(j).Second();
      long double numPathsValue = AConnection[b].entryPointHandle->handleContents->numPathsToExit[j];
      if (storingNumPathsToMiddle.find(k0) == storingNumPathsToMiddle.end()) {
        std::vector<long double> logOfPaths;
        logOfPaths.push_back(numPathsValue);
        storingNumPathsToMiddle[k0] = logOfPaths;
      }
      else {
        storingNumPathsToMiddle[k0].push_back(numPathsValue);
      }

      if (storingNumPathsToMiddle.find(k1) == storingNumPathsToMiddle.end()) {
        std::vector<long double> logOfPaths;
        logOfPaths.push_back(numPathsValue);
        storingNumPathsToMiddle[k1] = logOfPaths;
      }
      else {
        storingNumPathsToMiddle[k1].push_back(numPathsValue);
      }

    }
  }
	for (std::map<unsigned int, std::vector<long double>>::iterator it = storingNumPathsToMiddle.begin(); it != storingNumPathsToMiddle.end(); it++){
		std::sort(it->second.begin(), it->second.end());
		numPathsToMiddle[it->first] = addNumPathsToExit(it->second);
	}


  std::map<unsigned int, std::vector<long double>> storingNumPathsToExit;
  for(unsigned i = 0; i < numBConnections; ++i) {
    for(unsigned b = 0; b <= 1; ++b)
    {
      for(unsigned j = 0; j < BConnection[b][i].entryPointHandle->handleContents->numExits; ++j) {
        unsigned k0 = BConnection[b][i].returnMapHandle.Lookup(j).First();
        unsigned k1 = BConnection[b][i].returnMapHandle.Lookup(j).Second();
        long double numPathsValue = numPathsToMiddle[i] + BConnection[b][i].entryPointHandle->handleContents->numPathsToExit[j];

        if (storingNumPathsToExit.find(k0) == storingNumPathsToExit.end()) {
          std::vector<long double> logOfPaths;
          logOfPaths.push_back(numPathsValue);
          storingNumPathsToExit[k0] = logOfPaths;
        }
        else {
          storingNumPathsToExit[k0].push_back(numPathsValue);
        }

        if (storingNumPathsToExit.find(k1) == storingNumPathsToExit.end()) {
          std::vector<long double> logOfPaths;
          logOfPaths.push_back(numPathsValue);
          storingNumPathsToExit[k1] = logOfPaths;
        }
        else {
          storingNumPathsToExit[k1].push_back(numPathsValue);
        }

      }
    }
  }
	for (std::map<unsigned int, std::vector<long double>>::iterator it = storingNumPathsToExit.begin(); it != storingNumPathsToExit.end(); it++){
		std::sort(it->second.begin(), it->second.end());
		numPathsToExit[it->first] = addNumPathsToExit(it->second);
	}

  /* ETTODO
  numPathsToExit = new unsigned int[numExits];
  for (unsigned int i = 0; i < numExits; i++) {
    numPathsToExit[i] = 0;
  }
  //Create a 2D array with numpaths to exit from a midpoint to a specific exit
    //Calculate num paths to exit for the midpoints
		//Calculate num paths from A to a midpoints
  for (unsigned int i = 0; i < AConnection.entryPointHandle->handleContents->numExits; i++) {
    for (unsigned int j = 0; j < BConnection[i].entryPointHandle->handleContents->numExits; j++) {
      unsigned int k = BConnection[i].returnMapHandle.Lookup(j);
      numPathsToExit[k] += AConnection.entryPointHandle->handleContents->numPathsToExit[i] * BConnection[i].entryPointHandle->handleContents->numPathsToExit[j];
    }
  }*/
}
#endif

//********************************************************************
// NWAOBDDEpsilonNode
//********************************************************************

// Constructors/Destructor -------------------------------------------

// Default constructor
NWAOBDDEpsilonNode::NWAOBDDEpsilonNode()
  :  NWAOBDDNode(0)
{
  numExits = 1;
#ifdef PATH_COUNTING_ENABLED //ETTODO
  numPathsToExit = new long double[1];
  numPathsToExit[0] = 0.0;
#endif
}

NWAOBDDEpsilonNode::~NWAOBDDEpsilonNode()
{
}

// print
std::ostream& NWAOBDDEpsilonNode::print(std::ostream & out) const
{
  for (unsigned int i = level; i < maxLevel; i++) {
    out << "  ";
  }
  out << "Epsilon";
  return out;
}

void NWAOBDDEpsilonNode::FillSatisfyingAssignment(unsigned int, SH_OBDD::Assignment &assignment, unsigned int &index)
{
}

int NWAOBDDEpsilonNode::Traverse(SH_OBDD::AssignmentIterator &ai)
{
  return 0;
}

NWAOBDDNodeHandle NWAOBDDEpsilonNode::Reduce(ReductionMapHandle, unsigned int, bool)
{
  return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
}

//ETTODO Better Hash Functions
unsigned int NWAOBDDEpsilonNode::Hash(unsigned int modsize)
{
  return ((unsigned int) reinterpret_cast<uintptr_t>(this) >> 2) % modsize;
}

// Overloaded !=
bool NWAOBDDEpsilonNode::operator!= (const NWAOBDDNode & n)
{
  return n.NodeKind() != NWAOBDD_EPSILON;
}

// Overloaded ==
bool NWAOBDDEpsilonNode::operator== (const NWAOBDDNode & n)
{
  return n.NodeKind() == NWAOBDD_EPSILON;
}

void NWAOBDDEpsilonNode::IncrRef() {}
void NWAOBDDEpsilonNode::DecrRef() {}
void NWAOBDDEpsilonNode::DumpConnections(Hashset<NWAOBDDNode>*, std::ostream &) {}
void NWAOBDDEpsilonNode::CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount) {
  if(visitedNodes->Lookup(this) == NULL) {
    visitedNodes -> Insert(this);
    nodeCount++;
  }
}
