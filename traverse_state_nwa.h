#ifndef TRAVERSE_STATE_GUARD_NWA
#define TRAVERSE_STATE_GUARD_NWA

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


#include <iostream>
#include <fstream>

namespace NWA_OBDD {

  class NWAOBDDNode;

class TraverseState;

enum VisitState { FirstVisit, SecondVisit, ThirdVisit, Restart, RestartFirst, RestartSecond, SecondVisitOne, RestartThird };

class TraverseState {
 public:
  TraverseState();                              // Default constructor
  TraverseState(NWAOBDDNode *n, VisitState vs); // Constructor
  TraverseState(NWAOBDDNode *n, VisitState vs, int i); // Constructor
  TraverseState(NWAOBDDNode *n, VisitState vs, int i, int v1);
  TraverseState(NWAOBDDNode *n, VisitState vs, int i, int v1, int v2);
  TraverseState(NWAOBDDNode *n, VisitState vs, int i, int v1, int v2, int eI);
  TraverseState& operator= (const TraverseState& p);  // Overloaded assignment
  bool operator!= (const TraverseState& p);     // Overloaded !=
  bool operator== (const TraverseState& p);     // Overloaded ==
  std::ostream& print(std::ostream & out = std::cout) const;
  NWAOBDDNode *node;
  VisitState visitState;
  int index;
  int val1;
  int val2;
  int exitIndex;
};

std::ostream& operator<< (std::ostream & out, const TraverseState &r);

} // namespace NWA_OBDD

#endif
