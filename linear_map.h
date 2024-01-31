#ifndef LINEAR_MAP_GUARD
#define LINEAR_MAP_GUARD

//
//    Copyright (c) 2017 Thomas W. Reps
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
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include "hashset.h"

class LinearMapHandle;
class LinearMapBody;
namespace mp = boost::multiprecision;
//using namespace mp;

//typedef unsigned long long  ULLONG;
typedef mp::cpp_int ULLONG;
//***************************************************************
// LinearMapHandle
//***************************************************************

class LinearMapHandle {
  // friend LinearMapHandle& operator* (const int, const LinearMapHandle &);
  // friend LinearMapHandle& operator* (const LinearMapHandle &, const int);
 public:
  LinearMapHandle();                               // Default constructor
  ~LinearMapHandle();                              // Destructor
  LinearMapHandle(const LinearMapHandle &r);             // Copy constructor
  LinearMapHandle(const int size);             // constructor with size for vector
  LinearMapHandle& operator= (const LinearMapHandle &r); // Overloaded assignment
  bool operator!= (const LinearMapHandle &r) const;      // Overloaded !=
  bool operator== (const LinearMapHandle &r) const;      // Overloaded ==
  ULLONG& operator[](unsigned int i);                        // Overloaded []
  unsigned int Hash(unsigned int modsize);
  unsigned int Size();
  void AddToEnd(ULLONG y);
  bool Member(ULLONG y);
  ULLONG Lookup(unsigned int x);
  unsigned int LookupInv(ULLONG y);
  void Canonicalize();
  LinearMapBody *mapContents;
  static Hashset<LinearMapBody> *canonicalLinearMapBodySet;
  std::ostream& print(std::ostream & out = std::cout) const;
  LinearMapHandle operator+ (const LinearMapHandle) const; // binary addition
};

std::ostream& operator<< (std::ostream & out, const LinearMapHandle &r);

extern LinearMapHandle operator* (const ULLONG, const LinearMapHandle);
extern LinearMapHandle operator* (const LinearMapHandle, const ULLONG);
extern std::size_t hash_value(const LinearMapHandle& val);

//***************************************************************
// LinearMapBody
//***************************************************************

class LinearMapBody {

  friend void LinearMapHandle::Canonicalize();
  friend unsigned int LinearMapHandle::Hash(unsigned int modsize);

 public:
  LinearMapBody();    // Constructor
  void IncrRef();
  void DecrRef();
  unsigned int Hash(unsigned int modsize);
  void setHashCheck();
  unsigned int refCount;         // reference-count value
  std::vector<ULLONG> mapArray;
  bool operator==(const LinearMapBody &o) const;
  ULLONG& operator[](unsigned int i);                        // Overloaded []
  unsigned int hashCheck;
  bool isCanonical;              // Is this LinearMapBody in *canonicalLinearMapBodySet?

};

#endif
