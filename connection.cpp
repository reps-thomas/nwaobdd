#include "nwaobdd_node.h"
#include "connection.h"
// #include "return_map_specializations.h"

namespace NWA_OBDD {


// Default constructor
template <typename Handle>
ConnectionT<Handle>::ConnectionT()
{
}

template <typename Handle>
ConnectionT<Handle>::ConnectionT(const ConnectionT<Handle> &other) 
	: entryPointHandle(other.entryPointHandle), returnMapHandle(other.returnMapHandle)
{

}

// Constructor
template <typename Handle>
ConnectionT<Handle>::ConnectionT(NWAOBDDNodeHandle &entryPointHandle, Handle &returnMapHandle)
	: entryPointHandle(new NWAOBDDNodeHandle(entryPointHandle)), returnMapHandle(returnMapHandle)
{
}

// Constructor
template <typename Handle>
ConnectionT<Handle>::ConnectionT(NWAOBDDNode *entryPoint, Handle &returnMapHandle)
	: entryPointHandle(new NWAOBDDNodeHandle(entryPoint)), returnMapHandle(returnMapHandle)
{
}

template <typename Handle>
ConnectionT<Handle>::~ConnectionT()
{
	delete entryPointHandle;
	entryPointHandle = NULL;
}

// Hash
template <typename Handle>
unsigned int ConnectionT<Handle>::Hash(unsigned int modsize)
{
	unsigned int hvalue = 0;
	hvalue = (997 * returnMapHandle.Hash(modsize) + entryPointHandle->Hash(modsize)) % modsize;
	return hvalue;
}

// Overloaded =
template <typename Handle>
ConnectionT<Handle>& ConnectionT<Handle>::operator= (const ConnectionT<Handle>& C)
{
	if (this != &C)      // don't assign to self!
	{
		entryPointHandle = new NWAOBDDNodeHandle(*(C.entryPointHandle));
		returnMapHandle = C.returnMapHandle;
	}
	return *this;
}

// Overloaded !=
template <typename Handle>
bool ConnectionT<Handle>::operator!= (const ConnectionT<Handle> & C)
{
	return (returnMapHandle != C.returnMapHandle) || ((*entryPointHandle) != (*C.entryPointHandle));
}

// Overloaded ==
template <typename Handle>
bool ConnectionT<Handle>::operator== (const ConnectionT<Handle> & C)
{
	return (returnMapHandle == C.returnMapHandle) && ((*entryPointHandle) == (*C.entryPointHandle));
}

// print
template <typename Handle>
std::ostream& ConnectionT<Handle>::print(std::ostream & out) const
{
	out << (*entryPointHandle);
	out << returnMapHandle;
	return out;
}

template <typename Handle>	
std::ostream& operator<< (std::ostream & out, const ConnectionT<Handle> &c)
{
	c.print(out);
	return(out);
}
} // namespace NWA_OBDD
