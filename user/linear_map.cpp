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
#include <cassert>
#include "linear_map.h"

//***************************************************************
// LinearMapBody
//***************************************************************

// Constructor
LinearMapBody::LinearMapBody()
  : refCount(0), isCanonical(false)
{
	  hashCheck = NULL;
}

void LinearMapBody::IncrRef()
{
  refCount++;    // Warning: Saturation not checked
}

void LinearMapBody::DecrRef()
{
  if (--refCount == 0) {    // Warning: Saturation not checked
    if (isCanonical) {
      LinearMapHandle::canonicalLinearMapBodySet->DeleteEq(this);
    }
    delete this;
  }
}

unsigned int LinearMapBody::Hash(unsigned int modsize)
{
  unsigned int hvalue = 0;

  for (unsigned i = 0; i < mapArray.size(); i++)
  {
	  hvalue = (997* hvalue + (int)mapArray[i]) % modsize;
  }
  return hvalue;
}
void LinearMapBody::setHashCheck()
{
  unsigned int hvalue = 0;

  for (unsigned i = 0; i < mapArray.size(); i++) {
	  hvalue = (117*(hvalue+1) + (int)(mapArray[i]));
  }
  hashCheck = hvalue;
}

bool LinearMapBody::operator==(const LinearMapBody &o) const
{
	if (hashCheck != o.hashCheck) {
		return false;
	}
	else if (mapArray.size() != o.mapArray.size()) {
		return false;
	}
	else {
	    for (unsigned i = 0; i < mapArray.size(); i++) {
	  	    if (mapArray[i] != o.mapArray[i]) {
			    return false;
		    }
	    }
	}
	return true;
}

// Overloaded []
ULLONG& LinearMapBody::operator[](unsigned int i)
{
	return mapArray[i];
}

std::ostream& operator<< (std::ostream & out, const LinearMapBody &r)
{
	out << "{LMB: ";
	size_t last = r.mapArray.size() - 1;
	for(size_t i = 0; i <= last; ++i) {
		out << r.mapArray[i];
        if (i != last) 
            out << ", ";
    }
	out << " LMB}";
    return out;
}

//***************************************************************
// LinearMapHandle
//***************************************************************

// Initializations of static members ---------------------------------
Hashset<LinearMapBody> *LinearMapHandle::canonicalLinearMapBodySet = new Hashset<LinearMapBody>;

// Default constructor
LinearMapHandle::LinearMapHandle()
  :  mapContents(new LinearMapBody)
{
  mapContents->IncrRef();
}

// Destructor
LinearMapHandle::~LinearMapHandle()
{
  mapContents->DecrRef();
}

// Copy constructor
LinearMapHandle::LinearMapHandle(const LinearMapHandle &r)
  :  mapContents(r.mapContents)
{
  mapContents->IncrRef();
}

// Overloaded assignment
LinearMapHandle& LinearMapHandle::operator= (const LinearMapHandle &r)
{
  if (this != &r)      // don't assign to self!
  {
    LinearMapBody *temp = mapContents;
    mapContents = r.mapContents;
    mapContents->IncrRef();
    temp->DecrRef();
  }
  return *this;        
}

// Overloaded !=
bool LinearMapHandle::operator!=(const LinearMapHandle &r) const
{
  return (mapContents != r.mapContents);
}

// Overloaded ==
bool LinearMapHandle::operator==(const LinearMapHandle &r) const
{

  return (mapContents == r.mapContents);
}

// Overloaded []
ULLONG& LinearMapHandle::operator[](unsigned int i)
{
	return (*(this->mapContents))[i];
}

// print
std::ostream& LinearMapHandle::print(std::ostream & out) const
{
  out << *mapContents;
  return out;
}

std::ostream& operator<< (std::ostream & out, const LinearMapHandle &r)
{
  r.print(out);
  return(out);
}

unsigned int LinearMapHandle::Hash(unsigned int modsize)
{
	if (!(mapContents->isCanonical)) {
		std::cout << "Hash of a non-canonical LinearMapHandle occurred" << std::endl;
		abort();
		this->Canonicalize();
	}
	assert(mapContents->isCanonical);
	return ((unsigned int)reinterpret_cast<uintptr_t>(mapContents) >> 2) % modsize;
}

unsigned int LinearMapHandle::Size()
{
  return (unsigned int)mapContents->mapArray.size();
}

void LinearMapHandle::AddToEnd(ULLONG y)
{
	try{
		assert(mapContents->refCount <= 1);
		mapContents->mapArray.push_back(y);
	}
	catch (std::exception& e){
		std::cout << e.what() << std::endl;
		std::cout << mapContents->refCount << std::endl;
		std::cout << y << std::endl;
		std::cout << Size() << std::endl;
		std::cout << "AddToEnd" << std::endl;
		throw e;
	}
}

bool LinearMapHandle::Member(ULLONG y)
{
	for (unsigned i = 0; i < mapContents->mapArray.size(); i++)
	{
		if (mapContents->mapArray[i] == y) {
			return true;
		}
	}
	return false;
}

ULLONG LinearMapHandle::Lookup(unsigned int x)
{
	return mapContents->mapArray[x];
}

unsigned int LinearMapHandle::LookupInv(ULLONG y)
{
	for (unsigned i = 0; i < mapContents->mapArray.size(); i++)
	{
		if (mapContents->mapArray[i] == y)
		{
			return i;
		}
	}
	return -1;
}

void LinearMapHandle::Canonicalize()
{ 
  LinearMapBody *answerContents;
  mapContents->setHashCheck();
  if (!mapContents->isCanonical) {
	  unsigned int hash = canonicalLinearMapBodySet->GetHash(mapContents);
	answerContents = canonicalLinearMapBodySet->Lookup(mapContents, hash);
    if (answerContents == NULL) {
      canonicalLinearMapBodySet->Insert(mapContents, hash);
      mapContents->isCanonical = true;
    }
    else {
      answerContents->IncrRef();
      mapContents->DecrRef();
      mapContents = answerContents;
    }
  }
}

// Linear operators on LinearMapHandles ------------------------------------------------

// Binary addition
LinearMapHandle LinearMapHandle::operator+ (const LinearMapHandle mapHandle) const
{
	assert(mapContents->mapArray.size() == mapHandle.mapContents->mapArray.size());
	LinearMapHandle ans;
	try{
		for (unsigned i = 0; i < mapContents->mapArray.size(); i++)
		{
			ans.AddToEnd(mapContents->mapArray[i] + mapHandle.mapContents->mapArray[i]);
		}
		ans.Canonicalize();
		return ans;
	}
	catch (std::exception& e){
		std::cout << e.what() << std::endl;
		std::cout << "operator+" << std::endl;
		std::cout << mapContents->mapArray.size() << std::endl;
		std::cout << ans.Size() << std::endl;
		throw e;
	}
}

// Left scalar-multiplication
LinearMapHandle operator* (const ULLONG factor, const LinearMapHandle mapHandle)
{
	LinearMapHandle ans;
	for (unsigned i = 0; i < mapHandle.mapContents->mapArray.size(); i++)
	{
		ans.AddToEnd(factor * mapHandle.mapContents->mapArray[i]);
	}
	ans.Canonicalize();
	return ans;
}

// Right scalar-multiplication
LinearMapHandle operator* (const LinearMapHandle mapHandle, const ULLONG factor)
{
	LinearMapHandle ans;
	for (unsigned i = 0; i < mapHandle.mapContents->mapArray.size(); i++)
	{
		ans.AddToEnd(mapHandle.mapContents->mapArray[i] * factor);
	}
	ans.Canonicalize();
	return ans;
}

std::size_t hash_value(const LinearMapHandle& val)
{
	return val.mapContents->hashCheck;
}

