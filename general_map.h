#ifndef GENERAL_MAP_GUARD
#define GENERAL_MAP_GUARD

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
#include "infrastructure/hashset.h"

class GeneralMapHandle;
class GeneralMapBody;

typedef std::pair<long int, long int> INT_PAIR;

// Note that first element of the pair in the vector is always unique
// Eg: [(0,2),(1,3)] is valid
// Eg: [(0,2),(0,3)] is not valid.

//***************************************************************
// GeneralMapHandle
//***************************************************************

class GeneralMapHandle {
public:
	GeneralMapHandle();                               // Default constructor
	~GeneralMapHandle();                              // Destructor
	GeneralMapHandle(const GeneralMapHandle &r);             // Copy constructor
	GeneralMapHandle(const int size);             // constructor with size for vector
	GeneralMapHandle& operator= (const GeneralMapHandle &r); // Overloaded assignment
	bool operator!= (const GeneralMapHandle &r) const;      // Overloaded !=
	bool operator== (const GeneralMapHandle &r) const;      // Overloaded ==
	INT_PAIR& operator[](unsigned int i);                        // Overloaded []
	unsigned int Hash(unsigned int modsize);
	unsigned int Size();
	void AddToEnd(INT_PAIR y);
	bool Member(INT_PAIR y);
	bool MemberWithFirst(unsigned y);
	INT_PAIR Lookup(unsigned int x);
	unsigned int LookupInv(INT_PAIR y);
	unsigned int LookupInvWithFirst(unsigned y);
	void Canonicalize();
	GeneralMapBody *mapContents;
	static Hashset<GeneralMapBody> *canonicalGeneralMapBodySet;
	std::ostream& print(std::ostream & out = std::cout) const;
	GeneralMapHandle operator+ (const GeneralMapHandle) const; // binary addition
};

std::ostream& operator<< (std::ostream & out, const GeneralMapHandle &r);

extern GeneralMapHandle operator* (const unsigned int, const GeneralMapHandle);
extern GeneralMapHandle operator* (const GeneralMapHandle, const unsigned int);
extern std::size_t hash_value(const GeneralMapHandle& val);

//***************************************************************
// GeneralMapBody
//***************************************************************

class GeneralMapBody {

	friend void GeneralMapHandle::Canonicalize();
	friend unsigned int GeneralMapHandle::Hash(unsigned int modsize);

public:
	GeneralMapBody();    // Constructor
	void IncrRef();
	void DecrRef();
	unsigned int Hash(unsigned int modsize);
	void setHashCheck();
	unsigned int refCount;         // reference-count value
	std::vector<INT_PAIR> mapArray;
	bool operator==(const GeneralMapBody &o) const;
	INT_PAIR& operator[](unsigned int i);                        // Overloaded []
	unsigned int hashCheck;
	bool isCanonical;              // Is this GeneralMapBody in *canonicalGeneralMapBodySet?

};

#endif
