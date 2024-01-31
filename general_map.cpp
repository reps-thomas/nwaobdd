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
#include "general_map.h"

//***************************************************************
// GeneralMapBody
//***************************************************************

// Constructor
GeneralMapBody::GeneralMapBody()
	: refCount(0), isCanonical(false)
{
	hashCheck = NULL;
}

void GeneralMapBody::IncrRef()
{
	refCount++;    // Warning: Saturation not checked
}

void GeneralMapBody::DecrRef()
{
	if (--refCount == 0) {    // Warning: Saturation not checked
		if (isCanonical) {
			GeneralMapHandle::canonicalGeneralMapBodySet->DeleteEq(this);
		}
		delete this;
	}
}

unsigned int GeneralMapBody::Hash(unsigned int modsize)
{
	unsigned int hvalue = 0;

	for (unsigned i = 0; i < mapArray.size(); i++)
	{
		hvalue = (997 * hvalue + (int)(mapArray[i].first + 97*mapArray[i].second)) % modsize;
	}
	return hvalue;
}
void GeneralMapBody::setHashCheck()
{
	unsigned int hvalue = 0;

	for (unsigned i = 0; i < mapArray.size(); i++) {
		hvalue = (117 * (hvalue + 1) + (int)(mapArray[i].first + 97*mapArray[i].second));
	}
	hashCheck = hvalue;
}

bool GeneralMapBody::operator==(const GeneralMapBody &o) const
{
	if (hashCheck != o.hashCheck) {
		return false;
	}
	else if (mapArray.size() != o.mapArray.size()) {
		return false;
	}
	else {
		for (unsigned i = 0; i < mapArray.size(); i++) {
			if (mapArray[i].first != o.mapArray[i].first || mapArray[i].second != o.mapArray[i].second) {
				return false;
			}
		}
	}
	return true;
}

// Overloaded []
INT_PAIR& GeneralMapBody::operator[](unsigned int i)
{
	return mapArray[i];
}

std::ostream& operator<< (std::ostream & out, const GeneralMapBody &r)
{
	out << "{GMB: ";
	size_t last = r.mapArray.size() - 1;
	for (size_t i = 0; i <= last; ++i) {
		out << "(" << r.mapArray[i].first << "," << r.mapArray[i].second << ")";
		if (i != last)
			out << ", ";
	}
	out << " GMB}";
	return out;
}

//***************************************************************
// GeneralMapHandle
//***************************************************************

// Initializations of static members ---------------------------------
Hashset<GeneralMapBody> *GeneralMapHandle::canonicalGeneralMapBodySet = new Hashset<GeneralMapBody>;

// Default constructor
GeneralMapHandle::GeneralMapHandle()
	: mapContents(new GeneralMapBody)
{
	mapContents->IncrRef();
}

// Destructor
GeneralMapHandle::~GeneralMapHandle()
{
	mapContents->DecrRef();
}

// Copy constructor
GeneralMapHandle::GeneralMapHandle(const GeneralMapHandle &r)
	: mapContents(r.mapContents)
{
	mapContents->IncrRef();
}

// Overloaded assignment
GeneralMapHandle& GeneralMapHandle::operator= (const GeneralMapHandle &r)
{
	if (this != &r)      // don't assign to self!
	{
		GeneralMapBody *temp = mapContents;
		mapContents = r.mapContents;
		mapContents->IncrRef();
		temp->DecrRef();
	}
	return *this;
}

// Overloaded !=
bool GeneralMapHandle::operator!=(const GeneralMapHandle &r) const
{
	return (mapContents != r.mapContents);
}

// Overloaded ==
bool GeneralMapHandle::operator==(const GeneralMapHandle &r) const
{

	return (mapContents == r.mapContents);
}

// Overloaded []
INT_PAIR& GeneralMapHandle::operator[](unsigned int i)
{
	return (*(this->mapContents))[i];
}

// print
std::ostream& GeneralMapHandle::print(std::ostream & out) const
{
	out << *mapContents;
	return out;
}

std::ostream& operator<< (std::ostream & out, const GeneralMapHandle &r)
{
	r.print(out);
	return(out);
}

unsigned int GeneralMapHandle::Hash(unsigned int modsize)
{
	if (!(mapContents->isCanonical)) {
		std::cout << "Hash of a non-canonical LinearMapHandle occurred" << std::endl;
		abort();
		this->Canonicalize();
	}
	assert(mapContents->isCanonical);
	return ((unsigned int)reinterpret_cast<uintptr_t>(mapContents) >> 2) % modsize;
}

unsigned int GeneralMapHandle::Size()
{
	return (unsigned int)mapContents->mapArray.size();
}

void GeneralMapHandle::AddToEnd(INT_PAIR y)
{
	try{
		assert(mapContents->refCount <= 1);
		mapContents->mapArray.push_back(y);
	}
	catch (std::exception& e){
		std::cout << e.what() << std::endl;
		std::cout << mapContents->refCount << std::endl;
		std::cout << y.first << " " << y.second << std::endl;
		std::cout << Size() << std::endl;
		std::cout << "AddToEnd" << std::endl;
		throw e;
	}
}

bool GeneralMapHandle::Member(INT_PAIR y)
{
	for (unsigned i = 0; i < mapContents->mapArray.size(); i++)
	{
		if (mapContents->mapArray[i].first == y.first && mapContents->mapArray[i].second == y.second) {
			return true;
		}
	}
	return false;
}

bool GeneralMapHandle::MemberWithFirst(unsigned int first){
	
	for (unsigned int i = 0; i < mapContents->mapArray.size(); i++)
	{
		if (mapContents->mapArray[i].first == first)
			return true;
	}
	return false;
}

INT_PAIR GeneralMapHandle::Lookup(unsigned int x)
{
	return mapContents->mapArray[x];
}

unsigned int GeneralMapHandle::LookupInv(INT_PAIR y)
{
	for (unsigned i = 0; i < mapContents->mapArray.size(); i++)
	{
		if (mapContents->mapArray[i].first == y.first && mapContents->mapArray[i].second == y.second)
		{
			return i;
		}
	}
	return -1;
}

unsigned int GeneralMapHandle::LookupInvWithFirst(unsigned int first)
{
	for (unsigned i = 0; i < mapContents->mapArray.size(); i++)
	{
		if (mapContents->mapArray[i].first == first)
			return i;
	}
	return -1;
}

void GeneralMapHandle::Canonicalize()
{
	GeneralMapBody *answerContents;
	mapContents->setHashCheck();
	if (!mapContents->isCanonical) {
		unsigned int hash = canonicalGeneralMapBodySet->GetHash(mapContents);
		answerContents = canonicalGeneralMapBodySet->Lookup(mapContents, hash);
		if (answerContents == NULL) {
			canonicalGeneralMapBodySet->Insert(mapContents, hash);
			mapContents->isCanonical = true;
		}
		else {
			answerContents->IncrRef();
			mapContents->DecrRef();
			mapContents = answerContents;
		}
	}
}

// Linear operators on GeneralMapHandles ------------------------------------------------

// Binary addition
GeneralMapHandle GeneralMapHandle::operator+ (const GeneralMapHandle mapHandle) const
{
	GeneralMapHandle ans;
	for (unsigned int i = 0; i < mapHandle.mapContents->mapArray.size(); i++)
	{
		ans.AddToEnd(mapHandle.mapContents->mapArray[i]);
	}
	for (unsigned i = 0; i < mapContents->mapArray.size(); i++)
	{
		if (ans.MemberWithFirst(mapContents->mapArray[i].first)){
			unsigned int index = ans.LookupInvWithFirst(mapContents->mapArray[i].first);
			assert(index != -1);
			ans.mapContents->mapArray[index].second += mapContents->mapArray[i].second;
		}
		else
		{
			ans.AddToEnd(mapContents->mapArray[i]);
		}
	}
	ans.Canonicalize();
	return ans;
}

// Left scalar-multiplication
GeneralMapHandle operator* (const unsigned factor, const GeneralMapHandle mapHandle)
{
	GeneralMapHandle ans;
	for (unsigned i = 0; i < mapHandle.mapContents->mapArray.size(); i++)
	{
		ans.AddToEnd(std::make_pair(mapHandle.mapContents->mapArray[i].first, factor * mapHandle.mapContents->mapArray[i].second));
	}
	ans.Canonicalize();
	return ans;
}

// Right scalar-multiplication
GeneralMapHandle operator* (const GeneralMapHandle mapHandle, const unsigned factor)
{
	GeneralMapHandle ans;
	for (unsigned i = 0; i < mapHandle.mapContents->mapArray.size(); i++)
	{
		ans.AddToEnd(std::make_pair(mapHandle.mapContents->mapArray[i].first, mapHandle.mapContents->mapArray[i].second * factor));
	}
	ans.Canonicalize();
	return ans;
}

std::size_t hash_value(const GeneralMapHandle& val)
{
	return val.mapContents->hashCheck;
}