#ifndef CROSS_PRODUCT_GUARD_NWA
#define CROSS_PRODUCT_GUARD_NWA

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

#include "infra/intpair.h"
#include "infra/inttriple.h"
#include "nwaobdd_node.h"

// ********************************************************************
// 2-Way Cross Product
// ********************************************************************

// Classes and types declared in this file ---------------------
namespace NWA_OBDD {
class PairProductMapHandle;
class PairProductMapBody;
class PairProductKey;
class PairProductMapKey;
}

typedef ListIterator<intpair> PairProductMapBodyIterator;

//***************************************************************
// PairProductMapHandle
//***************************************************************
namespace NWA_OBDD {
class PairProductMapHandle {
 public:
  PairProductMapHandle();                               // Default constructor
  ~PairProductMapHandle();                              // Destructor
  PairProductMapHandle(const PairProductMapHandle &r);             // Copy constructor
  PairProductMapHandle& operator= (const PairProductMapHandle &r); // Overloaded assignment
  bool operator!= (const PairProductMapHandle &r);      // Overloaded !=
  bool operator== (const PairProductMapHandle &r);      // Overloaded ==
  unsigned int Hash(unsigned int modsize);
  unsigned int Size();
  intpair& operator[](unsigned int i);                       // Overloaded []
  void AddToEnd(const intpair& p);
  bool Member(intpair& p);
  int Lookup(intpair& p);
  void Canonicalize();
  PairProductMapHandle Flip();                          // Create map with reversed entries
  PairProductMapBody *mapContents;
};

//***************************************************************
// PairProductMapBody
//***************************************************************

class PairProductMapBody {//: public List<intpair> {

  friend void PairProductMapHandle::Canonicalize();

 public:
  PairProductMapBody();    // Constructor
  void IncrRef();
  void DecrRef();
  unsigned int Hash(unsigned int modsize);
  unsigned int refCount;         // reference-count value
  void setHashCheck();
  void AddToEnd(const intpair& y);          // Override AddToEnd
  std::vector<intpair> mapArray;
  bool operator==(const PairProductMapBody &p) const;
  intpair& operator[](unsigned int i);                       // Overloaded []
  unsigned int Size();
  unsigned int hashCheck;
 public:
  bool isCanonical;              // Is this PairProductMapBody in *canonicalPairProductMapBodySet?
  static Hashset<PairProductMapBody> *canonicalPairProductMapBodySet;

};

std::ostream& operator<< (std::ostream & out, const PairProductMapBody &r);
}
//***************************************************************
// PairProductMapKey
//***************************************************************

namespace NWA_OBDD{

class PairProductMapKey {

 public:
	 PairProductMapKey(NWAOBDDNodeHandle nodeHandle1, NWAOBDDNodeHandle nodeHandle2, PairProductMapHandle m1, PairProductMapHandle m2, int curANode, int b1, int b2, bool A, PairProductMapHandle PPMap); // Constructor
  unsigned int Hash(unsigned int modsize);
  PairProductMapKey& operator= (const PairProductMapKey& p);  // Overloaded assignment
  bool operator!= (const PairProductMapKey& p);        // Overloaded !=
  bool operator== (const PairProductMapKey& p);        // Overloaded ==
  NWAOBDDNodeHandle NodeHandle1() const { return nodeHandle1; }      // Access function
  NWAOBDDNodeHandle NodeHandle2() const { return nodeHandle2; }      // Access function
  std::ostream& print(std::ostream & out) const;

 private:
  NWAOBDDNodeHandle nodeHandle1;
  NWAOBDDNodeHandle nodeHandle2;
  PairProductMapHandle m1;
  PairProductMapHandle m2;
  PairProductMapHandle PPMap;
  int curANode;
  int b1;
  int b2;
  bool A;
  PairProductMapKey();                                 // Default constructor (hidden)
};

//*********************************************************************
// PairProductKey
//*********************************************************************
	class PairProductKey {
	public:
		PairProductKey(NWAOBDDNodeHandle nodeHandle1, NWAOBDDNodeHandle nodeHandle2); // Constructor
		unsigned int Hash(unsigned int modsize);
		PairProductKey& operator= (const PairProductKey& p);  // Overloaded assignment
		bool operator!= (const PairProductKey& p) const;        // Overloaded !=
		bool operator== (const PairProductKey& p) const;        // Overloaded ==
		NWAOBDDNodeHandle NodeHandle1() const { return nodeHandle1; }      // Access function
		NWAOBDDNodeHandle NodeHandle2() const { return nodeHandle2; }      // Access function
		std::ostream& print(std::ostream & out) const;

	private:
		NWAOBDDNodeHandle nodeHandle1;
		NWAOBDDNodeHandle nodeHandle2;
		PairProductKey();                                 // Default constructor (hidden)

	};
std::ostream& operator<< (std::ostream & out, const PairProductMapKey &p);
std::ostream& operator<< (std::ostream & out, const PairProductKey &p);

//***************************************************************
// PairProductMemo
//***************************************************************

class PairProductMemo {

 public:
  PairProductMemo();                                 // Default constructor
  PairProductMemo(NWAOBDDNodeHandle nodeHandle, PairProductMapHandle pairProductMapHandle); // Constructor
  PairProductMemo& operator= (const PairProductMemo& p);  // Overloaded assignment
  bool operator!= (const PairProductMemo& p);        // Overloaded !=
  bool operator== (const PairProductMemo& p);        // Overloaded ==

  NWAOBDDNodeHandle nodeHandle;
  PairProductMapHandle pairProductMapHandle;
};

// Auxiliary functions -----------------------------------------------
NWAOBDDNodeHandle PairProduct(NWAOBDDNodeHandle n1,
                              NWAOBDDNodeHandle n2,
                              PairProductMapHandle &pairProductMap
                             );
NWAOBDDNodeHandle PairProduct(NWAOBDDInternalNode *n1,
                              NWAOBDDInternalNode *n2,
                              PairProductMapHandle &pairProductMap
                             );


void InitPairProductCache();
void DisposeOfPairProductCache();
}

#endif