#include"connection.h"
//********************************************************************
// Connection
//********************************************************************

namespace NWA_OBDD { 
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


std::ostream& operator<< (std::ostream & out, const Connection &c)
{
  c.print(out);
  return(out);
}
} // namespace NWAOBDD