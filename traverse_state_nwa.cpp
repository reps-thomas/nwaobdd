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
#include "traverse_state_nwa.h"

using namespace NWA_OBDD;

//**********************************************************************
// TraverseState
//**********************************************************************

TraverseState::TraverseState()
  :  node(NULL), visitState(FirstVisit)
{
}

TraverseState::TraverseState(NWAOBDDNode *n, VisitState vs)
  :  node(n), visitState(vs)
{
}

TraverseState::TraverseState(NWAOBDDNode *n, VisitState vs, int i)
  :  node(n), visitState(vs), index(i)
{
}

TraverseState::TraverseState(NWAOBDDNode *n, VisitState vs, int i, int v1)
  :  node(n), visitState(vs), index(i), val1(v1)
{
}

TraverseState::TraverseState(NWAOBDDNode *n, VisitState vs, int i, int v1, int v2)
  :  node(n), visitState(vs), index(i), val1(v1), val2(v2)
{
}

TraverseState::TraverseState(NWAOBDDNode *n, VisitState vs, int i, int v1, int v2, int eI)
  :  node(n), visitState(vs), index(i), val1(v1), val2(v2), exitIndex(eI)
{
}

std::ostream& operator<< (std::ostream & out, const TraverseState &p)
{
  p.print(out);
  return(out);
}

TraverseState& TraverseState::operator= (const TraverseState& i)
{
  if (this != &i)      // don't assign to self!
  {
    node = i.node;
    visitState = i.visitState;
	if (i.visitState == ThirdVisit || i.visitState == RestartThird || i.visitState == RestartSecond)
      index = i.index;
	if (i.visitState == SecondVisit || i.visitState == SecondVisitOne || i.visitState == RestartSecond)
		val1 = i.val1;
	if (i.visitState == ThirdVisit || i.visitState == RestartThird)
		val2 = i.val2;
	if (i.visitState == SecondVisitOne || i.visitState == RestartThird)
		exitIndex = i.exitIndex;
  }
  return *this;        
}

// Overloaded !=
bool TraverseState::operator!=(const TraverseState& p)
{
  return !(*this == p);
}

// Overloaded ==
bool TraverseState::operator==(const TraverseState& p)
{
  if (visitState == ThirdVisit) {
    return (node == p.node) && (visitState == p.visitState) && (index == p.index);
  }
  return (node == p.node) && (visitState == p.visitState);
}

// print
std::ostream& TraverseState::print(std::ostream & out) const
{
  if (visitState == ThirdVisit) {
    out << "(" << node << ", " << visitState << ", " << index << ")";
  }
  else {
    out << "(" << node << ", " << visitState << ")";
  } 
  return out;
}
