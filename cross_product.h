#ifndef CROSS_PRODUCT_CFL_GUARD
#define CROSS_PRODUCT_CFL_GUARD

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

#include "intpair.h"
#include "inttriple.h"

// ********************************************************************
// 2-Way Cross Product
// ********************************************************************

// Classes and types declared in this file ---------------------
namespace NWA_OBDD {
class PairProductMapHandle;
class PairProductMapBody;
class PairProductKey;
}

//***************************************************************
// PairProductMapBodyIterator
//***************************************************************

typedef ListIterator<intpair> PairProductMapBodyIterator;

//***************************************************************
// PairProductMapHandle
//***************************************************************
namespace NWA_OBDD {
class PairProductMapHandle;

//***************************************************************
// PairProductMapBody
//***************************************************************

class PairProductMapBody;

std::ostream& operator<< (std::ostream & out, const PairProductMapBody &r);
}
//***************************************************************
// PairProductKey
//***************************************************************

namespace NWA_OBDD{

class PairProductKey;


std::ostream& operator<< (std::ostream & out, const PairProductKey &p);

//***************************************************************
// PairProductMemo
//***************************************************************

class PairProductMemo;

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
// ********************************************************************
// 3-Way Cross Product
// ********************************************************************

// Classes and types declared in this file ---------------------
namespace NWA_OBDD {
class TripleProductMapHandle;
class TripleProductMapBody;
class TripleProductKey;
}
//***************************************************************
// TripleProductMapBodyIterator
//***************************************************************

typedef ListIterator<inttriple> TripleProductMapBodyIterator;

//***************************************************************
// TripleProductMapHandle
//***************************************************************
namespace NWA_OBDD {
class TripleProductMapHandle;

//***************************************************************
// TripleProductMapBody
//***************************************************************

class TripleProductMapBody;
}
std::ostream& operator<< (std::ostream & out, const NWA_OBDD::TripleProductMapBody &r);



//***************************************************************
// TripleProductKey
//***************************************************************
namespace NWA_OBDD {
class TripleProductKey;

std::ostream& operator<< (std::ostream & out, const TripleProductKey &p);

//***************************************************************
// TripleProductMemo
//***************************************************************

class TripleProductMemo;


// Auxiliary functions -----------------------------------------------
NWAOBDDNodeHandle TripleProduct(NWAOBDDNodeHandle n1,
                                NWAOBDDNodeHandle n2,
                                NWAOBDDNodeHandle n3,
                                TripleProductMapHandle &tripleProductMap
                               );
NWAOBDDNodeHandle TripleProduct(NWAOBDDInternalNode *n1,
                                NWAOBDDInternalNode *n2,
                                NWAOBDDInternalNode *n3,
                                TripleProductMapHandle &tripleProductMap
                               );


void InitTripleProductCache();
void DisposeOfTripleProductCache();

} // namespace NWA_OBDD

#endif //CROSS_PRODUCT_CFL_GUARD
