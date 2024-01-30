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
#include "list_T.h"
#include "list_TPtr.h"
#include "intpair.h"
#include "conscell.h"
#include "assignment.h"
#include "bool_op.h"
#include "return_map_T.h"
#include "reduction_map.h"
#include "cross_product_nwa.h"
#include "traverse_state_nwa.h"
#include "hash.h"
#include "hashset.h"

using namespace NWA_OBDD;

//********************************************************************
// NWAOBDDNodeHandle
//
// Contains a canonical NWAOBDDNode*
//********************************************************************

// Initializations of static members ---------------------------------

Hashset<NWAOBDDNode> *NWAOBDDNodeHandle::canonicalNodeTable = new Hashset<NWAOBDDNode>(80000);
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
      n->AConnection[0].entryPointHandle = NoDistinctionNode[i-1];
      n->AConnection[1].entryPointHandle = NoDistinctionNode[i-1];
      m1.AddToEnd(intpair(0,0));
      m1.Canonicalize();
      n->AConnection[0].returnMapHandle = m1;
      n->AConnection[1].returnMapHandle = m1;
  
      n->numBConnections = 1;
	  n->BConnection[0] = new Connection[n->numBConnections];
	  n->BConnection[1] = new Connection[n->numBConnections];
      n->BConnection[0][0].entryPointHandle = NoDistinctionNode[i-1];
      n->BConnection[1][0].entryPointHandle = NoDistinctionNode[i-1];
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


NWAOBDDNodeHandle NWAOBDDNodeHandle::Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits)
{
  if (replacementNumExits == 1) {
    return NWAOBDDNodeHandle::NoDistinctionNode[handleContents->Level()];
  }

  if (redMapHandle.mapContents->isIdentityMap) {
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
    NWAOBDDNodeHandle temp = handleContents->Reduce(redMapHandle, replacementNumExits);
    reduceCache->Insert(NWAReduceKey(*this, redMapHandle), temp);
    return temp;
  }
}

void NWAOBDDNodeHandle::InitReduceCache()
{
  reduceCache = new Hashtable<NWAReduceKey, NWAOBDDNodeHandle>(40000);
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

enum IDTYPE {MATCH, PREV0, PREV1};
NWAOBDDNodeHandle MkIdRelationInterleaved(unsigned int level, IDTYPE idtype)
{
  NWAOBDDInternalNode *n;

  if (level == 0) {
    return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
    n->numExits = 1;
  }
  else if (level == 1) {
      n = new NWAOBDDInternalNode(level);
      if(idtype == MATCH){
          ReturnMapHandle<intpair> m01, m10, m11;
          m01.AddToEnd(intpair(0,1));
		  m01.Canonicalize();

          m10.AddToEnd(intpair(1,0));
		  m10.Canonicalize();

		  m11.AddToEnd(intpair(1,1));
		  m11.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, idtype);
          n->AConnection[0] = Connection(temp,m01);
          n->AConnection[1] = Connection(temp,m10);

          n->numBConnections = 2;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(temp,m01);
          n->BConnection[1][0] = Connection(temp,m10);
          n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);
          n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);
          n->numExits = 2;
      }
      else if (idtype == PREV0)
      {
		  ReturnMapHandle<intpair> m01, m22;
          m01.AddToEnd(intpair(0,1));
		  m01.Canonicalize();

          m22.AddToEnd(intpair(2,2));
		  m22.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, idtype);

          n->AConnection[0] = Connection(temp,m01);
          n->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);

          n->numBConnections = 3;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(temp,m01);
          n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);
          n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);
          n->BConnection[1][1] = Connection(temp, m01);
          n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);
          n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);
          n->numExits = 3;
      } else {
          ReturnMapHandle<intpair> m00, m12;
          m00.AddToEnd(intpair(0,0));
		  m00.Canonicalize();

          m12.AddToEnd(intpair(1,2));
          m12.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, idtype);

          n->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->AConnection[1] = Connection(temp,m12);

          n->numBConnections = 3;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->BConnection[0][1] = Connection(temp, m12);
          n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->BConnection[1][2] = Connection(temp, m12);
          n->numExits = 3;
      }
  }
  else {  // Create an appropriate NWAOBDDInternalNode
      n = new NWAOBDDInternalNode(level);
      if (idtype == MATCH){
          ReturnMapHandle<intpair> m01_10_11,m11_01_10;
          m01_10_11.AddToEnd(intpair(0,1));
          m01_10_11.AddToEnd(intpair(1,0));
          m01_10_11.AddToEnd(intpair(1,1));
		  m01_10_11.Canonicalize();

		  m11_01_10.AddToEnd(intpair(1,1));
          m11_01_10.AddToEnd(intpair(0,1));
          m11_01_10.AddToEnd(intpair(1,0));
          m11_01_10.Canonicalize();

          NWAOBDDNodeHandle temp0 = MkIdRelationInterleaved(level-1, PREV0);
          NWAOBDDNodeHandle temp1 = MkIdRelationInterleaved(level-1, PREV1);

          n->AConnection[0] = Connection(temp0,m01_10_11);
          n->AConnection[1] = Connection(temp1,m11_01_10);

		  ReturnMapHandle<intpair> m11;
		  m11.AddToEnd(intpair(1,1));
		  m11.Canonicalize();

          n->numBConnections = 2;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(temp0,m01_10_11);
          n->BConnection[1][0] = Connection(temp1,m11_01_10);
          n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
          n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
          n->numExits = 2;
      } else if (idtype == PREV0)
      {
          ReturnMapHandle<intpair> m01_22,m22;

          m01_22.AddToEnd(intpair(0,1));
		  m01_22.AddToEnd(intpair(2,2));
          m01_22.Canonicalize();
          
		  m22.AddToEnd(intpair(2,2));
          m22.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, MATCH);

          n->AConnection[0] = Connection(temp,m01_22);
          n->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);

          n->numBConnections = 3;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(temp, m01_22);
          n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);
          n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);
          n->BConnection[1][1] = Connection(temp, m01_22);
          n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);
          n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);
          n->numExits = 3;
      } else {
          ReturnMapHandle<intpair> m00, m12_00;
          m00.AddToEnd(intpair(0,0));
		  m00.Canonicalize();

		  m12_00.AddToEnd(intpair(1,2));
		  m12_00.AddToEnd(intpair(0,0));
          m12_00.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, MATCH);

          n->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->AConnection[1] = Connection(temp,m12_00);

          n->numBConnections = 3;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];

          n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->BConnection[0][1] = Connection(temp,m12_00);
          n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->BConnection[1][2] = Connection(temp,m12_00);
          n->numExits = 3;
      }
  }
  n->numExits = 2;
#ifdef PATH_COUNTING_ENABLED
  n->InstallPathCounts();
#endif
  return NWAOBDDNodeHandle(n);
} // MkIdRelationInterleaved


NWAOBDDNodeHandle MkAdditionNested(unsigned int level, bool carry)
{
    NWAOBDDInternalNode *n;

    assert(level >= 1);

    if(level == 1){
        n = new NWAOBDDInternalNode(level);
        ReturnMapHandle<intpair> m00,m01,m11,m12,m22;

		m00.AddToEnd(intpair(0,0));
		m00.Canonicalize();

        m01.AddToEnd(intpair(0,1));
        m01.Canonicalize();

		m11.AddToEnd(intpair(1,1));
		m11.Canonicalize();

        m12.AddToEnd(intpair(1,2));
        m12.Canonicalize();

        m22.AddToEnd(intpair(2,2));
        m22.Canonicalize();

        
        if(!carry) {
        // 3-return points for A's 0,1,2 - 00, 01/10, 11
        // Return Map for 0-AConnection: <0,1>
        // Return Map for 1-AConnection: <1,2>
            n->AConnection[0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m01);
            n->AConnection[1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m12);
    
            n->numBConnections = 3;
            n->BConnection[0] = new Connection[n->numBConnections];
            n->BConnection[1] = new Connection[n->numBConnections];

        // 3-return points for B's 0,1,2 -- True, False, True + Carry
        // Return Map for B0's 0-Connection: <0,0>
        // Return Map for B0's 1-Connection: <1,1>
        // Return Map for B1's 0-Connection: <1,1>
        // Return Map for B1's 1-Connection: <0,0>
        // Return Map for B2's 0-Connection: <2,2>
        // Return Map for B2's 1-Connection: <1,1>
            n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m11);
            n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m11);
            n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m22);
            n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m11);
            n->numExits = 3;
        } else { // carry = true
            //3 return points for the A-Connections 0, 1, 2 -- 00+1, 01/10 + 1, 11 +1
            //Return Map for A's 0-Connection: 0:0, 1:1
            //Return Map for A's 1-Connection: 0:1, 1:2
            n->AConnection[0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m01);
            n->AConnection[1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m12);

            n->numBConnections = 3;
            n->BConnection[0] = new Connection[n->numBConnections];
            n->BConnection[1] = new Connection[n->numBConnections];

            //There are 3 B-return points 0,1,2 -- F,T,C
            //Return Map for B0's 0 Connection: 0:0, 1:0
            //Return Map for B0's 1 Connection: 0:1, 1:1
            //Return Map for B1's 0-Connection: 0:2, 1:2
            //Return Map for B1's 1-Connection: 0:0, 1:0
            //Return Map for B2's 0-Connection: 0:0, 1:0
            //Return Map for B2's 1-Connection: 0:2, 1:2
            n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m11);
            n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m22);
            n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m22);
        }
    } else //level >=2
    {
        n = new NWAOBDDInternalNode(level); 
        ReturnMapHandle<intpair> m11, m00, m00_11_22, m00_12_23, m01_22_13, m00_23_34, m11_00_22, m13_22_34; 
        
        m11.AddToEnd(intpair(1,1));
        m11.Canonicalize();

		m00.AddToEnd(intpair(0, 0));
		m00.Canonicalize();

        m00_11_22.AddToEnd(intpair(0,0));
        m00_11_22.AddToEnd(intpair(1,1));
        m00_11_22.AddToEnd(intpair(2,2));
        m00_11_22.Canonicalize();

        m00_12_23.AddToEnd(intpair(0,0));
        m00_12_23.AddToEnd(intpair(1,2));
        m00_12_23.AddToEnd(intpair(2,3));
        m00_12_23.Canonicalize();

        m01_22_13.AddToEnd(intpair(0,1));
        m01_22_13.AddToEnd(intpair(2,2));
        m01_22_13.AddToEnd(intpair(1,3));
        m01_22_13.Canonicalize();

		m00_23_34.AddToEnd(intpair(0,0));
        m00_23_34.AddToEnd(intpair(2,3));
        m00_23_34.AddToEnd(intpair(3,4));
        m00_23_34.Canonicalize();

        m11_00_22.AddToEnd(intpair(1,1));
        m11_00_22.AddToEnd(intpair(0,0));
        m11_00_22.AddToEnd(intpair(2,2));
        m11_00_22.Canonicalize();

        m13_22_34.AddToEnd(intpair(1,3));
        m13_22_34.AddToEnd(intpair(2,2));
        m13_22_34.AddToEnd(intpair(3,4));
        m13_22_34.Canonicalize();

        NWAOBDDNodeHandle c0 = MkAdditionNested(level-1, false);
        NWAOBDDNodeHandle c1 = MkAdditionNested(level-1, true);
        if (!carry)
        {
            // A-Connections are (i-1) no carries
            // Their returns are T,F,c
            // There are 5 in between points
            // A0's Return Map is <0,1> <2,2> <1,3>
            // A1's Return Map is <1,3> <2,2> <3,4>
            n->AConnection[0] = Connection(c0,m01_22_13);
            n->AConnection[1] = Connection(c0,m13_22_34);

            n->numBConnections = 5;
            n->BConnection[0] = new Connection[n->numBConnections];
            n->BConnection[1] = new Connection[n->numBConnections];

            //There are 5 B-Connections 0: 0, 1:1, 2:false, 3:0/Carry 4: 1/carry
            n->BConnection[0][0] = Connection(c0, m00_11_22);
            n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[1][1] = Connection(c0, m00_11_22);
            n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[0][3] = Connection(c1, m11_00_22);
            n->BConnection[1][3] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[0][4] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[1][4] = Connection(c1, m11_00_22);
        } else { //carry = true
            n->AConnection[0] = Connection(c1,m00_12_23);
            n->AConnection[1] = Connection(c1,m00_23_34);
            
            n->numBConnections = 5;
            n->BConnection[0] = new Connection[n->numBConnections];
            n->BConnection[1] = new Connection[n->numBConnections];

            n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[0][1] = Connection(c0,m11_00_22);
            n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[1][2] = Connection(c0,m11_00_22);
            n->BConnection[0][3] = Connection(c1,m00_11_22);
            n->BConnection[1][3] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[0][4] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[1][4] = Connection(c1,m00_11_22);
        }
    }
    n->numExits = 3;
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
									NWAOBDDTopNodeRefPtr path = MkAnd(6, AEnt,AWeight,AExit,BEnt,BWeight,BExit); //Determine the weight of the path found from the start of the node to the given exit
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
									NWAOBDDTopNodeRefPtr path = MkAnd(6, AEnt,AWeight,AExit,BEnt,BWeight,BExit);
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
									NWAOBDDTopNodeRefPtr path = MkAnd(6,AEnt,AWeight,AExit,BEnt,BWeight,BExit);
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
									NWAOBDDTopNodeRefPtr path = MkAnd(6,AEnt,AWeight,AExit,BEnt,BWeight,BExit);
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
// Connection
//********************************************************************

// Default constructor
Connection::Connection()
{
}

// Constructor
Connection::Connection(NWAOBDDNodeHandle &entryPointHandle, ReturnMapHandle<intpair> (&returnMapHandle))
  : entryPointHandle(entryPointHandle), returnMapHandle(returnMapHandle){
}

// Constructor, rvalue-ref version to pass compiling
Connection::Connection(NWAOBDDNodeHandle &&entryPointHandle, ReturnMapHandle<intpair> (&returnMapHandle))
  : entryPointHandle(entryPointHandle), returnMapHandle(returnMapHandle){
}

// Constructor
Connection::Connection(NWAOBDDNode *entryPoint, ReturnMapHandle<intpair> (&returnMapHandle))
  : entryPointHandle(entryPoint), returnMapHandle(returnMapHandle){
}

Connection::~Connection()
{
}

//ETTODO update hash functions
// Hash
unsigned int Connection::Hash(unsigned int modsize)
{
  unsigned int hvalue = 0;
  hvalue = (returnMapHandle.Hash(modsize) + entryPointHandle.Hash(modsize)) % modsize;
  return hvalue;
}

// Overloaded =
Connection& Connection::operator= (const Connection& C)
{
  if (this != &C)      // don't assign to self!
  {
    entryPointHandle = C.entryPointHandle;
    returnMapHandle = C.returnMapHandle;
  }
  return *this;        
}

// Overloaded !=
bool Connection::operator!= (const Connection & C)
{
  return (returnMapHandle != C.returnMapHandle) || (entryPointHandle != C.entryPointHandle);
}

// Overloaded ==
bool Connection::operator== (const Connection & C)
{
  return (returnMapHandle == C.returnMapHandle) && (entryPointHandle == C.entryPointHandle);
}

// print
std::ostream& Connection::print(std::ostream & out) const
{
  out << entryPointHandle;
  out << returnMapHandle;
  return out;
}

namespace NWA_OBDD {
std::ostream& operator<< (std::ostream & out, const Connection &c)
{
  c.print(out);
  return(out);
}
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
    delete [] numPathsToExit;
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
  for (unsigned int i = 0; i < AConnection[0].entryPointHandle.handleContents->numExits; i++) {
	  int ival = AConnection[0].returnMapHandle.Lookup(i).First(); //for each 0 return from that exit
    //For each exit of the 0 B-Connection from that current point
    for (unsigned int j = 0; j < BConnection[0][ival].entryPointHandle.handleContents->numExits; j++) {
		unsigned int k = BConnection[0][ival].returnMapHandle.Lookup(j).First(); //Get the 0 return from that exit
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[0][ival].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 0;
        AConnection[0].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
	  k = BConnection[0][ival].returnMapHandle.Lookup(j).Second(); //else get the 1 return from that exit
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[0][i].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 0;
        AConnection[0].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
    }
	//Look at the 1-BConnection from our midpoint
    for (unsigned int j = 0; j < BConnection[1][ival].entryPointHandle.handleContents->numExits; j++) {
		unsigned int k = BConnection[1][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[1][ival].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 0;
        AConnection[0].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
	  k = BConnection[1][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[1][i].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 0;
        AConnection[0].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
    }
	ival = AConnection[0].returnMapHandle.Lookup(i).Second();
    for (unsigned int j = 0; j < BConnection[0][ival].entryPointHandle.handleContents->numExits; j++) {
      unsigned int k = BConnection[0][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[0][ival].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 1;
        AConnection[0].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
	  k = BConnection[0][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[0][i].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 1;
        AConnection[0].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
    }
    for (unsigned int j = 0; j < BConnection[1][ival].entryPointHandle.handleContents->numExits; j++) {
      unsigned int k = BConnection[1][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[1][ival].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 1;
        AConnection[0].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
      k = BConnection[1][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[1][i].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 1;
        AConnection[0].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 0;
        return;
      }
    }
  }
  for (unsigned int i = 0; i < AConnection[1].entryPointHandle.handleContents->numExits; i++) {
	  int ival = AConnection[1].returnMapHandle.Lookup(i).First(); //for each 0 return
    //For each 0 BConnection coming from a 0 a connection
    for (unsigned int j = 0; j < BConnection[0][ival].entryPointHandle.handleContents->numExits; j++) {
		unsigned int k = BConnection[0][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[0][ival].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 0;
        AConnection[1].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
      k = BConnection[0][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[0][i].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 0;
        AConnection[1].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
    }
    for (unsigned int j = 0; j < BConnection[1][ival].entryPointHandle.handleContents->numExits; j++) {
      unsigned int k = BConnection[1][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[1][ival].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 0;
        AConnection[1].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
      k = BConnection[1][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[1][i].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 0;
        AConnection[1].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
    }
	ival = AConnection[1].returnMapHandle.Lookup(i).Second(); //for each 1 return
    //For each 0 BConnection coming from a 1 a connection
    for (unsigned int j = 0; j < BConnection[0][ival].entryPointHandle.handleContents->numExits; j++) {
		unsigned int k = BConnection[0][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[0][ival].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 1;
        AConnection[1].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
      k = BConnection[0][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[0][i].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 0;
		index--;
		assignment[index] = 1;
        AConnection[1].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
    }
    for (unsigned int j = 0; j < BConnection[1][ival].entryPointHandle.handleContents->numExits; j++) {
		unsigned int k = BConnection[1][ival].returnMapHandle.Lookup(j).First();
      if (k == exitNumber) { // Found it
		index--;
		assignment[index] = 0;
        BConnection[1][ival].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 1;
        AConnection[1].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
		index--;
		assignment[index] = 1;
        return;
      }
      k = BConnection[1][ival].returnMapHandle.Lookup(j).Second();
      if (k == exitNumber) { 
		index--;
		assignment[index] = 1;
        BConnection[1][i].entryPointHandle.handleContents->FillSatisfyingAssignment(j, assignment, index);
		index--;
		assignment[index] = 1;
		index--;
		assignment[index] = 1;
        AConnection[1].entryPointHandle.handleContents->FillSatisfyingAssignment(i, assignment, index);
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
    i = AConnection[val].entryPointHandle.handleContents->Traverse(ai);
	bool val1 = ai.Current();
	ai.Next();
	if(!val1)
		j = AConnection[val].returnMapHandle.Lookup(i).First();
	else
		j = AConnection[val].returnMapHandle.Lookup(i).Second();

	bool val2 = ai.Current();
	ai.Next();
	k = BConnection[val2][j].entryPointHandle.handleContents->Traverse(ai);
	
	bool val3 = ai.Current();
	ai.Next();
    if(!val3)
		ans = BConnection[val2][j].returnMapHandle.Lookup(k).First();
	else
		ans = BConnection[val2][j].returnMapHandle.Lookup(k).Second();
  }
  return ans;
}

NWAOBDDNodeHandle NWAOBDDInternalNode::Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits)
{
  NWAOBDDInternalNode *n = new NWAOBDDInternalNode(level);
  ReductionMapHandle AReductionMapHandle;        // To record duplicate BConnections

  // Reduce the B connections
     n->BConnection[0] = new Connection[numBConnections];   // May create shorter version later
	 n->BConnection[1] = new Connection[numBConnections];
     n->numBConnections = 0;
     for (unsigned int i = 0; i < numBConnections; i++) {
        ReductionMapHandle inducedReductionMapHandle0, inducedReductionMapHandle1;
        ReturnMapHandle<intpair> inducedReturnMap0, inducedReturnMap1;
        ReturnMapHandle<intpair> reducedReturnMap0 = BConnection[0][i].returnMapHandle.Compose(redMapHandle);
		ReturnMapHandle<intpair> reducedReturnMap1 = BConnection[1][i].returnMapHandle.Compose(redMapHandle);
        reducedReturnMap0.InducedReductionAndReturnMap(inducedReductionMapHandle0, inducedReturnMap0);
		reducedReturnMap1.InducedReductionAndReturnMap(inducedReductionMapHandle1, inducedReturnMap1);
        NWAOBDDNodeHandle temp0 = BConnection[0][i].entryPointHandle.Reduce(inducedReductionMapHandle0, inducedReturnMap0.Size());
		NWAOBDDNodeHandle temp1 = BConnection[1][i].entryPointHandle.Reduce(inducedReductionMapHandle1, inducedReturnMap1.Size());
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
     NWAOBDDNodeHandle tempHandle = AConnection[0].entryPointHandle.Reduce(inducedA0ReductionMapHandle, inducedA0ReturnMap.Size());
     n->AConnection[0] = Connection(tempHandle, inducedA0ReturnMap);

	 reducedAReturnMap = AConnection[1].returnMapHandle.Compose(AReductionMapHandle);
	 reducedAReturnMap.InducedReductionAndReturnMap(inducedA1ReductionMapHandle, inducedA1ReturnMap);
     NWAOBDDNodeHandle tempHandle1 = AConnection[1].entryPointHandle.Reduce(inducedA1ReductionMapHandle, inducedA1ReturnMap.Size());
     n->AConnection[1] = Connection(tempHandle1, inducedA1ReturnMap);

  // Other material that has to be filled in
     n->numExits = replacementNumExits;
#ifdef PATH_COUNTING_ENABLED
     n->InstallPathCounts();
#endif
     
  return NWAOBDDNodeHandle(n);
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
    AConnection[0].entryPointHandle.handleContents->DumpConnections(visited, out);
    AConnection[1].entryPointHandle.handleContents->DumpConnections(visited, out);
    for (i = 0; i < numBConnections; i++) {
      BConnection[0][i].entryPointHandle.handleContents->DumpConnections(visited, out);
      BConnection[1][i].entryPointHandle.handleContents->DumpConnections(visited, out);
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
    AConnection[0].entryPointHandle.handleContents->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
	if (visitedEdges->Lookup(AConnection[1].returnMapHandle.mapContents) == NULL) {
	  visitedEdges->Insert(AConnection[1].returnMapHandle.mapContents);
	  edgeCount += 2*AConnection[1].returnMapHandle.Size();
	}
    for (unsigned int i = 0; i < numBConnections; i++) {
      BConnection[0][i].entryPointHandle.handleContents->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
      if (visitedEdges->Lookup(BConnection[0][i].returnMapHandle.mapContents) == NULL) {
        visitedEdges->Insert(BConnection[0][i].returnMapHandle.mapContents);
        edgeCount += 2*BConnection[0][i].returnMapHandle.Size();
      }
	  BConnection[1][i].entryPointHandle.handleContents->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
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

//ETTODO PathCounts
#ifdef PATH_COUNTING_ENABLED
// InstallPathCounts
void NWAOBDDInternalNode::InstallPathCounts()
{/* ETTODO
  numPathsToExit = new unsigned int[numExits];
  for (unsigned int i = 0; i < numExits; i++) {
    numPathsToExit[i] = 0;
  }
  //Create a 2D array with numpaths to exit from a midpoint to a specific exit
    //Calculate num paths to exit for the midpoints
		//Calculate num paths from A to a midpoints
  for (unsigned int i = 0; i < AConnection.entryPointHandle.handleContents->numExits; i++) {
    for (unsigned int j = 0; j < BConnection[i].entryPointHandle.handleContents->numExits; j++) {
      unsigned int k = BConnection[i].returnMapHandle.Lookup(j);
      numPathsToExit[k] += AConnection.entryPointHandle.handleContents->numPathsToExit[i] * BConnection[i].entryPointHandle.handleContents->numPathsToExit[j];
    }
  }*/
}
#endif


//********************************************************************
// NWAOBDDLeafNode
//********************************************************************

// Constructors/Destructor -------------------------------------------

// Default constructor
NWAOBDDLeafNode::NWAOBDDLeafNode()
  :  NWAOBDDNode(0)
{
  refCount = 1;
}

NWAOBDDLeafNode::~NWAOBDDLeafNode()
{
}

void NWAOBDDLeafNode::DumpConnections(Hashset<NWAOBDDNode> *, std::ostream & /* = std::cout */ )
{
}

//ETTODO CountNodesAndEdges
void NWAOBDDLeafNode::CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *, unsigned int &nodeCount, unsigned int &)
{
  if (visitedNodes->Lookup(this) == NULL) {
    visitedNodes->Insert(this);
    nodeCount++;
  }
}

void NWAOBDDLeafNode::IncrRef() { }
void NWAOBDDLeafNode::DecrRef() { }

//********************************************************************
// NWAOBDDEpsilonNode
//********************************************************************

// Constructors/Destructor -------------------------------------------

// Default constructor
NWAOBDDEpsilonNode::NWAOBDDEpsilonNode()
  :  NWAOBDDLeafNode()
{
  numExits = 1;
#ifdef PATH_COUNTING_ENABLED //ETTODO
  numPathsToExit = new unsigned int[1];
  numPathsToExit[0] = 2;
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

NWAOBDDNodeHandle NWAOBDDEpsilonNode::Reduce(ReductionMapHandle, unsigned int)
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

//********************************************************************
// NWAOBDDTopNode
//********************************************************************

// Initializations of static members ---------------------------------

unsigned int const NWAOBDDTopNode::maxLevel = NWAOBDDMaxLevel;
Hashset<NWAOBDDTopNode> *NWAOBDDTopNode::computedCache = new Hashset<NWAOBDDTopNode>(10000);

// Constructors/Destructor -------------------------------------------

NWAOBDDTopNode::NWAOBDDTopNode(NWAOBDDNode *n, ReturnMapHandle<intpair>(&mapHandle)) : NWAOBDDBaseNode(n,mapHandle)
{
	rootConnection = Connection(n, mapHandle);
}

NWAOBDDTopNode::NWAOBDDTopNode(NWAOBDDNodeHandle &nodeHandle, ReturnMapHandle<intpair>(&mapHandle)) : NWAOBDDBaseNode(nodeHandle, mapHandle)
{
	rootConnection = Connection(nodeHandle, mapHandle);
}

NWAOBDDTopNode::~NWAOBDDTopNode()
{
}

void NWAOBDDTopNode::DeallocateMemory()
{
	NWAOBDDTopNode::~NWAOBDDTopNode();
}

// Evaluate
//    Return the value of the Boolean function under the given assignment
bool NWAOBDDTopNode::Evaluate(SH_OBDD::Assignment &assignment)
{
  SH_OBDD::AssignmentIterator ai(assignment);
  int i = rootConnection.entryPointHandle.handleContents->Traverse(ai);
  bool ans = rootConnection.returnMapHandle.Lookup(i).First();
  return ans;
}


//ETTODO IterativeEvaluation
// EvaluateIteratively
//    Return the value of the Boolean function under the given assignment
bool NWAOBDDTopNode::EvaluateIteratively(SH_OBDD::Assignment &assignment)
{
  return true; //ETTODO
  /*
  AssignmentIterator ai(assignment);

  unsigned int exitIndex = 0;
  ConsCell<TraverseState> *S = NULL;
  TraverseState ts;
  bool ans;

  S = new ConsCell<TraverseState>(TraverseState(rootConnection.entryPointHandle.handleContents,FirstVisit), S);
  while (S != NULL) {
    ts = S->Item();
    S = S->Next();
    if (ts.node->NodeKind() == NWAOBDD_EPSILON) {
      ai.Next();
      exitIndex = 0;
    }
    else if (ts.node->NodeKind() == NWAOBDD_FORK) {
      bool val = ai.Current();
      ai.Next();
      exitIndex = (int)val;
    }
    else {  // Must be a NWAOBDDInternalNode
      NWAOBDDInternalNode *n = (NWAOBDDInternalNode *)ts.node;

      if (ts.visitState == FirstVisit) {
        S = new ConsCell<TraverseState>(TraverseState(n, SecondVisit), S);
        S = new ConsCell<TraverseState>(TraverseState(n->AConnection.entryPointHandle.handleContents, FirstVisit), S);
      }
      else if (ts.visitState == SecondVisit) {
        int i = n->AConnection.returnMapHandle.Lookup(exitIndex);
        S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, i), S);
        S = new ConsCell<TraverseState>(TraverseState(n->BConnection[i].entryPointHandle.handleContents, FirstVisit), S);
      }
      else {  // if (ts.visitState == ThirdVisit)
        exitIndex = n->BConnection[ts.index].returnMapHandle.Lookup(exitIndex);
      }
    }
  }
  ans = rootConnection.returnMapHandle.Lookup(exitIndex);
  return ans;*/
}

// PrintYield -----------------------------------------------------

//ETTODO PrintYield
// PrintYieldAux
void NWAOBDDTopNode::PrintYieldAux(std::ostream * out, List<ConsCell<TraverseState> *> &T, ConsCell<TraverseState> *S)
{//ETTODO
  unsigned int exitIndex = 0;
  TraverseState ts;
  int ans;

  while (S != NULL) {
    ts = S->Item();
    S = S->Next();
    if (ts.node->NodeKind() == NWAOBDD_EPSILON) {
        exitIndex = 0;
    }
    else {  // Must be a NWAOBDDInternalNode
      NWAOBDDInternalNode *n = (NWAOBDDInternalNode *)ts.node;

      if (ts.visitState == FirstVisit) {
		  T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,RestartFirst), S));
        S = new ConsCell<TraverseState>(TraverseState(n, SecondVisit, 0, 0), S);
        S = new ConsCell<TraverseState>(TraverseState(n->AConnection[0].entryPointHandle.handleContents, FirstVisit), S);
      } else if (ts.visitState == RestartFirst) {
        S = new ConsCell<TraverseState>(TraverseState(n, SecondVisit, 0, 1), S);
        S = new ConsCell<TraverseState>(TraverseState(n->AConnection[1].entryPointHandle.handleContents, FirstVisit), S);
	  } else if (ts.visitState == SecondVisit) {
		  T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,SecondVisitOne, 0, ts.val1, 0, exitIndex), S));
          int i = n->AConnection[ts.val1].returnMapHandle.Lookup(exitIndex).First();
		  T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,RestartSecond, i, ts.val1, 0), S));
        S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, i, ts.val1, 0), S);
        S = new ConsCell<TraverseState>(TraverseState(n->BConnection[0][i].entryPointHandle.handleContents, FirstVisit), S);
      } else if (ts.visitState == RestartSecond) {
		  S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, ts.index, ts.val1, 1), S);
        S = new ConsCell<TraverseState>(TraverseState(n->BConnection[1][ts.index].entryPointHandle.handleContents, FirstVisit), S);
	  } else if (ts.visitState == SecondVisitOne) {
		  int i = n->AConnection[ts.val1].returnMapHandle.Lookup(ts.exitIndex).Second();
		  T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,RestartSecond, i, ts.val1, 0), S));
		  S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, i, ts.val1, 0), S);
		  S = new ConsCell<TraverseState>(TraverseState(n->BConnection[0][i].entryPointHandle.handleContents, FirstVisit), S);
	  } else if (ts.visitState == ThirdVisit) {
		  T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,RestartThird, ts.index, 0, ts.val2, exitIndex), S));
		  exitIndex = n->BConnection[ts.val2][ts.index].returnMapHandle.Lookup(exitIndex).First();
	  } else {  // if (ts.visitState == RestartThird)
        exitIndex = n->BConnection[ts.val2][ts.index].returnMapHandle.Lookup(ts.exitIndex).Second();
      }
    }
  }
  ans = rootConnection.returnMapHandle.Lookup(exitIndex).First();
  if (out != NULL) *out << ans;
}

//ETTODO PrintYield
// PrintYield
//
// print the yield of the NWAOBDDTopNode (i.e., the leaves of 0's and 1's
// in "left-to-right order").
//
void NWAOBDDTopNode::PrintYield(std::ostream * out)
{
  ConsCell<TraverseState> *S = NULL;   // Traversal stack
  List<ConsCell<TraverseState> *> T;   // Snapshot stack

  S = new ConsCell<TraverseState>(TraverseState(rootConnection.entryPointHandle.handleContents,FirstVisit), S);
  PrintYieldAux(out, T, S);
  while (!T.IsEmpty()) {
    S = T.RemoveFirst();
    PrintYieldAux(out, T, S);
  }
}

// Satisfaction Operations ------------------------------------

//ETTODO NumSatAssignments
#ifdef PATH_COUNTING_ENABLED
// NumSatisfyingAssignments
//
// Return the number of satisfying assignments
//
// Running time: Linear in the size of the NWAOBDDTopNode
//
unsigned int NWAOBDDTopNode::NumSatisfyingAssignments()
{
  unsigned int ans = 0;

  /*for (unsigned int i = 0; i < rootConnection.entryPointHandle.handleContents->numExits; i++) {
    unsigned int k = rootConnection.returnMapHandle[0].Lookup(i);
    if (k == 1) {
      ans += rootConnection.entryPointHandle.handleContents->numPathsToExit[i];
    }
  }*/
  return ans;
}
#endif

// FindOneSatisfyingAssignment
//
// If a satisfying assignment exists, allocate and place such an
//    assignment in variable "assignment" and return true.
// Otherwise return false.
//
// Running time: Linear in the number of variables
//
bool NWAOBDDTopNode::FindOneSatisfyingAssignment(SH_OBDD::Assignment * &assignment)
{
  for (unsigned int i = 0; i < rootConnection.entryPointHandle.handleContents->numExits; i++) {
    unsigned int k = rootConnection.returnMapHandle.Lookup(i).First();
    if (k == 1) {  // A satisfying assignment must exist
      unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDDTopNode::maxLevel + 2)) - (unsigned int)4));
      assignment = new SH_OBDD::Assignment(size);
      rootConnection.entryPointHandle.handleContents->FillSatisfyingAssignment(i, *assignment, size);
      return true;
    }
  }
  return false;
}

void NWAOBDDTopNode::DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out /* = std::cout */)
{
  rootConnection.entryPointHandle.handleContents->DumpConnections(visited, out);
  out << rootConnection << std::endl;
}

//ETTODO CountNodesAndEdges
void NWAOBDDTopNode::CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount)
{//ETTODO
  rootConnection.entryPointHandle.handleContents->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
  if (visitedEdges->Lookup(rootConnection.returnMapHandle.mapContents) == NULL) {
    visitedEdges->Insert(rootConnection.returnMapHandle.mapContents);
    edgeCount += rootConnection.returnMapHandle.Size();
  }
}

// Hash
unsigned int NWAOBDDTopNode::Hash(unsigned int modsize)
{
  return rootConnection.Hash(modsize);
}

// Overloaded !=
bool NWAOBDDTopNode::operator!= (const NWAOBDDTopNode & C)
{
  return rootConnection != C.rootConnection;
}

// Overloaded ==
bool NWAOBDDTopNode::operator== (const NWAOBDDTopNode & C)
{
  return rootConnection == C.rootConnection;
}

// print
std::ostream& NWAOBDDTopNode::print(std::ostream & out) const
{
  out << rootConnection.entryPointHandle << std::endl;
  out << rootConnection.returnMapHandle << std::endl;
  return out;
}

//********************************************************************
// NWAOBDDBaseNode
//********************************************************************

// Constructors/Destructor -------------------------------------------

NWAOBDDBaseNode::NWAOBDDBaseNode(NWAOBDDNode *n, ReturnMapHandle<intpair>(&mapHandle))
{
	assert(mapHandle.Size() <= 2);
	level = n->level;
	rootConnection = Connection(n, mapHandle);
}

NWAOBDDBaseNode::NWAOBDDBaseNode(NWAOBDDNodeHandle &nodeHandle, ReturnMapHandle<intpair>(&mapHandle))
{
	assert(mapHandle.Size() <= 2);
	level = nodeHandle.handleContents->level;
	rootConnection = Connection(nodeHandle, mapHandle);
}

NWAOBDDBaseNode::~NWAOBDDBaseNode()
{
}

void NWAOBDDBaseNode::DeallocateMemory()
{
	NWAOBDDBaseNode::~NWAOBDDBaseNode();
}

// PrintYield -----------------------------------------------------

//ETTODO PrintYield
// PrintYieldAux
void NWAOBDDBaseNode::PrintYieldAux(std::ostream * out, List<ConsCell<TraverseState> *> &T, ConsCell<TraverseState> *S)
{//ETTODO
	unsigned int exitIndex = 0;
	TraverseState ts;
	int ans;

	while (S != NULL) {
		ts = S->Item();
		S = S->Next();
		if (ts.node->NodeKind() == NWAOBDD_EPSILON) {
			exitIndex = 0;
		}
		else {  // Must be a NWAOBDDInternalNode
			NWAOBDDInternalNode *n = (NWAOBDDInternalNode *)ts.node;

			if (ts.visitState == FirstVisit) {
				T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node, RestartFirst), S));
				S = new ConsCell<TraverseState>(TraverseState(n, SecondVisit, 0, 0), S);
				S = new ConsCell<TraverseState>(TraverseState(n->AConnection[0].entryPointHandle.handleContents, FirstVisit), S);
			}
			else if (ts.visitState == RestartFirst) {
				S = new ConsCell<TraverseState>(TraverseState(n, SecondVisit, 0, 1), S);
				S = new ConsCell<TraverseState>(TraverseState(n->AConnection[1].entryPointHandle.handleContents, FirstVisit), S);
			}
			else if (ts.visitState == SecondVisit) {
				T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node, SecondVisitOne, 0, ts.val1, 0, exitIndex), S));
				int i = n->AConnection[ts.val1].returnMapHandle.Lookup(exitIndex).First();
				T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node, RestartSecond, i, ts.val1, 0), S));
				S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, i, ts.val1, 0), S);
				S = new ConsCell<TraverseState>(TraverseState(n->BConnection[0][i].entryPointHandle.handleContents, FirstVisit), S);
			}
			else if (ts.visitState == RestartSecond) {
				S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, ts.index, ts.val1, 1), S);
				S = new ConsCell<TraverseState>(TraverseState(n->BConnection[1][ts.index].entryPointHandle.handleContents, FirstVisit), S);
			}
			else if (ts.visitState == SecondVisitOne) {
				int i = n->AConnection[ts.val1].returnMapHandle.Lookup(ts.exitIndex).Second();
				T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node, RestartSecond, i, ts.val1, 0), S));
				S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, i, ts.val1, 0), S);
				S = new ConsCell<TraverseState>(TraverseState(n->BConnection[0][i].entryPointHandle.handleContents, FirstVisit), S);
			}
			else if (ts.visitState == ThirdVisit) {
				T.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node, RestartThird, ts.index, 0, ts.val2, exitIndex), S));
				exitIndex = n->BConnection[ts.val2][ts.index].returnMapHandle.Lookup(exitIndex).First();
			}
			else {  // if (ts.visitState == RestartThird)
				exitIndex = n->BConnection[ts.val2][ts.index].returnMapHandle.Lookup(ts.exitIndex).Second();
			}
		}
	}
	ans = rootConnection.returnMapHandle.Lookup(exitIndex).First();
	if (out != NULL) *out << ans;
}

//ETTODO PrintYield
// PrintYield
//
// print the yield of the NWAOBDDTopNode (i.e., the leaves of 0's and 1's
// in "left-to-right order").
//
void NWAOBDDBaseNode::PrintYield(std::ostream * out)
{
	ConsCell<TraverseState> *S = NULL;   // Traversal stack
	List<ConsCell<TraverseState> *> T;   // Snapshot stack

	S = new ConsCell<TraverseState>(TraverseState(rootConnection.entryPointHandle.handleContents, FirstVisit), S);
	PrintYieldAux(out, T, S);
	while (!T.IsEmpty()) {
		S = T.RemoveFirst();
		PrintYieldAux(out, T, S);
	}
}

// Hash
unsigned int NWAOBDDBaseNode::Hash(unsigned int modsize)
{
	return rootConnection.Hash(modsize);
}

// Overloaded !=
bool NWAOBDDBaseNode::operator!= (const NWAOBDDBaseNode & C)
{
	return rootConnection != C.rootConnection;
}

// Overloaded ==
bool NWAOBDDBaseNode::operator== (const NWAOBDDBaseNode & C)
{
	return rootConnection == C.rootConnection;
}

// print
std::ostream& NWAOBDDBaseNode::print(std::ostream & out) const
{
	out << rootConnection.entryPointHandle << std::endl;
	out << rootConnection.returnMapHandle << std::endl;
	return out;
}


namespace NWA_OBDD {

void setMaxLevel(unsigned int level)
{
	NWAOBDDMaxLevel = level;
}


std::ostream& operator<< (std::ostream & out, const NWAOBDDTopNode &d)
{
  d.print(out);
  return(out);
}

// NWAOBDDTopNode-creation operations --------------------------------------

// Create representation of \x.true
NWAOBDDTopNodeRefPtr MkTrueTop()
{
  NWAOBDDTopNodeRefPtr v;
  ReturnMapHandle<intpair> m;

  m.AddToEnd(intpair(1,1));  // Map the one exit of the body to T
  m.Canonicalize();
  
  v = new NWAOBDDTopNode(NWAOBDDNodeHandle::NoDistinctionNode[NWAOBDDMaxLevel], m);
  return v;
}

// Create representation of \x.false
NWAOBDDTopNodeRefPtr MkFalseTop()
{
  NWAOBDDTopNodeRefPtr v;
  ReturnMapHandle<intpair> m;

  m.AddToEnd(intpair(0,0));  // Map the one exit of the body to F
  m.Canonicalize();

  v = new NWAOBDDTopNode(NWAOBDDNodeHandle::NoDistinctionNode[NWAOBDDMaxLevel], m);
  return v;
}

// Create representation of \x.x_i
NWAOBDDTopNodeRefPtr MkDistinction(unsigned int i)
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle<intpair> m;

  assert(i < ((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4));   // i.e., i < 2**maxLevel

  tempHandle = MkDistinction(NWAOBDDMaxLevel, i);
  m.AddToEnd(intpair(0,0));
  m.AddToEnd(intpair(1,1));
  m.Canonicalize();

  v = new NWAOBDDTopNode(tempHandle, m);
  return v;
}

// Create representation of identity relation (with interleaved variable order).
// That is, input (x0,y0,x1,y1,...,xN,yN) yield Id[(x0,x1,...,xN)][(y0,y1,...,yN)]
// which equals 1 iff xi == yi, for 0 <= i <= N.
NWAOBDDTopNodeRefPtr MkIdRelationInterleavedTop()
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle<intpair> m;

  tempHandle = MkIdRelationInterleaved(NWAOBDDMaxLevel, MATCH);
  m.AddToEnd(intpair(1,1));
  m.AddToEnd(intpair(0,0));
  m.Canonicalize();

  v = new NWAOBDDTopNode(tempHandle, m);
  return v;
}

NWAOBDDTopNodeRefPtr MkIdRelationNestedTop()
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle<intpair> m;

  tempHandle = MkIdRelationNested(NWAOBDDMaxLevel);
  m.AddToEnd(intpair(1,1));
  m.AddToEnd(intpair(0,0));
  m.Canonicalize();

  v = new NWAOBDDTopNode(tempHandle, m);
  return v;
}

// Create representation of addition relation with interleaved variables
// { (xi yi zi _)* | vec{x} + vec{y} = vec{z} }
NWAOBDDTopNodeRefPtr MkAdditionNestedTop()
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle n;
  ReturnMapHandle<intpair> m10;

  n = MkAdditionNested(NWAOBDDMaxLevel, false);

  //ETTODO FixReductionMap
  // Reduce n by mapping the "carry=0" and "carry=1" exits to accept
     ReturnMapHandle<intpair> retMapHandle;
     m10.AddToEnd(intpair(1,1));
     m10.AddToEnd(intpair(0,0));
     m10.Canonicalize();
     ReductionMapHandle reductionMapHandle;
     reductionMapHandle.AddToEnd(0);
     reductionMapHandle.AddToEnd(1);
     reductionMapHandle.AddToEnd(0);
     //     NWAOBDDNodeHandle::InitReduceCache();
     NWAOBDDNodeHandle reduced_n = n.Reduce(reductionMapHandle, m10.Size());
     //     NWAOBDDNodeHandle::DisposeOfReduceCache();


  // Create and return NWAOBDDTopNode
     v = new NWAOBDDTopNode(reduced_n, m10);
     return(v);
}


//ETTODO Detensor
// Create representation of (W,X,Y,Z) s.t. X==Y with interleaved variables
NWAOBDDTopNodeRefPtr MkDetensorConstraintInterleavedTop()
{
  NWAOBDDTopNodeRefPtr v;/*ETTODO 
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  tempHandle = MkDetensorConstraintInterleaved(NWAOBDDMaxLevel);
  m.AddToEnd(0);
  m.AddToEnd(1);
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);*/
  return v;
}

// Create representation of parity function
NWAOBDDTopNodeRefPtr MkParityTop()
{
  NWAOBDDTopNodeRefPtr v; 
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle<intpair> m;

  tempHandle = MkParity(NWAOBDDMaxLevel);
  m.AddToEnd(intpair(0,0));
  m.AddToEnd(intpair(1,1));
  m.Canonicalize();

  v = new NWAOBDDTopNode(tempHandle, m);
  return v;
}

//ETTODO Walsh

// Create representation of the Walsh matrix W(2**(i-1))
// [i.e., a matrix of size 2**(2**(i-1))) x 2**(2**(i-1)))]
// with interleaved indexing of components: that is, input
// (x0,y0,x1,y1,...,xN,yN) yields W[(x0,x1,...,xN)][(y0,y1,...,yN)]
NWAOBDDTopNodeRefPtr MkWalshInterleavedTop(unsigned int i)
{
  NWAOBDDTopNodeRefPtr v;/*ETTODO 
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  assert(i <= NWAOBDDMaxLevel);

  tempHandle = MkWalshInterleavedNode(i);
  m.AddToEnd(1);
  m.AddToEnd(-1);
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);*/
  return v;
}

//ETTODO ReedMuller
// Create representation of the Inverse Reed-Muller matrix IRM(2**(i-1))
// [i.e., a matrix of size 2**(2**(i-1))) x 2**(2**(i-1)))]
// with interleaved indexing of components: that is, input
// (x0,y0,x1,y1,...,xN,yN) yields IRM[(x0,x1,...,xN)][(y0,y1,...,yN)]
NWAOBDDTopNodeRefPtr MkInverseReedMullerInterleavedTop(unsigned int i)
{
  NWAOBDDTopNodeRefPtr v;/*ETTODO 
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  assert(i <= NWAOBDDMaxLevel);

  tempHandle = MkInverseReedMullerInterleavedNode(i);
  m.AddToEnd(1);
  m.AddToEnd(0);
  m.AddToEnd(2);
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);*/
  return v;
}

//ETTODO StepFunctions
// Create representation of step function
NWAOBDDTopNodeRefPtr MkStepUpOneFourthTop()
{
  NWAOBDDTopNodeRefPtr v;/*ETTODO
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  assert(NWAOBDDMaxLevel >= 1);
  tempHandle = MkStepOneFourth(NWAOBDDMaxLevel);
  m.AddToEnd(0);
  m.AddToEnd(1);
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);*/
  return v;
}

// Create representation of step function
NWAOBDDTopNodeRefPtr MkStepDownOneFourthTop()
{
  return MkNot(MkStepUpOneFourthTop());
}

//ETTODO Step Functions
#ifdef ARBITRARY_STEP_FUNCTIONS
// Create representation of step function
NWAOBDDTopNodeRefPtr MkStepUpTop(unsigned int i)
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  assert(NWAOBDDMaxLevel <= 5);
  tempHandle = MkStepNode(NWAOBDDMaxLevel, i, 0, (1 << (1 << NWAOBDDMaxLevel)) - i);
  if (i != 0) {
    m.AddToEnd(0);
  }
  if (i != (1 << (1 << NWAOBDDMaxLevel))) {
    m.AddToEnd(1);
  }
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);
  return v;
}

// Create representation of step function
NWAOBDDTopNodeRefPtr MkStepDownTop(unsigned int i)
{
  return MkNot(MkStepUpTop(i));
}
#endif

// Unary operations on NWAOBDDTopNodes --------------------------------------

// Implements \f.!f
NWAOBDDTopNodeRefPtr MkNot(NWAOBDDTopNodeRefPtr f)
{
  NWAOBDDTopNodeRefPtr answer;
  ReturnMapHandle<intpair> m = f->rootConnection.returnMapHandle.Complement();

  answer = new NWAOBDDTopNode(f->rootConnection.entryPointHandle, m);
  return answer;
}

// Binary operations on NWAOBDDTopNodes ------------------------------------

// \f.\g.(f && g)
NWAOBDDTopNodeRefPtr MkAnd(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, andOp);
}

// \f.\g.!(f && g)
NWAOBDDTopNodeRefPtr MkNand(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, nandOp);
}

// \f.\g.(f || g)
NWAOBDDTopNodeRefPtr MkOr(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, orOp);
}

// \f.\g.!(f || g)
NWAOBDDTopNodeRefPtr MkNor(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, norOp);
}

// \f.\g.(f == g)
NWAOBDDTopNodeRefPtr MkIff(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, iffOp);
}

// \f.\g.(f != g)
NWAOBDDTopNodeRefPtr MkExclusiveOr(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, exclusiveOrOp);
}

// \f.\g.(!f || g)
NWAOBDDTopNodeRefPtr MkImplies(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, impliesOp);
}

// \f.\g.(f && !g)
NWAOBDDTopNodeRefPtr MkMinus(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, minusOp);
}

// \f.\g.(!g || f)
NWAOBDDTopNodeRefPtr MkQuotient(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, quotientOp);
}

// \f.\g.(g && !f)
NWAOBDDTopNodeRefPtr MkNotQuotient(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce(f, g, notQuotientOp);
}

// \f.\g.f
NWAOBDDTopNodeRefPtr MkFirst(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr )
{
  return f;
}

// \f.\g.!f
NWAOBDDTopNodeRefPtr MkNotFirst(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr )
{
  return MkNot(f);
}

// \f.\g.g
NWAOBDDTopNodeRefPtr MkSecond(NWAOBDDTopNodeRefPtr , NWAOBDDTopNodeRefPtr g)
{
  return g;
}

// \f.\g.!g
NWAOBDDTopNodeRefPtr MkNotSecond(NWAOBDDTopNodeRefPtr , NWAOBDDTopNodeRefPtr g)
{
  return MkNot(g);
}

// N-ary operations on NWAOBDDTopNodes -----------------------------------

// \f1. ... \fk.(f1 && ... && fk)
NWAOBDDTopNodeRefPtr  MkAnd(int N, ...)
{
  NWAOBDDTopNodeRefPtr  temp;

  assert(N >= 2);
  va_list ap;
  va_start(ap, N);
  temp = va_arg(ap, NWAOBDDTopNodeRefPtr );
  for (int k = 1; k < N; k++) {
    temp = MkAnd(temp, va_arg(ap, NWAOBDDTopNodeRefPtr ));
  }
  va_end(ap);
  return temp;
}

// \f1. ... \fk.!(f1 && ... && fk)
NWAOBDDTopNodeRefPtr  MkNand(int N, ...)
{
  NWAOBDDTopNodeRefPtr  temp;

  assert(N >= 2);
  va_list ap;
  va_start(ap, N);
  temp = va_arg(ap, NWAOBDDTopNodeRefPtr );
  for (int k = 1; k < N; k++) {
    temp = MkAnd(temp, va_arg(ap, NWAOBDDTopNodeRefPtr ));
  }
  va_end(ap);
  return MkNot(temp);
}

// \f1. ... \fk.(f1 || ... || fk)
NWAOBDDTopNodeRefPtr  MkOr(int N, ...)
{
  NWAOBDDTopNodeRefPtr  temp;

  assert(N >= 2);
  va_list ap;
  va_start(ap, N);
  temp = va_arg(ap, NWAOBDDTopNodeRefPtr );
  for (int k = 1; k < N; k++) {
    temp = MkOr(temp, va_arg(ap, NWAOBDDTopNodeRefPtr ));
  }
  va_end(ap);
  return temp;
}

// \f1. ... \fk.!(f1 || ... || fk)
NWAOBDDTopNodeRefPtr  MkNor(int N, ...)
{
  NWAOBDDTopNodeRefPtr  temp;

  assert(N >= 2);
  va_list ap;
  va_start(ap, N);
  temp = va_arg(ap, NWAOBDDTopNodeRefPtr );
  for (int k = 1; k < N; k++) {
    temp = MkOr(temp, va_arg(ap, NWAOBDDTopNodeRefPtr ));
  }
  va_end(ap);
  return MkNot(temp);
}

// Ternary operations on NWAOBDDTopNodes ------------------------------------

// \a.\b.\c.(a && b) || (!a && c)
NWAOBDDTopNodeRefPtr MkIfThenElse(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g, NWAOBDDTopNodeRefPtr h)
{
  return ApplyAndReduce(f, g, h, ifThenElseOp);
}

// \a.\b.\c.(b && !a) || (c && !a) || (b && c)
NWAOBDDTopNodeRefPtr MkNegMajority(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g, NWAOBDDTopNodeRefPtr h)
{
  return ApplyAndReduce(f, g, h, negMajorityOp);
}

// Create representation of \f . exists x_i : f
NWAOBDDTopNodeRefPtr MkExists(NWAOBDDTopNodeRefPtr f, unsigned int i)
{
  NWAOBDDTopNodeRefPtr tempTrue = MkRestrict(f, i, true);
  NWAOBDDTopNodeRefPtr tempFalse = MkRestrict(f, i, false);
  return MkOr(tempTrue, tempFalse);
}

// Create representation of \f . forall x_i : f
NWAOBDDTopNodeRefPtr MkForall(NWAOBDDTopNodeRefPtr f, unsigned int i)
{
  NWAOBDDTopNodeRefPtr tempTrue = MkRestrict(f, i, true);
  NWAOBDDTopNodeRefPtr tempFalse = MkRestrict(f, i, false);
  return MkAnd(tempTrue, tempFalse);
}

//ETTODO count++ count--?
NWAOBDDTopNodeRefPtr MkComposeTop(NWAOBDDTopNodeRefPtr f, int i, NWAOBDDTopNodeRefPtr g)              // \f. f | x_i = g
{
	  // DLC inefficient

  // Simple but slow method, see Bryant 1986 _GBAfBFM_:
  
  // f|x_i=g is g*(f|x_i=1) + (!g)*(f|x_i=0)

  NWAOBDDTopNodeRefPtr f__x_i_1 = MkRestrict(f, i, true);
  NWAOBDDTopNodeRefPtr f__x_i_0 = MkRestrict(f, i, false);
  NWAOBDDTopNodeRefPtr not_g = MkNot(g);


  NWAOBDDTopNodeRefPtr left_product = MkAnd(g, f__x_i_1);


  NWAOBDDTopNodeRefPtr right_product = MkAnd(not_g, f__x_i_0);

  NWAOBDDTopNodeRefPtr composition = MkOr(left_product, right_product);
 
  return composition;
}

// Unary operations on NWAOBDDBaseNodes --------------------------------------

// Implements \f.!f
NWAOBDDBaseNodeRefPtr MkNot(NWAOBDDBaseNodeRefPtr f)
{
	NWAOBDDBaseNodeRefPtr answer;
	ReturnMapHandle<intpair> m = f->rootConnection.returnMapHandle.Complement();

	answer = new NWAOBDDBaseNode(f->rootConnection.entryPointHandle, m);
	return answer;
}

// Binary operations on NWAOBDDBaseNodes ------------------------------------

// \f.\g.(f && g)
NWAOBDDBaseNodeRefPtr MkAnd(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return ApplyAndReduce(f, g, andOp);
}

// \f.\g.!(f && g)
NWAOBDDBaseNodeRefPtr MkNand(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return ApplyAndReduce(f, g, nandOp);
}

// \f.\g.(f || g)
NWAOBDDBaseNodeRefPtr MkOr(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return ApplyAndReduce(f, g, orOp);
}


// \f.\g.!(f || g)
NWAOBDDBaseNodeRefPtr MkNor(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return ApplyAndReduce(f, g, norOp);
}

// \f.\g.(f == g)
NWAOBDDBaseNodeRefPtr MkIff(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return ApplyAndReduce(f, g, iffOp);
}

// \f.\g.(f != g)
NWAOBDDBaseNodeRefPtr MkExclusiveOr(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level); 
	return ApplyAndReduce(f, g, exclusiveOrOp);
}

// \f.\g.(!f || g)
NWAOBDDBaseNodeRefPtr MkImplies(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return ApplyAndReduce(f, g, impliesOp);
	
}

// \f.\g.(f && !g)
NWAOBDDBaseNodeRefPtr MkMinus(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level); 
	return ApplyAndReduce(f, g, minusOp);
}

// \f.\g.(!g || f)
NWAOBDDBaseNodeRefPtr MkQuotient(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return ApplyAndReduce(f, g, quotientOp);
}

// \f.\g.(g && !f)
NWAOBDDBaseNodeRefPtr MkNotQuotient(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return ApplyAndReduce(f, g, notQuotientOp);
}

// \f.\g.f
NWAOBDDBaseNodeRefPtr MkFirst(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return f;
}

// \f.\g.!f
NWAOBDDBaseNodeRefPtr MkNotFirst(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return MkNot(f);
}

// \f.\g.g
NWAOBDDBaseNodeRefPtr MkSecond(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return g;
}

// \f.\g.g
NWAOBDDBaseNodeRefPtr MkNotSecond(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g)
{
	assert(f->level == g->level);
	return MkNot(g);
}

// Ternary operations on NWAOBDDBaseNodeRefPtrs ------------------------------------

// \a.\b.\c.(a && b) || (!a && c)
NWAOBDDBaseNodeRefPtr MkIfThenElse(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g, NWAOBDDBaseNodeRefPtr h)
{
	assert(f->level == g->level && f->level == g->level);
	return ApplyAndReduce(f, g, h, ifThenElseOp);
}

// \a.\b.\c.(b && !a) || (c && !a) || (b && c)
NWAOBDDBaseNodeRefPtr MkNegMajority(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g, NWAOBDDBaseNodeRefPtr h)
{
	assert(f->level == g->level && f->level == g->level);
	return ApplyAndReduce(f, g, h, negMajorityOp);
}

// Create representation of \f . exists x_i : f
NWAOBDDBaseNodeRefPtr MkExists(NWAOBDDBaseNodeRefPtr f, unsigned int i)
{
	NWAOBDDBaseNodeRefPtr tempTrue = MkRestrict(f, i, true);
	NWAOBDDBaseNodeRefPtr tempFalse = MkRestrict(f, i, false);
	return MkOr(tempTrue, tempFalse);
}

// Create representation of \f . forall x_i : f
NWAOBDDBaseNodeRefPtr MkForall(NWAOBDDBaseNodeRefPtr f, unsigned int i)
{
	NWAOBDDBaseNodeRefPtr tempTrue = MkRestrict(f, i, true);
	NWAOBDDBaseNodeRefPtr tempFalse = MkRestrict(f, i, false);
	return MkAnd(tempTrue, tempFalse);
}

NWAOBDDBaseNodeRefPtr MkComposeBase(NWAOBDDBaseNodeRefPtr f, int i, NWAOBDDBaseNodeRefPtr g)              // \f. f | x_i = g
{
	// DLC inefficient

	// Simple but slow method, see Bryant 1986 _GBAfBFM_:

	// f|x_i=g is g*(f|x_i=1) + (!g)*(f|x_i=0)

	NWAOBDDBaseNodeRefPtr f__x_i_1 = MkRestrict(f, i, true);
	NWAOBDDBaseNodeRefPtr f__x_i_0 = MkRestrict(f, i, false);
	NWAOBDDBaseNodeRefPtr not_g = MkNot(g);


	NWAOBDDBaseNodeRefPtr left_product = MkAnd(g, f__x_i_1);


	NWAOBDDBaseNodeRefPtr right_product = MkAnd(not_g, f__x_i_0);

	NWAOBDDBaseNodeRefPtr composition = MkOr(left_product, right_product);

	return composition;
}


// Restrict -----------------------------------------------------------

NWAOBDDNodeHandle Restrict(NWAOBDDNodeHandle g, unsigned int i, bool val,
                           ReturnMapHandle<int> &MapHandle
                          )
{
    NWAOBDDNodeHandle answer;
  
    if (g.handleContents->NodeKind() == NWAOBDD_INTERNAL) {
      answer = Restrict((NWAOBDDInternalNode *)g.handleContents, i, val,
                        MapHandle
                       );
    }
    else { /* g.handleContents->NodeKind() == NWAOBDD_EPSILON */
      MapHandle.AddToEnd(0);
      MapHandle.Canonicalize();
      answer = g;
    }
    return answer;
}

NWAOBDDNodeHandle Restrict(NWAOBDDInternalNode *g, unsigned int i, bool val,
                           ReturnMapHandle<int> &MapHandle
                          )
{

  if (g == NWAOBDDNodeHandle::NoDistinctionNode[g->level].handleContents) {
    MapHandle.AddToEnd(0);
    MapHandle.Canonicalize();
    return NWAOBDDNodeHandle(g);
  }

  unsigned int size = ((unsigned int)((((unsigned int)1) << (g->level + 2)) - (unsigned int)4));
  ReturnMapHandle<intpair> AMap;
  NWAOBDDInternalNode *n = new NWAOBDDInternalNode(g->level);
  unsigned int curExit = 0;

  if (i == 0)  { //i is outbound a variable
	  n->AConnection[0].entryPointHandle = g->AConnection[val].entryPointHandle;
	  n->AConnection[1].entryPointHandle = g->AConnection[val].entryPointHandle;
	  
	  int max = 0;
	  //iterator through the entries in the 0 A Connection's Return Map
	  AMap = g->AConnection[val].returnMapHandle;
	  ReturnMapHandle<int> BList;
	  ReturnMapHandle<intpair> ARet;
	  
	  unsigned aMapSize = AMap.mapContents->mapArray.size();
	  for (unsigned sAIT = 0; sAIT < aMapSize; sAIT++)
	  {
		  intpair b = AMap.mapContents->mapArray[sAIT];
		  int b0 = b.First();
		  int b1 = b.Second();
		  int index0, index1;
		  if (BList.Member(b0)) {
			  index0 = BList.LookupInv(b0);
		  }
		  else {
			  BList.AddToEnd(b0);
			  index0 = max;
			  max++;
		  }
		  if (BList.Member(b1)) {
			  index1 = BList.LookupInv(b0);
		  }
		  else {
			  BList.AddToEnd(b1);
			  index1 = max;
			  max++;
		  }
		  ARet.AddToEnd(intpair(index0, index1));
	  }

	  n->BConnection[0] = new Connection[max]; //May be reduced later
	  n->BConnection[1] = new Connection[max]; //May be reduced later
	  
	  ARet.Canonicalize();
      n->AConnection[0].returnMapHandle = ARet;
	  n->AConnection[1].returnMapHandle = ARet;
	  n->numBConnections = max;
	  for (unsigned sAI = 0; sAI < max; sAI++)
	  {
		  int midB = BList.Lookup(sAI);
		  n->BConnection[0][sAI].entryPointHandle = NWAOBDDNodeHandle(g->BConnection[0][midB].entryPointHandle);
		  n->BConnection[1][sAI].entryPointHandle = NWAOBDDNodeHandle(g->BConnection[1][midB].entryPointHandle);
		  for (unsigned int l = 0; l < 2; l++)
		  {
			  ReturnMapHandle<intpair> BMap = g->BConnection[l][midB].returnMapHandle;
			  unsigned bMapSize = BMap.mapContents->mapArray.size();
			  for (unsigned sBI = 0; sBI < bMapSize; sBI++)
			  {
				  intpair c = BMap.mapContents->mapArray[sBI];
				  int c0 = c.First();
				  int c1 = c.Second();

				  int endIndex0;
				  int endIndex1;

				  if (MapHandle.Member(c0))
				  {
					  endIndex0 = MapHandle.LookupInv(c0);
				  } else {
					  MapHandle.AddToEnd(c0);
					  endIndex0 = curExit;
						  curExit++;
				  }

				  if (MapHandle.Member(c1))
				  {
					  endIndex1 = MapHandle.LookupInv(c1);
				  } else {
					  MapHandle.AddToEnd(c1);
					  endIndex1 = curExit;
					  curExit++;
				  }

				  n->BConnection[l][sAI].returnMapHandle.AddToEnd(intpair(endIndex0,endIndex1));
			  }
			  n->BConnection[l][sAI].returnMapHandle.Canonicalize();
		  }
	  }

  }
  else if (i == (size / 2 - 1)) { //The AConnections remain the same, but the 0 and 1 return values for the maps become the same
	  ReturnMapHandle<intpair> AMap0 = g->AConnection[0].returnMapHandle;
	  ReturnMapHandle<intpair> AMap1 = g->AConnection[1].returnMapHandle;

	  ReturnMapHandle<int> BList;
	  ReturnMapHandle<intpair> ARet0;
	  ReturnMapHandle<intpair> ARet1;

	  unsigned aMapSize = AMap0.mapContents->mapArray.size();
	  int max = 0;
	  int bret;
	  for (unsigned sAIT = 0; sAIT < aMapSize; sAIT++)
	  {
		  intpair b = AMap0.mapContents->mapArray[sAIT];
		  if (val)
		  {
			  bret = b.Second();
		  }
		  else
		  {
			  bret = b.First();
		  }
		  int index;
		  if (BList.Member(bret)) {
			  index = BList.LookupInv(bret);
		  }
		  else {
			  BList.AddToEnd(bret);
			  index = max;
			  max++;
		  }
		  ARet0.AddToEnd(intpair(index, index));
	  }

	  aMapSize = AMap1.mapContents->mapArray.size();
	  for (unsigned sAIT2 = 0; sAIT2 < aMapSize; sAIT2++)
	  {
		  intpair b = AMap1.mapContents->mapArray[sAIT2];
		  if (val)
		  {
			  bret = b.Second();
		  }
		  else
		  {
			  bret = b.First();
		  }
		  int index;
		  if (BList.Member(bret)) {
			  index = BList.LookupInv(bret);
		  }
		  else {
			  BList.AddToEnd(bret);
			  index = max;
			  max++;
		  }
		  ARet1.AddToEnd(intpair(index, index));
	  }


	  ARet0.Canonicalize();
	  ARet1.Canonicalize();
	  n->AConnection[0].entryPointHandle = g->AConnection[0].entryPointHandle;
	  n->AConnection[1].entryPointHandle = g->AConnection[1].entryPointHandle;
	  n->AConnection[0].returnMapHandle = ARet0;
	  n->AConnection[1].returnMapHandle = ARet1;

	  n->BConnection[0] = new Connection[max];
	  n->BConnection[1] = new Connection[max];
	  n->numBConnections = max;

	  for (unsigned sAI = 0; sAI < max; sAI++)
	  {
		  int midB = BList.Lookup(sAI);
		  n->BConnection[0][sAI].entryPointHandle = NWAOBDDNodeHandle(g->BConnection[0][midB].entryPointHandle);
		  n->BConnection[1][sAI].entryPointHandle = NWAOBDDNodeHandle(g->BConnection[1][midB].entryPointHandle);
		  for (unsigned int l = 0; l < 2; l++)
		  {
			  ReturnMapHandle<intpair> BMap = g->BConnection[l][midB].returnMapHandle;
			  unsigned bMapSize = BMap.mapContents->mapArray.size();
			  for (unsigned sBI = 0; sBI < bMapSize; sBI++)
			  {
				  intpair c = BMap.mapContents->mapArray[sBI];
				  int c0 = c.First();
				  int c1 = c.Second();

				  int endIndex0;
				  int endIndex1;

				  if (MapHandle.Member(c0))
				  {
					  endIndex0 = MapHandle.LookupInv(c0);
				  }
				  else {
					  MapHandle.AddToEnd(c0);
					  endIndex0 = curExit;
					  curExit++;
				  }

				  if (MapHandle.Member(c1))
				  {
					  endIndex1 = MapHandle.LookupInv(c1);
				  }
				  else {
					  MapHandle.AddToEnd(c1);
					  endIndex1 = curExit;
					  curExit++;
				  }

				  n->BConnection[l][sAI].returnMapHandle.AddToEnd(intpair(endIndex0, endIndex1));
			  }
			  n->BConnection[l][sAI].returnMapHandle.Canonicalize();
		  }
	  }
  } else if (i == size/2) {  //The outbound bvariable is restricticted to val
	  n->AConnection[0] = g->AConnection[0];
	  n->AConnection[1] = g->AConnection[1];

	  n->numBConnections = 0;

	  n->BConnection[0] = new Connection[g->numBConnections];
	  n->BConnection[1] = new Connection[g->numBConnections];
	  ReductionMapHandle AReductionMapHandle;

	  for (unsigned int j = 0; j < g->numBConnections; j++)
	  {
		  NWAOBDDNodeHandle m = g->BConnection[val][j].entryPointHandle;
		  ReturnMapHandle<intpair> inducedReturnMapHandleB;
		  ReturnMapHandle<intpair> BMap = g->BConnection[val][j].returnMapHandle;
		  unsigned bMapSize = BMap.mapContents->mapArray.size();
		  for (unsigned sBI = 0; sBI < bMapSize; sBI++)
		  {
			  intpair c = BMap.mapContents->mapArray[sBI];
			  int c0 = c.First();
			  int c1 = c.Second();
			  int index0,index1;
			  if (MapHandle.Member(c0)){
				  index0 = MapHandle.LookupInv(c0);
			  } else {
				  MapHandle.AddToEnd(c0);
				  index0 = curExit;
				  curExit++;
			  }

			  if (MapHandle.Member(c1)){
				  index1 = MapHandle.LookupInv(c1);
			  } else {
				  MapHandle.AddToEnd(c1);
				  index1 = curExit;
				  curExit++;
			  }
			  inducedReturnMapHandleB.AddToEnd(intpair(index0, index1));
		  }
		  inducedReturnMapHandleB.Canonicalize();
		  Connection candidate0(m, inducedReturnMapHandleB);
		  Connection candidate1(m, inducedReturnMapHandleB);
		  unsigned int position = n->InsertBConnection(n->numBConnections, candidate0, candidate1);
		  AReductionMapHandle.AddToEnd(position);
	  }
	  AReductionMapHandle.Canonicalize();
	  if (n->numBConnections < g->numBConnections) {  // Shorten
		  Connection *temp0 = n->BConnection[0];
		  Connection *temp1 = n->BConnection[1];
		  n->BConnection[0] = new Connection[n->numBConnections];
		  n->BConnection[1] = new Connection[n->numBConnections];
		  for (unsigned int k = 0; k < n->numBConnections; k++) {
			  n->BConnection[0][k] = temp0[k];
			  n->BConnection[1][k] = temp1[k];
		  }
		  delete[] temp0;
		  delete[] temp1;
	  }
	  ReductionMapHandle inducedA0ReductionMapHandle;
	  ReductionMapHandle inducedA1ReductionMapHandle;
	  ReturnMapHandle<intpair> inducedA0ReturnMap;
	  ReturnMapHandle<intpair> inducedA1ReturnMap;
	  ReturnMapHandle<intpair> reducedAReturnMap = n->AConnection[0].returnMapHandle.Compose(AReductionMapHandle);
	  reducedAReturnMap.InducedReductionAndReturnMap(inducedA0ReductionMapHandle, inducedA0ReturnMap);
	  NWAOBDDNodeHandle tempHandle = n->AConnection[0].entryPointHandle.Reduce(inducedA0ReductionMapHandle, inducedA0ReturnMap.Size());
	  n->AConnection[0] = Connection(tempHandle, inducedA0ReturnMap);

	  reducedAReturnMap = n->AConnection[1].returnMapHandle.Compose(AReductionMapHandle);
	  reducedAReturnMap.InducedReductionAndReturnMap(inducedA1ReductionMapHandle, inducedA1ReturnMap);
	  NWAOBDDNodeHandle tempHandle1 = n->AConnection[1].entryPointHandle.Reduce(inducedA1ReductionMapHandle, inducedA1ReturnMap.Size());
	  n->AConnection[1] = Connection(tempHandle1, inducedA1ReturnMap);
  } else if (i == size - 1) { //The B-Return is restricted to Val
	  n->AConnection[0] = g->AConnection[0];
	  n->AConnection[1] = g->AConnection[1];

	  n->numBConnections = 0;

	  n->BConnection[0] = new Connection[g->numBConnections];
	  n->BConnection[1] = new Connection[g->numBConnections];

	  ReductionMapHandle AReductionMapHandle;

	  for (unsigned int l = 0; l < g->numBConnections; l++)
	  {
			  NWAOBDDNodeHandle m0 = g->BConnection[0][l].entryPointHandle;
			  NWAOBDDNodeHandle m1 = g->BConnection[1][l].entryPointHandle;
			  ReturnMapHandle<intpair> BMap0 = g->BConnection[0][l].returnMapHandle;
			  ReturnMapHandle<intpair> BMap1 = g->BConnection[1][l].returnMapHandle; 
			  ReturnMapHandle<intpair> inducedReturnMapHandleB0, inducedReturnMapHandleB1;
			  unsigned bMapSize0 = BMap0.mapContents->mapArray.size();
			  unsigned bMapSize1 = BMap1.mapContents->mapArray.size();
			  for (unsigned sBI = 0; sBI < bMapSize0; sBI++)
			  {
				  int curValue;
				  if (val == 0)
				  {
					  curValue = BMap0.mapContents->mapArray[sBI].First();
				  } else {
					  curValue = BMap0.mapContents->mapArray[sBI].Second();
				  }
				  if (MapHandle.Member(curValue))  {
					  int index = MapHandle.LookupInv(curValue);
					  inducedReturnMapHandleB0.AddToEnd(intpair(index, index));
				  } else {
					  MapHandle.AddToEnd(curValue);
					  inducedReturnMapHandleB0.AddToEnd(intpair(curExit, curExit));
					  curExit++;
				  }
			  }
			  for (unsigned sBIN = 0; sBIN < bMapSize1; sBIN++)
			  {
				  int curValue;
				  if (val == 0)
				  {
					  curValue = BMap1.mapContents->mapArray[sBIN].First();
				  }
				  else {
					  curValue = BMap1.mapContents->mapArray[sBIN].Second();
				  }
				  if (MapHandle.Member(curValue))  {
					  int index = MapHandle.LookupInv(curValue);
					  inducedReturnMapHandleB1.AddToEnd(intpair(index, index));
				  }
				  else {
					  MapHandle.AddToEnd(curValue);
					  inducedReturnMapHandleB1.AddToEnd(intpair(curExit, curExit));
					  curExit++;
				  }
			  }
			  inducedReturnMapHandleB0.Canonicalize();
			  inducedReturnMapHandleB1.Canonicalize();
			  Connection candidate0(m0, inducedReturnMapHandleB0);
			  Connection candidate1(m1, inducedReturnMapHandleB1);
			  unsigned int position = n->InsertBConnection(n->numBConnections, candidate0, candidate1);
			  AReductionMapHandle.AddToEnd(position);
	  }

	  AReductionMapHandle.Canonicalize();
	  if (n->numBConnections < g->numBConnections) {  // Shorten
		  Connection *temp0 = n->BConnection[0];
		  Connection *temp1 = n->BConnection[1];
		  n->BConnection[0] = new Connection[n->numBConnections];
		  n->BConnection[1] = new Connection[n->numBConnections];
		  for (unsigned int j = 0; j < n->numBConnections; j++) {
			  n->BConnection[0][j] = temp0[j];
			  n->BConnection[1][j] = temp1[j];
		  }
		  delete[] temp0;
		  delete[] temp1;
	  }

	  ReductionMapHandle inducedA0ReductionMapHandle;
	  ReductionMapHandle inducedA1ReductionMapHandle;
	  ReturnMapHandle<intpair> inducedA0ReturnMap;
	  ReturnMapHandle<intpair> inducedA1ReturnMap;
	  ReturnMapHandle<intpair> reducedAReturnMap = n->AConnection[0].returnMapHandle.Compose(AReductionMapHandle);
	  reducedAReturnMap.InducedReductionAndReturnMap(inducedA0ReductionMapHandle, inducedA0ReturnMap);
	  NWAOBDDNodeHandle tempHandle = n->AConnection[0].entryPointHandle.Reduce(inducedA0ReductionMapHandle, inducedA0ReturnMap.Size());
	  n->AConnection[0] = Connection(tempHandle, inducedA0ReturnMap);

	  reducedAReturnMap = n->AConnection[1].returnMapHandle.Compose(AReductionMapHandle);
	  reducedAReturnMap.InducedReductionAndReturnMap(inducedA1ReductionMapHandle, inducedA1ReturnMap);
	  NWAOBDDNodeHandle tempHandle1 = n->AConnection[1].entryPointHandle.Reduce(inducedA1ReductionMapHandle, inducedA1ReturnMap.Size());
	  n->AConnection[1] = Connection(tempHandle1, inducedA1ReturnMap);
  } else if (i > (size/2)) { //The var is located in the B-Connection
	  n->AConnection[0] = g->AConnection[0];
	  n->AConnection[1] = g->AConnection[1];
	  n->numBConnections = 0;
	  n->BConnection[0] = new Connection[g->numBConnections];
	  n->BConnection[1] = new Connection[g->numBConnections];
	  ReductionMapHandle AReductionMapHandle;
	  for (unsigned int l = 0; l < g->numBConnections; l++)
	  {
		  ReturnMapHandle<int> BRedMap0;
		  ReturnMapHandle<int> BRedMap1;
		  ReturnMapHandle<intpair> InducedReturnMapB0, InducedReturnMapB1;
		  NWAOBDDNodeHandle m0 = Restrict(g->BConnection[0][l].entryPointHandle, i - (size / 2) - 1, val, BRedMap0);
		  NWAOBDDNodeHandle m1 = Restrict(g->BConnection[1][l].entryPointHandle, i - (size / 2) - 1, val, BRedMap1);
		  for (unsigned int j = 0; j < BRedMap0.Size(); j++)
		  {
			  int retPoint = BRedMap0.Lookup(j);
			  int c0 = g->BConnection[0][l].returnMapHandle.Lookup(retPoint).First();
			  int c1 = g->BConnection[0][l].returnMapHandle.Lookup(retPoint).Second();
			  int index0, index1;
			  if (MapHandle.Member(c0)) {
				  index0 = MapHandle.LookupInv(c0);
			  }
			  else {
				  index0 = curExit;
				  MapHandle.AddToEnd(c0);
				  curExit++;
			  }
			  if (MapHandle.Member(c1)) {
				  index1 = MapHandle.LookupInv(c1);
			  }
			  else {
				  index1 = curExit;
				  MapHandle.AddToEnd(c1);
				  curExit++;
			  }

			  InducedReturnMapB0.AddToEnd(intpair(index0, index1));
		  }

		  for (unsigned int j = 0; j < BRedMap1.Size(); j++)
		  {
			  int retPoint = BRedMap1.Lookup(j);
			  int c0 = g->BConnection[1][l].returnMapHandle.Lookup(retPoint).First();
			  int c1 = g->BConnection[1][l].returnMapHandle.Lookup(retPoint).Second();
			  int index0, index1;
			  if (MapHandle.Member(c0)) {
				  index0 = MapHandle.LookupInv(c0);
			  }
			  else {
				  index0 = curExit;
				  MapHandle.AddToEnd(c0);
				  curExit++;
			  }
			  if (MapHandle.Member(c1)) {
				  index1 = MapHandle.LookupInv(c1);
			  }
			  else {
				  index1 = curExit;
				  MapHandle.AddToEnd(c1);
				  curExit++;
			  }

			  InducedReturnMapB1.AddToEnd(intpair(index0, index1));
		  }

		  InducedReturnMapB0.Canonicalize();
		  InducedReturnMapB1.Canonicalize();
		  Connection candidate0(m0, InducedReturnMapB0);
		  Connection candidate1(m1, InducedReturnMapB1);
		  unsigned int position = n->InsertBConnection(n->numBConnections, candidate0, candidate1);
		  AReductionMapHandle.AddToEnd(position);

	  }
			  AReductionMapHandle.Canonicalize();
			  if (n->numBConnections < g->numBConnections) {  // Shorten
				  Connection *temp0 = n->BConnection[0];
				  Connection *temp1 = n->BConnection[1];
				  n->BConnection[0] = new Connection[n->numBConnections];
				  n->BConnection[1] = new Connection[n->numBConnections];
				  for (unsigned int j = 0; j < n->numBConnections; j++) {
					  n->BConnection[0][j] = temp0[j];
					  n->BConnection[1][j] = temp1[j];
				  }
				  delete[] temp0;
				  delete[] temp1;
			  }

			  ReductionMapHandle inducedA0ReductionMapHandle;
			  ReductionMapHandle inducedA1ReductionMapHandle;
			  ReturnMapHandle<intpair> inducedA0ReturnMap;
			  ReturnMapHandle<intpair> inducedA1ReturnMap;
			  ReturnMapHandle<intpair> reducedAReturnMap = n->AConnection[0].returnMapHandle.Compose(AReductionMapHandle);
			  reducedAReturnMap.InducedReductionAndReturnMap(inducedA0ReductionMapHandle, inducedA0ReturnMap);
			  NWAOBDDNodeHandle tempHandle = n->AConnection[0].entryPointHandle.Reduce(inducedA0ReductionMapHandle, inducedA0ReturnMap.Size());
			  n->AConnection[0] = Connection(tempHandle, inducedA0ReturnMap);

			  reducedAReturnMap = n->AConnection[1].returnMapHandle.Compose(AReductionMapHandle);
			  reducedAReturnMap.InducedReductionAndReturnMap(inducedA1ReductionMapHandle, inducedA1ReturnMap);
			  NWAOBDDNodeHandle tempHandle1 = n->AConnection[1].entryPointHandle.Reduce(inducedA1ReductionMapHandle, inducedA1ReturnMap.Size());
			  n->AConnection[1] = Connection(tempHandle1, inducedA1ReturnMap);  
  } else { //The var is located in the A-Connections

	  ReturnMapHandle<int> BList;
	  int numB = 0;
	  curExit = 0;
	  ReturnMapHandle<int> AMap0,AMap1;
		  n->AConnection[0].entryPointHandle = Restrict(g->AConnection[0].entryPointHandle, i - 1, val, AMap0);
		  for (unsigned int j = 0; j < AMap0.Size(); j++)
		  {
			  int retPoint = AMap0.Lookup(j);
			  int b0 = g->AConnection[0].returnMapHandle.Lookup(retPoint).First();
			  int b1 = g->AConnection[0].returnMapHandle.Lookup(retPoint).Second();
			  int index0, index1;
			  if (BList.Member(b0))
			  {
				  index0 = BList.LookupInv(b0);
			  }
			  else {
				  index0 = numB;
				  BList.AddToEnd(b0);
				  numB++;
			  } if (BList.Member(b1)){
				  index1 = BList.LookupInv(b1);
			  }
			  else {
				  index1 = numB;
				  BList.AddToEnd(b1);
				  numB++;
			  }
			  n->AConnection[0].returnMapHandle.AddToEnd(intpair(index0, index1));
		  }

		  n->AConnection[1].entryPointHandle = Restrict(g->AConnection[1].entryPointHandle, i - 1, val, AMap1);
		  for (unsigned int j = 0; j < AMap1.Size(); j++)
		  {
			  int retPoint = AMap1.Lookup(j);
			  int b0 = g->AConnection[1].returnMapHandle.Lookup(retPoint).First();
			  int b1 = g->AConnection[1].returnMapHandle.Lookup(retPoint).Second();
			  int index0, index1;
			  if (BList.Member(b0))
			  {
				  index0 = BList.LookupInv(b0);
			  }
			  else {
				  index0 = numB;
				  BList.AddToEnd(b0);
				  numB++;
			  } if (BList.Member(b1)){
				  index1 = BList.LookupInv(b1);
			  }
			  else {
				  index1 = numB;
				  BList.AddToEnd(b1);
				  numB++;
			  }
			  n->AConnection[1].returnMapHandle.AddToEnd(intpair(index0, index1));
		  }
		n->AConnection[0].returnMapHandle.Canonicalize();
		n->AConnection[1].returnMapHandle.Canonicalize();

		n->numBConnections = numB;
		n->BConnection[0] = new Connection[n->numBConnections];
		n->BConnection[1] = new Connection[n->numBConnections];
		for (unsigned fin = 0; fin < numB; fin++)
		{
		  int midB = BList.Lookup(fin);
		  n->BConnection[0][fin].entryPointHandle = NWAOBDDNodeHandle(g->BConnection[0][midB].entryPointHandle);
		  n->BConnection[1][fin].entryPointHandle = NWAOBDDNodeHandle(g->BConnection[1][midB].entryPointHandle);
		  for (unsigned int l = 0; l < 2; l++)
		  {
			  ReturnMapHandle<intpair> BMap = g->BConnection[l][midB].returnMapHandle;
			  unsigned bMapSize = BMap.mapContents->mapArray.size();
			  for (unsigned sBI = 0; sBI < bMapSize; sBI++)
			  {
				  intpair c = BMap.mapContents->mapArray[sBI];
				  int c0 = c.First();
				  int c1 = c.Second();

				  int endIndex0;
				  int endIndex1;

				  if (MapHandle.Member(c0))
				  {
					  endIndex0 = MapHandle.LookupInv(c0);
				  }
				  else {
					  MapHandle.AddToEnd(c0);
					  endIndex0 = curExit;
					  curExit++;
				  }

				  if (MapHandle.Member(c1))
				  {
					  endIndex1 = MapHandle.LookupInv(c1);
				  }
				  else {
					  MapHandle.AddToEnd(c1);
					  endIndex1 = curExit;
					  curExit++;
				  }

				  n->BConnection[l][fin].returnMapHandle.AddToEnd(intpair(endIndex0, endIndex1));
			  }
			  n->BConnection[l][fin].returnMapHandle.Canonicalize();
		  }
		}
  } 
  n->numExits = curExit;
  return NWAOBDDNodeHandle(n);
}
}
namespace NWA_OBDD {

NWAOBDDTopNodeRefPtr MkSchemaAdjust(NWAOBDDTopNodeRefPtr n, int s[])
{
	NWAOBDDTopNodeRefPtr g = NWAOBDDNodeHandle::SchemaAdjust(n->rootConnection.entryPointHandle, 1, s, 0);
	return g;
}

NWAOBDDTopNodeRefPtr MkPathSummary(NWAOBDDTopNodeRefPtr n)
{
	NWAOBDDTopNodeRefPtr g = NWAOBDDNodeHandle::PathSummary(n->rootConnection.entryPointHandle, 1, 0);
	return g;
}

NWAOBDDTopNodeRefPtr MkRestrict(NWAOBDDTopNodeRefPtr n, unsigned int i, bool val)
{
  ReturnMapHandle<int> MapHandle;
  NWAOBDDNodeHandle g = Restrict(n->rootConnection.entryPointHandle, i, val,
                                 MapHandle);
  g.Canonicalize();

  // Create returnMapHandle from MapHandle
     ReturnMapHandle<intpair> returnMapHandle;
	 unsigned mapSize = MapHandle.mapContents->mapArray.size();
     for (unsigned i = 0; i <mapSize; i++) {
		 int d = MapHandle.mapContents->mapArray[i];
       int c = n->rootConnection.returnMapHandle.Lookup(d).First();
       returnMapHandle.AddToEnd(intpair(c,c));
     }
     returnMapHandle.Canonicalize();

  // Create and return NWAOBDDTopNode

     return(new NWAOBDDTopNode(g, returnMapHandle));
}

// ApplyAndReduce -----------------------------------------------------------
NWAOBDDTopNodeRefPtr ApplyAndReduce(NWAOBDDTopNodeRefPtr n1,
                               NWAOBDDTopNodeRefPtr n2,
                               BoolOp op)
{
  // Perform 2-way cross product of n1 and n2
     PairProductMapHandle MapHandle;
     NWAOBDDNodeHandle n = PairProduct(n1->rootConnection.entryPointHandle,
                                       n2->rootConnection.entryPointHandle,
                                       MapHandle);
  // Create returnMapHandle from MapHandle: Fold the pairs in MapHandle by applying
  // [n1->rootConnection.returnMapHandle, n2->rootConnection.returnMapHandle]
  // (component-wise) to each pair.
     ReturnMapHandle<intpair> returnMapHandle;
     PairProductMapBodyIterator MapIterator(*MapHandle.mapContents);
     MapIterator.Reset();
     while (!MapIterator.AtEnd()) {
       int c1, c2;
       int first, second;
       first = MapIterator.Current().First();
       second = MapIterator.Current().Second();
       c1 = n1->rootConnection.returnMapHandle.Lookup(first).First();
       c2 = n2->rootConnection.returnMapHandle.Lookup(second).First();
	   int r = op[c1][c2];
       returnMapHandle.AddToEnd(intpair(r,r));
       MapIterator.Next();
     }
     returnMapHandle.Canonicalize();

  // Perform reduction on n, with respect to the common elements that returnMapHandle maps together
     ReductionMapHandle inducedReductionMapHandle;
     ReturnMapHandle<intpair> inducedReturnMap;
     returnMapHandle.InducedReductionAndReturnMap(inducedReductionMapHandle, inducedReturnMap);
     //     NWAOBDDNodeHandle::InitReduceCache();
     NWAOBDDNodeHandle reduced_n = n.Reduce(inducedReductionMapHandle, inducedReturnMap.Size());
     //     NWAOBDDNodeHandle::DisposeOfReduceCache();

  // Create and return NWAOBDDTopNode
   return(new NWAOBDDTopNode(reduced_n, inducedReturnMap));
}


NWAOBDDBaseNodeRefPtr MkRestrict(NWAOBDDBaseNodeRefPtr n, unsigned int i, bool val)
{
	ReturnMapHandle<int> MapHandle;
	NWAOBDDNodeHandle g = Restrict(n->rootConnection.entryPointHandle, i, val,
		MapHandle);
	g.Canonicalize();

	// Create returnMapHandle from MapHandle
	ReturnMapHandle<intpair> returnMapHandle;
	unsigned mapSize = MapHandle.mapContents->mapArray.size();
	for (unsigned i = 0; i <mapSize; i++) {
		int d = MapHandle.mapContents->mapArray[i];
		int c = n->rootConnection.returnMapHandle.Lookup(d).First();
		returnMapHandle.AddToEnd(intpair(c, c));
	}
	returnMapHandle.Canonicalize();

	// Create and return NWAOBDDTopNode

	return(new NWAOBDDBaseNode(g, returnMapHandle));
}

// ApplyAndReduce -----------------------------------------------------------
NWAOBDDBaseNodeRefPtr ApplyAndReduce(NWAOBDDBaseNodeRefPtr n1,
	NWAOBDDBaseNodeRefPtr n2,
	BoolOp op)
{
	// Perform 2-way cross product of n1 and n2
	PairProductMapHandle MapHandle;
	NWAOBDDNodeHandle n = PairProduct(n1->rootConnection.entryPointHandle,
		n2->rootConnection.entryPointHandle,
		MapHandle);

	// Create returnMapHandle from MapHandle: Fold the pairs in MapHandle by applying
	// [n1->rootConnection.returnMapHandle, n2->rootConnection.returnMapHandle]
	// (component-wise) to each pair.
	ReturnMapHandle<intpair> returnMapHandle;
	PairProductMapBodyIterator MapIterator(*MapHandle.mapContents);
	MapIterator.Reset();
	while (!MapIterator.AtEnd()) {
		int c1, c2;
		int first, second;
		first = MapIterator.Current().First();
		second = MapIterator.Current().Second();
		c1 = n1->rootConnection.returnMapHandle.Lookup(first).First();
		c2 = n2->rootConnection.returnMapHandle.Lookup(second).First();
		int r = op[c1][c2];
		returnMapHandle.AddToEnd(intpair(r, r));
		MapIterator.Next();
	}
	returnMapHandle.Canonicalize();

	// Perform reduction on n, with respect to the common elements that returnMapHandle maps together
	ReductionMapHandle inducedReductionMapHandle;
	ReturnMapHandle<intpair> inducedReturnMap;
	returnMapHandle.InducedReductionAndReturnMap(inducedReductionMapHandle, inducedReturnMap);
	//     NWAOBDDNodeHandle::InitReduceCache();
	NWAOBDDNodeHandle reduced_n = n.Reduce(inducedReductionMapHandle, inducedReturnMap.Size());
	//     NWAOBDDNodeHandle::DisposeOfReduceCache();

	// Create and return NWAOBDDTopNode
	//ETTODO
	return(new NWAOBDDBaseNode(reduced_n, inducedReturnMap));
}

//ETTODO Ternary ApplyAndReduce
NWAOBDDTopNodeRefPtr ApplyAndReduce(NWAOBDDTopNodeRefPtr n1,
                               NWAOBDDTopNodeRefPtr n2,
                               NWAOBDDTopNodeRefPtr n3,
                               BoolOp3 op)
{/*ETTODO
  // Perform 3-way cross product of n1, n2, and n3
     TripleProductMapHandle MapHandle;
     NWAOBDDNodeHandle n = TripleProduct(n1->rootConnection.entryPointHandle,
                                         n2->rootConnection.entryPointHandle,
                                         n3->rootConnection.entryPointHandle,
                                         MapHandle);

  // Create returnMapHandle from MapHandle: Fold the pairs in MapHandle by applying
  // [n1->rootConnection.returnMapHandle, n2->rootConnection.returnMapHandle, n3->rootConnection.returnMapHandle]
  // (component-wise) to each triple.
     ReturnMapHandle returnMapHandle;
     TripleProductMapBodyIterator MapIterator(*MapHandle.mapContents);
     MapIterator.Reset();
     while (!MapIterator.AtEnd()) {
       int c1, c2, c3;
       int first, second, third;
       first = MapIterator.Current().First();
       second = MapIterator.Current().Second();
       third = MapIterator.Current().Third();
       c1 = n1->rootConnection.returnMapHandle[0].Lookup(first);
       c2 = n2->rootConnection.returnMapHandle[0].Lookup(second);
       c3 = n3->rootConnection.returnMapHandle[0].Lookup(third);
       returnMapHandle.AddToEnd(op[c1][c2][c3]);
       MapIterator.Next();
     }
    returnMapHandle.Canonicalize();

  // Perform reduction on n, with respect to the common elements that returnMapHandle maps together
     ReductionMapHandle inducedReductionMapHandle;
     ReturnMapHandle inducedReturnMap;
     returnMapHandle.InducedReductionAndReturnMap(inducedReductionMapHandle, inducedReturnMap);
     //     NWAOBDDNodeHandle::InitReduceCache();
     NWAOBDDNodeHandle reduced_n = n.Reduce(inducedReductionMapHandle, inducedReturnMap.Size());
     //     NWAOBDDNodeHandle::DisposeOfReduceCache();

     //ETTODO 
     ReturnMapHandle rm[2];
     rm[0] = inducedReturnMap;
     rm[1] = inducedReturnMap;
  // Create and return NWAOBDDTopNode
     return(new NWAOBDDTopNode(reduced_n, rm));*/
	 NWAOBDDTopNodeRefPtr  temp;
	 return temp;
}


//ETTODO Ternary ApplyAndReduce
NWAOBDDBaseNodeRefPtr ApplyAndReduce(NWAOBDDBaseNodeRefPtr n1,
	NWAOBDDBaseNodeRefPtr n2,
	NWAOBDDBaseNodeRefPtr n3,
	BoolOp3 op)
{/*ETTODO
 // Perform 3-way cross product of n1, n2, and n3
 TripleProductMapHandle MapHandle;
 NWAOBDDNodeHandle n = TripleProduct(n1->rootConnection.entryPointHandle,
 n2->rootConnection.entryPointHandle,
 n3->rootConnection.entryPointHandle,
 MapHandle);

 // Create returnMapHandle from MapHandle: Fold the pairs in MapHandle by applying
 // [n1->rootConnection.returnMapHandle, n2->rootConnection.returnMapHandle, n3->rootConnection.returnMapHandle]
 // (component-wise) to each triple.
 ReturnMapHandle returnMapHandle;
 TripleProductMapBodyIterator MapIterator(*MapHandle.mapContents);
 MapIterator.Reset();
 while (!MapIterator.AtEnd()) {
 int c1, c2, c3;
 int first, second, third;
 first = MapIterator.Current().First();
 second = MapIterator.Current().Second();
 third = MapIterator.Current().Third();
 c1 = n1->rootConnection.returnMapHandle[0].Lookup(first);
 c2 = n2->rootConnection.returnMapHandle[0].Lookup(second);
 c3 = n3->rootConnection.returnMapHandle[0].Lookup(third);
 returnMapHandle.AddToEnd(op[c1][c2][c3]);
 MapIterator.Next();
 }
 returnMapHandle.Canonicalize();

 // Perform reduction on n, with respect to the common elements that returnMapHandle maps together
 ReductionMapHandle inducedReductionMapHandle;
 ReturnMapHandle inducedReturnMap;
 returnMapHandle.InducedReductionAndReturnMap(inducedReductionMapHandle, inducedReturnMap);
 //     NWAOBDDNodeHandle::InitReduceCache();
 NWAOBDDNodeHandle reduced_n = n.Reduce(inducedReductionMapHandle, inducedReturnMap.Size());
 //     NWAOBDDNodeHandle::DisposeOfReduceCache();

 //ETTODO
 ReturnMapHandle rm[2];
 rm[0] = inducedReturnMap;
 rm[1] = inducedReturnMap;
 // Create and return NWAOBDDBaseNode
 return(new NWAOBDDBaseNode(reduced_n, rm));*/
	NWAOBDDBaseNodeRefPtr  temp;
	return temp;
}
}
