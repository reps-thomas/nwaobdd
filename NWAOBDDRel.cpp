#include "NWAOBDDRel.hpp"
#include <unordered_map>

using namespace wali::domains::nwaobddrel;
using std::endl;
using wali::waliErr;
using std::cout;


int NWAOBDDContext::numNWAOBDDContexts = 0;
// ////////////////////////////


namespace wali
{
  namespace domains
  {
    namespace nwaobddrel
    {
      static NWAOBDDRel* convert(wali::SemElem* se) 
      {
        NWAOBDDRel* br = dynamic_cast<NWAOBDDRel*>(se);
        if (br == NULL) {
          *waliErr << "[ERROR] Cannot cast to class NWA_OBDD::NWAOBDD::NWAOBDDRel.\n";
          se->print( *waliErr << "    " ) << endl;
          assert(false);
        }
        // When done with developement
        // NWAOBDDRel* br = static_cast<NWAOBDDRel*>(se)
        return br;
      }

	        typedef std::pair< NWA_OBDD::NWAOBDD, bool> StarCacheKey;
      struct StarCacheHash
      {
        size_t operator() (StarCacheKey k) const
        {
          return k.first.Hash(256) << 1 & (int) k.second; //ETTODO - fix hash
        }
      };
      std::unordered_map< StarCacheKey, sem_elem_t, StarCacheHash> star_cache;
	
	}
  }
}

/*
*  NWAOBDDContext constructors
*
*
*/
  NWAOBDDContext::NWAOBDDContext(ordering o) :
  count(0),
  order(o)
{
  numNWAOBDDContexts++;
  //release mutex

  cachedBaseOne = NULL;
  cachedBaseZero = NULL;
  cachedBaseTop = NULL;
  cachedTensorOne = NULL;
  cachedTensorZero = NULL;
  cachedTensorTop = NULL;
  setUpLocations();
}

//Don't copy count
NWAOBDDContext::NWAOBDDContext(const NWAOBDDContext& other) :
  count(0),
  cachedBaseOne(other.cachedBaseOne),
  cachedBaseZero(other.cachedBaseZero),
  cachedTensorOne(other.cachedTensorOne),
  cachedTensorZero(other.cachedTensorZero),
  order(other.order)
{
  numNWAOBDDContexts++;
  populateCache(other.order);
}

/*
*  Add the strings for the local variables for the caller/callee pair represented by loc to mergeVars
*/
void NWAOBDDContext::addVarList(std::pair<int,int> loc, std::vector<std::string> lVars1, std::vector<std::string> lVars2)
{
	mergeVars[loc] = std::pair<std::vector<std::string>,std::vector<std::string>>(lVars1,lVars2);
}

/*
*  Get the strings for the local variables for the caller/callee pair represented by loc
*/
std::pair<std::vector<std::string>, std::vector<std::string>> NWAOBDDContext::getLocalVars(std::pair<int, int> loc) const
{
	return mergeVars.at(loc);
}

/*
*  For a given variable ordering "order" set up the locations of the various vocabularies in
*  the tier.  There are 12 possible locations.
*/
void NWAOBDDContext::setUpLocations()
{
	switch (order)
	{
	case BASE_1ST_TENSOR_ROOT:
		basePreLoc = 1;
		basePostLoc = 2;
		baseExtraLoc = 3;
		tensorPreLoc = 0;
		tensorPostLoc = 5;
		tensorExtraLoc = 11;
		regAInfo = 6;
		regBInfo = 7;
		regCInfo = 8;
		/* This ordering is:
		TensorPre
		BasePre
		Base Post
		Base Extra
		*
		TensorPost
		*
		*
		*
		*
		*
		TensorExtra
		*/
		break;
	case SEPERATE_LEVELS:
		basePreLoc = 1;
		basePostLoc = 2;
		baseExtraLoc = 3;
		tensorPreLoc = 7;
		tensorPostLoc = 8;
		tensorExtraLoc = 9;
		regAInfo = 5;
		regBInfo = 0;
		regCInfo = 6;
		/*  This ordering is:
		*
		  BasePre
		  BasePost
		  BaseExtra
		  *
		*
		*
		  TensorPre
		  TensorPost
		  TensorExtra
		  *
		*
		*/
		break;
	case AFFINITY:
		basePreLoc = 5;
		basePostLoc = 6;
		baseExtraLoc = 11;
		tensorPreLoc = 2;
		tensorPostLoc = 0;
		tensorExtraLoc = 1;
		regAInfo = 8;
		regBInfo = 7;
		regCInfo = 9;
		/*  This ordering is:
		tensorPost
			tensorExtra
			tensorPre
			*
			*
		basePre
		basePost
			*
			*
			*
			*
		base Extra
		*/
		break;
	}
}

/*
*  Copy constructor for NWAOBDDContexts
*/
NWAOBDDContext& NWAOBDDContext::operator = (const NWAOBDDContext& other)
{
  if(this!=&other){
    count=0;
    cachedBaseOne=other.cachedBaseOne;
    cachedBaseZero=other.cachedBaseZero;
    cachedTensorOne=other.cachedTensorOne;
    cachedTensorZero=other.cachedTensorZero;
	order=other.order;
    populateCache(order);
  }
  return *this;
}


NWAOBDDContext::~NWAOBDDContext()
{
  // shared_ptr::reset sets it to NULL

  //Delete cached NWAOBDDRel objects.
  cachedBaseOne = NULL;
  cachedBaseZero = NULL;
  cachedBaseTop = NULL;
  cachedTensorOne = NULL;
  cachedTensorZero = NULL;
  cachedTensorTop = NULL;


  //lock mutex
  numNWAOBDDContexts--;
  if(numNWAOBDDContexts == 0){
    //All NWAOBDDContexts are now dead. So we must shutdown buddy.
    star_cache.clear();
    //Also clean up the NWAOBDDRel class
    NWAOBDDRel::reset();
  }
  //release mutex
}

/*
*  Havoc the variables in the BASE_EXTRA vocabulary (do this after extends)
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::HavocExtra(NWA_OBDD::NWAOBDD current) const
{
	for (int i = 0; i < this->varMap.size(); i++)
	{
		int varOff = getVLoc(BASE_EXTRA);
		int vLoc = findVarLoc(i, 0, this->maxLevel, varOff);
		current = NWA_OBDD::MkExists(current, vLoc);
	}
	return current;
}

/*
*  Havoc the specific variable in the NWAOBDD current represented by var
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::HavocVar(std::string var, NWA_OBDD::NWAOBDD current)
{
	int bi;
	if (this->varMap.find(var) == this->varMap.end()){
		//LOG(WARNING) << "[NWAOBDDContext::Havoc] Unknown Variable: " << var;
		return NWA_OBDD::MkTrue();
	}
	else {
		bi = this->varMap.find(var)->second;
	}
	int varOff = getVLoc(BASE_POST);
	int vLoc = findVarLoc(bi, 0, this->maxLevel, varOff);
	NWA_OBDD::NWAOBDD h = NWA_OBDD::MkExists(current, vLoc); //BDDExits
	return h;
}

/*
*  Perform an assignment.  Do this by moving the value in REG_A (the result of the right side of the expression as
*  an NWAOBDD) to the value being assigned and perform a series of Ands and Ors.  Quantifieing out the redundant info once
*  finished
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::AssignGen(std::string var, NWA_OBDD::NWAOBDD expr, NWA_OBDD::NWAOBDD aType) const
{
	int bi;
	if (this->varMap.find(var) == this->varMap.end()){
		//LOG(WARNING) << "[NWAOBDDContext::AssignGen] Unknown Variable: " << var;
		return NWA_OBDD::MkTrue();
	}
	else {
		bi = this->varMap.find(var)->second;
	}
	int varPostOff = getVLoc(BASE_POST);
	int vPostLoc = findVarLoc(bi, 0, this->maxLevel, varPostOff);

	int varPreOff = getVLoc(BASE_PRE);
	int vPreLoc = findVarLoc(bi, 0, this->maxLevel, varPreOff);

	int a = getVLoc(REG_A_INFO);
	int regALoc = findVarLoc(0, 0, this->maxLevel, a);
	NWA_OBDD::NWAOBDD A = NWA_OBDD::MkDistinction(regALoc);

	NWA_OBDD::NWAOBDD VPost = NWA_OBDD::MkDistinction(vPostLoc);

	//regA2Var -> <A:0, biRhs:0> <A:1, biRhs:1>
	NWA_OBDD::NWAOBDD regA2Var = NWA_OBDD::MkOr(NWA_OBDD::MkAnd(A, VPost), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), NWA_OBDD::MkNot(VPost)));

	//nExpr = relprod(expr, regA2var, A)
	NWA_OBDD::NWAOBDD nExpr = NWA_OBDD::MkExists(NWA_OBDD::MkAnd(expr, regA2Var), regALoc);

	NWA_OBDD::NWAOBDD c = NWA_OBDD::MkExists(NWA_OBDD::MkExists(aType,vPostLoc),vPreLoc);

	NWA_OBDD::NWAOBDD ret = NWA_OBDD::MkAnd(nExpr, c);
	return ret;
}


/*
*  Return the NWAOBDD representing the "True" assignment to var
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::True(std::string var)
{
	int v = this->varMap[var];
	int temp = getVLoc(BASE_POST);
	int t = this->findVarLoc(v, 0, this->maxLevel,temp);
	return NWA_OBDD::MkDistinction(t);
}

/*
*  Return the NWAOBDD representing the "False" assignment to var
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::False(std::string var) 
{
	int v = this->varMap[var];
	int offset = getVLoc(BASE_POST);
	int t = this->findVarLoc(v, 0, this->maxLevel, offset);
	NWA_OBDD::NWAOBDD temp = NWA_OBDD::MkNot(NWA_OBDD::MkDistinction(t));
	return temp;
}

/*
*  Return the NWAOBDD representing the "True" assignment to Reg_A (a place holder
*  location for assignements)
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::True() const
{
	int t = getVLoc(REG_A_INFO);
	int regALoc = this->findVarLoc(0, 0, this->maxLevel, t);
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::NWAOBDD(NWA_OBDD::MkDistinction(regALoc));
	return ret;
}

/*
*  Return the NWAOBDD representing the "False" assignment to Reg_A (a place holder
*  location for assignements)
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::False() const
{
	int t = getVLoc(REG_A_INFO);
	int regALoc = this->findVarLoc(0, 0, this->maxLevel, t);
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::NWAOBDD(NWA_OBDD::MkNot(NWA_OBDD::MkDistinction(regALoc)));
	return ret;
}

/*
*  Implement the Assume expression.  Create an NWAOBDD that only accepts assignments where the result of rexpr and
*  the result of lexpr are the same.
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::Assume(NWA_OBDD::NWAOBDD lexpr, NWA_OBDD::NWAOBDD rexpr) const
{
	int a = getVLoc(REG_A_INFO);
	int b = getVLoc(REG_B_INFO);
	
	int regALoc = this->findVarLoc(0, 0, this->maxLevel, a);
	int regBLoc = this->findVarLoc(0, 0, this->maxLevel, b);

	NWA_OBDD::NWAOBDD A = NWA_OBDD::MkDistinction(regALoc);
	NWA_OBDD::NWAOBDD B = NWA_OBDD::MkDistinction(regBLoc);

	NWA_OBDD::NWAOBDD eqAB = NWA_OBDD::MkOr(NWA_OBDD::MkAnd(A, B), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), NWA_OBDD::MkNot(B)));
	NWA_OBDD::NWAOBDD equate = NWA_OBDD::MkAnd(this->BaseID(), eqAB);
	NWA_OBDD::NWAOBDD shift = NWA_OBDD::MkAnd(rexpr, eqAB);
	NWA_OBDD::NWAOBDD nRexpr = NWA_OBDD::MkExists(shift, regALoc);
	NWA_OBDD::NWAOBDD expr = NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(lexpr, nRexpr), equate);
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::MkExists(NWA_OBDD::MkExists(expr, regALoc), regBLoc);
	return ret;
}

/*
*  Implement the AND opperation on the result of the lexpr and rexpr
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::And(NWA_OBDD::NWAOBDD lexpr, NWA_OBDD::NWAOBDD rexpr) const
{
	return applyBinOp(lexpr, rexpr, NWAAnd());
}

/*
*  Implement the OR opperation on the result of the lexpr and rexpr
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::Or(NWA_OBDD::NWAOBDD lexpr, NWA_OBDD::NWAOBDD rexpr) const
{
	return applyBinOp(lexpr, rexpr, NWAOr());
}

/*
*  Implement the NOT opperation on the result of the expre
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::Not(NWA_OBDD::NWAOBDD expr) const
{
	return applyUnOp(expr, NWANot()); 
}

/*
*  Apply a unary op on expr.  The result of the unary op applied to REG_A is stored in REG_C of op
*  Perform an and on the two NWAOBDDs and move the result back into REG_A 
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::applyUnOp(NWA_OBDD::NWAOBDD expr, NWA_OBDD::NWAOBDD op) const
{
	int a = getVLoc(REG_A_INFO);
	int c = getVLoc(REG_C_INFO);

	int regALoc = this->findVarLoc(0, 0, this->maxLevel, a);
	int regCLoc = this->findVarLoc(0, 0, this->maxLevel, c);

	NWA_OBDD::NWAOBDD A = NWA_OBDD::MkDistinction(regALoc);
	NWA_OBDD::NWAOBDD C = NWA_OBDD::MkDistinction(regCLoc);

	NWA_OBDD::NWAOBDD shiftBack = NWA_OBDD::MkOr(NWA_OBDD::MkAnd(A, C), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), NWA_OBDD::MkNot(C)));

	NWA_OBDD::NWAOBDD s1 = NWA_OBDD::MkAnd(expr, op);
	NWA_OBDD::NWAOBDD s2 = NWA_OBDD::MkExists(s1, regALoc);
	NWA_OBDD::NWAOBDD s3 = NWA_OBDD::MkAnd(s2, shiftBack);
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::MkExists(s3, regCLoc);
	return ret;
}

/*
*  Apply a unary op on expr.  The result of the binary op applied to REG_A and REG_C is stored in REG_B of op
*  Perform an and on the three NWAOBDDs and move the result back into REG_A
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::applyBinOp(NWA_OBDD::NWAOBDD lexpr, NWA_OBDD::NWAOBDD rexpr, NWA_OBDD::NWAOBDD op) const
{
	int a = getVLoc(REG_A_INFO);
	int c = getVLoc(REG_C_INFO);
	int b = getVLoc(REG_B_INFO);

	int regALoc = this->findVarLoc(0, 0, this->maxLevel, a); 
	int regCLoc = this->findVarLoc(0, 0, this->maxLevel, c);
	int regBLoc = this->findVarLoc(0, 0, this->maxLevel, b);

	NWA_OBDD::NWAOBDD A = NWA_OBDD::MkDistinction(regALoc);
	NWA_OBDD::NWAOBDD C = NWA_OBDD::MkDistinction(regCLoc);
	NWA_OBDD::NWAOBDD B = NWA_OBDD::MkDistinction(regBLoc);

	NWA_OBDD::NWAOBDD shiftAtoC = NWA_OBDD::MkOr(NWA_OBDD::MkAnd(A, C), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), NWA_OBDD::MkNot(C)));
	NWA_OBDD::NWAOBDD rShifted = NWA_OBDD::MkExists(NWA_OBDD::MkAnd(rexpr, shiftAtoC),regALoc);
	NWA_OBDD::NWAOBDD applyOp = NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(lexpr, rShifted), op);

	NWA_OBDD::NWAOBDD shiftBack = NWA_OBDD::MkOr(NWA_OBDD::MkAnd(A, B), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), NWA_OBDD::MkNot(B)));
	NWA_OBDD::NWAOBDD removeExtra = NWA_OBDD::MkExists(NWA_OBDD::MkExists(applyOp, regALoc), regCLoc);
	NWA_OBDD::NWAOBDD sb = NWA_OBDD::MkAnd(removeExtra, shiftBack);
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::MkExists(sb, regBLoc);
	/*nwaobdd_t tmp = new NWAOBDDRel(this, ret, false);
	tmp->print(std::cout);*/
	return ret;
}

/*
*  Create an NWAOBDD repressenting the not operation on REG_A stored into REG_C 
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::NWANot() const
{
	int x = getVLoc(REG_A_INFO);
	int y = getVLoc(REG_C_INFO);
	
	int regALoc = this->findVarLoc(0, 0, this->maxLevel, x);
	int regCLoc = this->findVarLoc(0, 0, this->maxLevel, y);
	NWA_OBDD::NWAOBDD A = NWA_OBDD::MkDistinction(regALoc);
	NWA_OBDD::NWAOBDD C = NWA_OBDD::MkDistinction(regCLoc);
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::MkOr(NWA_OBDD::MkAnd(A, NWA_OBDD::MkNot(C)), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), C));
	return ret;
}


/*
*  Create an NWAOBDD representing the or operation on REG_A and REG_C stored into REG_B
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::NWAOr() const
{
	int x = getVLoc(REG_A_INFO);
	int y = getVLoc(REG_C_INFO);
	int z = getVLoc(REG_B_INFO);

	int regALoc = this->findVarLoc(0, 0, this->maxLevel, x);
	int regCLoc = this->findVarLoc(0, 0, this->maxLevel, y);
	int regBLoc = this->findVarLoc(0, 0, this->maxLevel, z);
	NWA_OBDD::NWAOBDD A = NWA_OBDD::MkDistinction(regALoc);
	NWA_OBDD::NWAOBDD C = NWA_OBDD::MkDistinction(regCLoc);
	NWA_OBDD::NWAOBDD B = NWA_OBDD::MkDistinction(regBLoc);
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::MkOr(NWA_OBDD::MkOr(
		NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(A, C), B), 
		NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), C), B)),
		NWA_OBDD::MkOr(NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(A, NWA_OBDD::MkNot(C)), B), 
		NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), NWA_OBDD::MkNot(C)), NWA_OBDD::MkNot(B))));
	return ret;
}

/*
*  Create an NWAOBDD representing the and operation on REG_A and REG_C stored into REG_B
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::NWAAnd() const
{
	int x = getVLoc(REG_A_INFO);
	int y = getVLoc(REG_C_INFO);
	int z = getVLoc(REG_B_INFO);

	int regALoc = this->findVarLoc(0, 0, this->maxLevel, x);
	int regCLoc = this->findVarLoc(0, 0, this->maxLevel, y);
	int regBLoc = this->findVarLoc(0, 0, this->maxLevel, z);
	NWA_OBDD::NWAOBDD A = NWA_OBDD::MkDistinction(regALoc);
	NWA_OBDD::NWAOBDD C = NWA_OBDD::MkDistinction(regCLoc);
	NWA_OBDD::NWAOBDD B = NWA_OBDD::MkDistinction(regBLoc);
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::MkOr(NWA_OBDD::MkOr(
												NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(A, C), B), //1 & 1 : 1
												NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), C), NWA_OBDD::MkNot(B))),
										   NWA_OBDD::MkOr(
												NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(A, NWA_OBDD::MkNot(C)), NWA_OBDD::MkNot(B)),
												NWA_OBDD::MkAnd(NWA_OBDD::MkAnd(NWA_OBDD::MkNot(A), NWA_OBDD::MkNot(C)), NWA_OBDD::MkNot(B))));
	return ret;
}

/*
*  Return an NWAOBDD representing the ID relation between the base pre and post vocabularies
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::BaseID() const
{
	return cachedBaseOne->getNWAOBDD();
}

/*
*  Create an NWAOBDD that represents var being assigned the value of true
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::NWAOBDDTCreate(int var, vocabulary_t vocab)
{
	int t = getVLoc(vocab);
	int vLoc = this->findVarLoc(var,0, this->maxLevel,t);
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::NWAOBDD(NWA_OBDD::MkDistinction(vLoc));
	return ret;
}

/*
*  Find the location of var in the NWAOBDD represented by the context
*  Offset is the specific vocabulary of that var, level is the current level, vocabOff is
*  the offset generated by the recursive calls.
*/
int NWAOBDDContext::findVarLoc(int var, int offset, int level, int vocabOff) const
{
	int numTiers = level / 2;
	unsigned int size = ((unsigned int)((((unsigned int)1) << (level + 2)) - (unsigned int)4));
	int oSize = (size / 2) - 2;
	int numVars = 1;
	for (int i = numTiers - 1; i > 0; i--)
	{
		numVars += pow(4,i);
	}
	int varOffset = (numVars - 1) / 4;
	if (var == 0)  //We are on the correct tier, so now determine the exact location in the 
	{			   //overall NWAOBDD using varOff and size and oSize
		if (vocabOff == 0)
		{
			return offset;
		}
		else if (vocabOff == 1)
		{
			return offset + 1;
		}
		else if (vocabOff == 2)
		{
			return offset + (oSize / 2);
		}
		else if (vocabOff == 3)
		{
			return offset + (oSize / 2) + 1;
		}
		else if (vocabOff == 4)
		{
			return offset + (size / 2) - 2;
		}
		else if (vocabOff == 5)
		{
			return offset + (size / 2) - 1;
		}
		else if (vocabOff == 6)
		{
			return offset + (size / 2);
		}
		else if (vocabOff == 7)
		{
			return offset + (size / 2) + 1;
		}
		else if (vocabOff == 8)
		{
			return offset + (size / 2) + (oSize / 2);
		}
		else if (vocabOff == 9)
		{
			return offset + (size / 2) + (oSize / 2) + 1;
		}
		else if (vocabOff == 10)
		{
			return offset + size - 2;
		}
		else
		{
			return offset + size - 1;
		}
	}
	else if (var <= varOffset)  //The variable is in the AA connection from this tier
	{
		return findVarLoc((var-1), offset + 2, level - 2,vocabOff);
	}
	else if (var > 3 * varOffset) //The variable is in the BB connection from this tier
	{
		return findVarLoc(var - (3 * varOffset) - 1, offset + (size / 2) + (oSize / 2) + 2, level - 2, vocabOff);
	}
	else if (var > 2 * varOffset)  //The variable is in the BA connection from this tier
	{
		return findVarLoc(var - (2 * varOffset) - 1, offset + (size / 2) + 2, level - 2, vocabOff);
	}
	else // (var > varOffset)  //The variable is in the AB connection from the tier
	{
		return findVarLoc(var - varOffset - 1, offset +  (oSize / 2) + 2, level - 2, vocabOff);
	}
}

/*
*  Get a map that maps each vocabulary in a tier to itself for use in the path summary code
*/
int * NWAOBDDContext::getUnchangedMap() const {
	int destinationMap[12];
	for (int i = 0; i < 12; i++)
	{
		destinationMap[i] = i;
	}
	return destinationMap;
}

/*
*  Fetch functions for the locations of the various vocabularies
*/
int NWAOBDDContext::getBasePreLoc() const { return basePreLoc; }

int NWAOBDDContext::getBasePostLoc() const {return basePostLoc;}

int NWAOBDDContext::getBaseExtraLoc() const {return baseExtraLoc;}

int NWAOBDDContext::getTensorPreLoc() const {return tensorPreLoc;}

int NWAOBDDContext::getTensorPostLoc() const {return tensorPostLoc;}

int NWAOBDDContext::getTensorExtraLoc() const {return tensorExtraLoc;}

int NWAOBDDContext::getRegAInfo() const {return regAInfo;}

ordering NWAOBDDContext::getOrdering() {
	return order;
}

int * NWAOBDDContext::getNoDistinctionMap() const {
	int destinationMap[12];
	for (int i = 0; i < 12; i++)
	{
		destinationMap[i] = NO_DISTINCTION;
	}
	return destinationMap;
}

int NWAOBDDContext::getVLoc(vocabulary_t v) const {
	switch (v){
	case BASE_PRE:
		return basePreLoc;
		break;
	case BASE_POST:
		return basePostLoc;
		break;
	case BASE_EXTRA:
		return baseExtraLoc;
		break;
	case TENSOR_PRE:
		return tensorPreLoc;
		break;
	case TENSOR_POST:
		return tensorPostLoc;
		break;
	case TENSOR_EXTRA:
		return tensorExtraLoc;
		break;
	case REG_A_INFO:
		return regAInfo;
		break;
	case REG_B_INFO:
		return regBInfo;
		break;
	case REG_C_INFO:
		return regCInfo;
		break;
	}
}

void NWAOBDDContext::setIntVars(const std::map<std::string, int>& inflatvars, int levels)
{
	int i = 0;
	for (std::map<std::string, int>::const_iterator ci = inflatvars.begin(); ci != inflatvars.end(); ++ci){
		this->varMap[ci->first] = i;
		i++;
	}

	this->numTotalVars = i;
	this->maxLevel = levels;

	NWAOBDDContext::setupCachedNWAOBDDs(this->order);
}

NWA_OBDD::NWAOBDD NWAOBDDContext::From(std::string lhs, std::string var)
{
	NWA_OBDD::NWAOBDD ret = NWA_OBDD::MkFalse();
	std::map<std::string,int>::const_iterator iter = (this->varMap.find(var));
	std::map<std::string, int>::const_iterator iter2 = (this->varMap.find(lhs));
	//if (iter == this->varMap.end())
		//LOG(FATAL) << "attempted from () on \"" << var << "\". i don't recognize this name\n";
	const int bi = iter->second;
	const int li = iter2->second;
	int vOff1 = getVLoc(BASE_PRE);
	int vOff2 = getVLoc(BASE_POST);
	int lhsLoc = findVarLoc(li, 0, this->maxLevel, vOff2);
	int rhsLoc = findVarLoc(bi, 0, this->maxLevel, vOff1);
	NWA_OBDD::NWAOBDD t1 = NWA_OBDD::MkDistinction(lhsLoc);
	NWA_OBDD::NWAOBDD n1 = NWA_OBDD::MkNot(t1);
	NWA_OBDD::NWAOBDD t2 = NWA_OBDD::MkDistinction(rhsLoc);
	NWA_OBDD::NWAOBDD n2 = NWA_OBDD::MkNot(t2);

	ret = NWA_OBDD::MkOr(NWA_OBDD::MkAnd(t1, t2), NWA_OBDD::MkAnd(n1, n2));
	return ret;
}

NWA_OBDD::NWAOBDD NWAOBDDContext::From(std::string var)
{
	std::map<std::string, int>::const_iterator iter = (this->varMap.find(var));
	//if (iter == this->varMap.end())
	//LOG(FATAL) << "attempted from () on \"" << var << "\". i don't recognize this name\n";
	const int bi = iter->second;
	int vOff1 = getVLoc(BASE_PRE);
	int vOff2 = getVLoc(REG_A_INFO);
	int preLoc = findVarLoc(bi, 0, this->maxLevel, vOff1);
	int regALoc = findVarLoc(0, 0, this->maxLevel, vOff2);
	NWA_OBDD::NWAOBDD pre = NWA_OBDD::MkDistinction(preLoc);
	NWA_OBDD::NWAOBDD extra = NWA_OBDD::MkDistinction(regALoc);

	NWA_OBDD::NWAOBDD ret = NWA_OBDD::MkOr(MkAnd(pre,extra), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(pre),NWA_OBDD::MkNot(extra)));
	return ret;
}

/*
*  These are the NWAOBDDs that will be used often, so only create them once in the beginning
*/
void NWAOBDDContext::setupCachedNWAOBDDs(ordering o)
{
  cachedBaseOne = new NWAOBDDRel(this, this->createBaseOneTop(o), false);
  cachedBaseZero = new NWAOBDDRel(this, this->createBaseZeroTop(o), false);
  cachedBaseTop = new NWAOBDDRel(this, this->createBaseTopTop(o), false);
  cachedTensorOne = new NWAOBDDRel(this, this->createTensorOneTop(o), true);
  cachedTensorZero = new NWAOBDDRel(this, this->createTensorZeroTop(o), true);
  cachedTensorTop = new NWAOBDDRel(this, this->createTensorTopTop(o), true);
  cachedDetensorId = new NWAOBDDRel(this, this->createDetensorId(), true);
  baseId = this->createBaseID(o);
}

/*
*  This is the NWAOBDD that represents equating A' = B;
*/
NWA_OBDD::NWAOBDD NWAOBDDContext::createDetensorId(){
	NWA_OBDD::NWAOBDD g;
	g = NWA_OBDD::MkTrue();
	int level = this->maxLevel;
	int v1 = getVLoc(BASE_PRE);
	int v2 = getVLoc(TENSOR_PRE);
	for (int j = 0; j < numTotalVars; j++) //This is a very innefficient way of doing things ETTODO - FIX!
	{
		int p1 = findVarLoc(j, 0, level, v1);
		int p2 = findVarLoc(j, 0, level, v2);
		g = NWA_OBDD::MkAnd(g, NWA_OBDD::MkOr(NWA_OBDD::MkAnd(NWA_OBDD::MkDistinction(p1), NWA_OBDD::MkDistinction(p2)), NWA_OBDD::MkNot(NWA_OBDD::MkOr(NWA_OBDD::MkDistinction(p1), NWA_OBDD::MkDistinction(p2)))));
	}

	return g;
}


NWA_OBDD::NWAOBDD NWAOBDDContext::createBaseID(ordering o){
	NWA_OBDD::NWAOBDD t1 = cachedBaseOne->getNWAOBDD();
	NWA_OBDD::NWAOBDD t2 = cachedBaseZero->getNWAOBDD();
	return NWA_OBDD::MkOr(t1, t2);
}

NWA_OBDD::NWAOBDD NWAOBDDContext::createBaseOneTop(ordering orderType)
{

	NWA_OBDD::NWAOBDDTopNodeRefPtr v;
	NWA_OBDD::NWAOBDDNodeHandle h;
	ReturnMapHandle<intpair> m;

	h = createBaseOne(this->maxLevel, orderType, 0);

	m.AddToEnd(intpair(1,1));
	m.AddToEnd(intpair(0,0));
	m.Canonicalize();

	v = new NWA_OBDD::NWAOBDDTopNode(h, m);

	return NWA_OBDD::NWAOBDD(v);
}

NWA_OBDD::NWAOBDD NWAOBDDContext::createBaseTopTop(ordering orderType)
{
	return NWA_OBDD::MkTrue();
}

NWA_OBDD::NWAOBDD NWAOBDDContext::createBaseZeroTop(ordering orderType)
{
	return NWA_OBDD::MkFalse();
}

NWA_OBDD::NWAOBDD NWAOBDDContext::createTensorOneTop(ordering orderType)
{
	NWA_OBDD::NWAOBDDTopNodeRefPtr v;
	NWA_OBDD::NWAOBDDNodeHandle h;
	ReturnMapHandle<intpair> m;

	h = createTensorOne(this->maxLevel, orderType);

	m.AddToEnd(intpair(1,1));
	m.AddToEnd(intpair(0,0));
	m.Canonicalize();

	v = new NWA_OBDD::NWAOBDDTopNode(h, m);
	
	return NWA_OBDD::NWAOBDD(v);
}


NWA_OBDD::NWAOBDD NWAOBDDContext::createTensorTopTop(ordering orderType)
{
	return NWA_OBDD::MkTrue();
}

NWA_OBDD::NWAOBDD NWAOBDDContext::createTensorZeroTop(ordering orderType)
{
	return NWA_OBDD::MkFalse();
}

NWA_OBDD::NWAOBDDNodeHandle NWAOBDDContext::createBaseOne(int level, ordering orderType, int offset)
{
	unsigned int size = ((unsigned int)((((unsigned int)1) << (level + 2)) - (unsigned int)4));//The number of possible variables in this level of NWA_OBDD::NWAOBDD
	int numTiers = level / 2;
	int numTotalVars = 1;
	int vals[12];
	for (int z = numTiers - 1; z > 0; z--)
	{
		numTotalVars += pow(4, z);
	}
	int varOffset = (numTotalVars - 1) / 4;
	int vars[5];
	vars[0] = offset;
	vars[1] = offset + 1;
	vars[2] = offset + varOffset + 1;
	vars[3] = offset + 2 * varOffset + 1;
	vars[4] = offset + 3 * varOffset + 1;
	NWA_OBDD::NWAOBDDInternalNode *nLower;
	switch (orderType)
	{
	/* This ordering is:
		TensorPre
			BasePre
			Base Post
			Base Extra
			*
		TensorPost
		* <---- A
			* <-------B
			* <-------C
			*
			*
		TensorExtra
	*/
	case BASE_1ST_TENSOR_ROOT:
		if (level == 0)
		{
			return NWA_OBDD::NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
		} 
		else if (level == 2)
		{
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level-1);
			ReturnMapHandle<intpair> m00, m01, m10, m11, m00_11;

			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m01.AddToEnd(intpair(0,1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1,0));
			m10.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();

			NWA_OBDD::NWAOBDDNodeHandle temp = createBaseOne(level - 2, orderType, 2 + offset);
			nUA->AConnection[0] = NWA_OBDD::Connection(temp,m01);
			nUA->AConnection[1] = NWA_OBDD::Connection(temp,m10);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
			nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUA->numExits = 2;
			
			NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);
			nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);
			nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
			nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

			nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

			nLower->numExits = 2;
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		} 
		else { //level > 2
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			NWA_OBDD::NWAOBDDInternalNode *nUB;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level-1);
			nUB = new NWA_OBDD::NWAOBDDInternalNode(level-1);
			ReturnMapHandle<intpair> m00, m11, m01, m10, m01_11, m10_11, m00_11;

			m00.AddToEnd(intpair(0, 0));
			m00.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m01.AddToEnd(intpair(0, 1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1, 0));
			m10.Canonicalize();

			m01_11.AddToEnd(intpair(0,1));
			m01_11.AddToEnd(intpair(1,1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1,0));
			m10_11.AddToEnd(intpair(1,1));
			m10_11.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();
			if (vars[1] < this->numTotalVars) {
				NWA_OBDD::NWAOBDDNodeHandle tempAA = createBaseOne(level - 2, orderType, 1 + offset);
				nUA->AConnection[0] = NWA_OBDD::Connection(tempAA, m01_11);
				nUA->AConnection[1] = NWA_OBDD::Connection(tempAA, m10_11);

				nUA->numBConnections = 2;
				nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
				nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

				if (vars[2] < this->numTotalVars)
				{
					NWA_OBDD::NWAOBDDNodeHandle tempAB = createBaseOne(level - 2, orderType, varOffset + 1 + offset);
					nUA->BConnection[0][0] = NWA_OBDD::Connection(tempAB, m00_11);
					nUA->BConnection[1][0] = NWA_OBDD::Connection(tempAB, m00_11);
					nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
					nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

					nUA->numExits = 2;

					NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);

					nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);
					nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);

					nLower->numBConnections = 2;

					nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
					nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

					if (vars[3] < this->numTotalVars)
					{
						NWA_OBDD::NWAOBDDNodeHandle tempBA = createBaseOne(level - 2, orderType, 2 * varOffset + 1 + offset);
						tempBA.Canonicalize();
						nUB->AConnection[0] = NWA_OBDD::Connection(tempBA, m00_11);
						nUB->AConnection[1] = NWA_OBDD::Connection(tempBA, m00_11);
						nUB->numBConnections = 2;

						nUB->BConnection[0] = new NWA_OBDD::Connection[nUB->numBConnections];
						nUB->BConnection[1] = new NWA_OBDD::Connection[nUB->numBConnections];

						if (vars[4] < this->numTotalVars)
						{
							NWA_OBDD::NWAOBDDNodeHandle tempBB = createBaseOne(level - 2, orderType, 3 * varOffset + 1 + offset);
							tempBB.Canonicalize();
							nUB->BConnection[0][0] = NWA_OBDD::Connection(tempBB, m00_11);
							nUB->BConnection[1][0] = NWA_OBDD::Connection(tempBB, m00_11);
							nUB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
							nUB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

							nUB->numExits = 2;
							NWA_OBDD::NWAOBDDNodeHandle tnUB = NWA_OBDD::NWAOBDDNodeHandle(nUB);

							nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

							nLower->numExits = 2;
						}
						else
						{
							nUB->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
							nUB->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
							nUB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
							nUB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

							nUB->numExits = 2;
							NWA_OBDD::NWAOBDDNodeHandle tnUB = NWA_OBDD::NWAOBDDNodeHandle(nUB);
							nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

							nLower->numExits = 2;
						}
					}
					else
					{
						nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
						nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
						nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
						nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
						nLower->numExits = 2;
					}
				}
				else
				{
					nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
					nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
					nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
					nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

					nUA->numExits = 2;

					NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);

					nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);
					nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);

					nLower->numBConnections = 2;

					nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
					nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

					nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
					nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
					nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
					nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

					nLower->numExits = 2;
				}
			}
			else
			{
				nUA->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m01);
				nUA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m10);

				nUA->numBConnections = 2;
				nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
				nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

				nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
				nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
				nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
				nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

				nUA->numExits = 2;
				NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);
				nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);
				nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);

				nLower->numBConnections = 2;

				nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
				nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

				nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
				nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
				nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
				nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

				nLower->numExits = 2;
			}

			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		break;
	case SEPERATE_LEVELS:
		if (level == 0)
		{
			return NWA_OBDD::NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
		}
		else if (level == 2)
		{
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			ReturnMapHandle<intpair> m00, m01, m10, m11, m00_11;

			m00.AddToEnd(intpair(0, 0));
			m00.Canonicalize();

			m01.AddToEnd(intpair(0, 1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1, 0));
			m10.Canonicalize();

			m11.AddToEnd(intpair(1, 1));
			m11.Canonicalize();

			m00_11.AddToEnd(intpair(0, 0));
			m00_11.AddToEnd(intpair(1, 1));
			m00_11.Canonicalize();

			NWA_OBDD::NWAOBDDNodeHandle temp = createBaseOne(level - 2, orderType, 2 + offset);
			nUA->AConnection[0] = NWA_OBDD::Connection(temp, m01);
			nUA->AConnection[1] = NWA_OBDD::Connection(temp, m10);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
			nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
			nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
			nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
			nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

			nUA->numExits = 2;

			NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);
			nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);
			nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
			nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

			nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
			nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
			nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

			nLower->numExits = 2;
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		else 
		{
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			NWA_OBDD::NWAOBDDInternalNode *nUB;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			nUB = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			ReturnMapHandle<intpair> m00, m11, m01, m10, m01_11, m10_11, m00_11;

			m00.AddToEnd(intpair(0, 0));
			m00.Canonicalize();

			m11.AddToEnd(intpair(1, 1));
			m11.Canonicalize();

			m01.AddToEnd(intpair(0, 1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1, 0));
			m10.Canonicalize();

			m01_11.AddToEnd(intpair(0, 1));
			m01_11.AddToEnd(intpair(1, 1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1, 0));
			m10_11.AddToEnd(intpair(1, 1));
			m10_11.Canonicalize();

			m00_11.AddToEnd(intpair(0, 0));
			m00_11.AddToEnd(intpair(1, 1));
			m00_11.Canonicalize();
			if (vars[1] < this->numTotalVars) {
				NWA_OBDD::NWAOBDDNodeHandle tempAA = createBaseOne(level - 2, orderType, 1 + offset);
				nUA->AConnection[0] = NWA_OBDD::Connection(tempAA, m01_11);
				nUA->AConnection[1] = NWA_OBDD::Connection(tempAA, m10_11);

				nUA->numBConnections = 2;
				nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
				nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

				if (vars[2] < this->numTotalVars)
				{
					NWA_OBDD::NWAOBDDNodeHandle tempAB = createBaseOne(level - 2, orderType, varOffset + 1 + offset);
					nUA->BConnection[0][0] = NWA_OBDD::Connection(tempAB, m00_11);
					nUA->BConnection[1][0] = NWA_OBDD::Connection(tempAB, m00_11);
					nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
					nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

					nUA->numExits = 2;

					NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);

					nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);
					nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);

					nLower->numBConnections = 2;

					nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
					nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

					if (vars[3] < this->numTotalVars)
					{
						NWA_OBDD::NWAOBDDNodeHandle tempBA = createBaseOne(level - 2, orderType, 2 * varOffset + 1 + offset);
						tempBA.Canonicalize();
						nUB->AConnection[0] = NWA_OBDD::Connection(tempBA, m00_11);
						nUB->AConnection[1] = NWA_OBDD::Connection(tempBA, m00_11);
						nUB->numBConnections = 2;

						nUB->BConnection[0] = new NWA_OBDD::Connection[nUB->numBConnections];
						nUB->BConnection[1] = new NWA_OBDD::Connection[nUB->numBConnections];

						if (vars[4] < this->numTotalVars)
						{
							NWA_OBDD::NWAOBDDNodeHandle tempBB = createBaseOne(level - 2, orderType, 3 * varOffset + 1 + offset);
							tempBB.Canonicalize();
							nUB->BConnection[0][0] = NWA_OBDD::Connection(tempBB, m00_11);
							nUB->BConnection[1][0] = NWA_OBDD::Connection(tempBB, m00_11);
							nUB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
							nUB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

							nUB->numExits = 2;
							NWA_OBDD::NWAOBDDNodeHandle tnUB = NWA_OBDD::NWAOBDDNodeHandle(nUB);

							nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

							nLower->numExits = 2;
						}
						else
						{
							nUB->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
							nUB->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
							nUB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
							nUB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

							nUB->numExits = 2;
							NWA_OBDD::NWAOBDDNodeHandle tnUB = NWA_OBDD::NWAOBDDNodeHandle(nUB);
							nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

							nLower->numExits = 2;
						}
					}
					else
					{
						nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
						nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
						nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
						nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
						nLower->numExits = 2;
					}
				}
				else
				{
					nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
					nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
					nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
					nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

					nUA->numExits = 2;

					NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);

					nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);
					nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);

					nLower->numBConnections = 2;

					nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
					nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

					nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
					nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
					nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
					nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

					nLower->numExits = 2;
				}
			}
			else
			{
				nUA->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m01);
				nUA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m10);

				nUA->numBConnections = 2;
				nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
				nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

				nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
				nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
				nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
				nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

				nUA->numExits = 2;
				NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);
				nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);
				nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m00_11);

				nLower->numBConnections = 2;

				nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
				nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

				nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
				nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
				nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
				nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

				nLower->numExits = 2;
			}

			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		break;
	case AFFINITY:
		if (level == 0)
		{
			return NWA_OBDD::NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
		}
		else if (level == 2)
		{
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			ReturnMapHandle<intpair> m00, m01, m10, m11, m00_11;

			m00.AddToEnd(intpair(0, 0));
			m00.Canonicalize();

			m01.AddToEnd(intpair(0, 1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1, 0));
			m10.Canonicalize();

			m11.AddToEnd(intpair(1, 1));
			m11.Canonicalize();

			m00_11.AddToEnd(intpair(0, 0));
			m00_11.AddToEnd(intpair(1, 1));
			m00_11.Canonicalize();

			NWA_OBDD::NWAOBDDNodeHandle temp = createBaseOne(level - 2, orderType, 2 + offset);
			
			nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m01);
			nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m01);

			nLower->numBConnections = 2;
			
			nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
			nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

			
			nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
			nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);

			nLower->numExits = 2;
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		else
		{
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			NWA_OBDD::NWAOBDDInternalNode *nUB;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			nUB = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			ReturnMapHandle<intpair> m00, m11, m01, m10, m01_11, m10_11, m00_11, m01_22;

			m00.AddToEnd(intpair(0, 0));
			m00.Canonicalize();

			m11.AddToEnd(intpair(1, 1));
			m11.Canonicalize();

			m01.AddToEnd(intpair(0, 1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1, 0));
			m10.Canonicalize();

			m01_11.AddToEnd(intpair(0, 1));
			m01_11.AddToEnd(intpair(1, 1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1, 0));
			m10_11.AddToEnd(intpair(1, 1));
			m10_11.Canonicalize();

			m00_11.AddToEnd(intpair(0, 0));
			m00_11.AddToEnd(intpair(1, 1));
			m00_11.Canonicalize();

			m01_22.AddToEnd(intpair(0, 1));
			m01_22.AddToEnd(intpair(2, 2));
			if (vars[1] < this->numTotalVars) {
				NWA_OBDD::NWAOBDDNodeHandle tempAA = createBaseOne(level - 2, orderType, 1 + offset);
				nUA->AConnection[0] = NWA_OBDD::Connection(tempAA, m00_11);
				nUA->AConnection[1] = NWA_OBDD::Connection(tempAA, m00_11);

				nUA->numBConnections = 2;
				nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
				nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

				if (vars[2] < this->numTotalVars)
				{
					NWA_OBDD::NWAOBDDNodeHandle tempAB = createBaseOne(level - 2, orderType, varOffset + 1 + offset);
					nUA->BConnection[0][0] = NWA_OBDD::Connection(tempAB, m00_11);
					nUA->BConnection[1][0] = NWA_OBDD::Connection(tempAB, m00_11);
					nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
					nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

					nUA->numExits = 2;

					NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);

					nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m01_22);
					nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m01_22);

					nLower->numBConnections = 3;

					nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
					nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];
					
					if (vars[3] < this->numTotalVars)
					{
						NWA_OBDD::NWAOBDDNodeHandle tempBA = createBaseOne(level - 2, orderType, 2 * varOffset + 1 + offset);
						tempBA.Canonicalize();
						nUB->AConnection[0] = NWA_OBDD::Connection(tempBA, m00_11);
						nUB->AConnection[1] = NWA_OBDD::Connection(tempBA, m00_11);
						nUB->numBConnections = 2;

						nUB->BConnection[0] = new NWA_OBDD::Connection[nUB->numBConnections];
						nUB->BConnection[1] = new NWA_OBDD::Connection[nUB->numBConnections];

						if (vars[4] < this->numTotalVars)
						{
							NWA_OBDD::NWAOBDDNodeHandle tempBB = createBaseOne(level - 2, orderType, 3 * varOffset + 1 + offset);
							tempBB.Canonicalize();
							nUB->BConnection[0][0] = NWA_OBDD::Connection(tempBB, m00_11);
							nUB->BConnection[1][0] = NWA_OBDD::Connection(tempBB, m00_11);
							nUB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
							nUB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

							nUB->numExits = 2;
							NWA_OBDD::NWAOBDDNodeHandle tnUB = NWA_OBDD::NWAOBDDNodeHandle(nUB);

							nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[0][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[1][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->numExits = 2;
						}
						else
						{
							nUB->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
							nUB->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
							nUB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
							nUB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
							nUB->numExits = 2;
							NWA_OBDD::NWAOBDDNodeHandle tnUB = NWA_OBDD::NWAOBDDNodeHandle(nUB);
							nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUB), m00_11);
							nLower->BConnection[0][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
							nLower->BConnection[1][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

							nLower->numExits = 2;
						}
					}
					else
					{
						nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
						nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
						nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
						nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
						nLower->BConnection[0][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
						nLower->BConnection[1][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
						nLower->numExits = 2;
					}
				}
				else
				{
					nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
					nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
					nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
					nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

					nUA->numExits = 2;

					NWA_OBDD::NWAOBDDNodeHandle tnUA = NWA_OBDD::NWAOBDDNodeHandle(nUA);

					nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m01_22);
					nLower->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle(tnUA), m01_22);

					nLower->numBConnections = 3;

					nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
					nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

					nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
					nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
					nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
					nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
					nLower->BConnection[0][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
					nLower->BConnection[1][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
					nLower->numExits = 2;
				}
			}
			else
			{
				nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m01);
				nLower->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m01);

				nLower->numBConnections = 2;

				nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
				nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

				nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
				nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
				nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
				nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);

				nLower->numExits = 2;
			}
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		break;
	}
}
// The various get functions

NWA_OBDD::NWAOBDDNodeHandle NWAOBDDContext::createTensorOne(int level, ordering orderType)
{
	NWA_OBDD::NWAOBDDInternalNode *nLower;
	switch (orderType)
	{
	/* This ordering is:
		TensorPre
			BasePre
			Base Post
			Base Extra
			*
		TensorPost
			*
			*
			*
			*
		TensorExtra
	*/
	case BASE_1ST_TENSOR_ROOT:
		if (level == 0)
		{
			return NWA_OBDD::NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
		} 
		else if (level == 2)
		{
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level-1);
			ReturnMapHandle<intpair> m00, m01, m10, m11, m01_11, m10_11;

			m00.AddToEnd(intpair(0,0));
			m00.Canonicalize();

			m01.AddToEnd(intpair(0,1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1,0));
			m10.Canonicalize();

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m01_11.AddToEnd(intpair(0,1));
			m01_11.AddToEnd(intpair(1,1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1,0));
			m10_11.AddToEnd(intpair(1,1));
			m10_11.Canonicalize();

			NWA_OBDD::NWAOBDDNodeHandle temp = createTensorOne(level-2, orderType);
			nUA->AConnection[0] = NWA_OBDD::Connection(temp,m01);
			nUA->AConnection[1] = NWA_OBDD::Connection(temp,m10);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
			nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m00);
			nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUA->numExits = 2;

			NWA_OBDD::NWAOBDDNodeHandle t2 = NWA_OBDD::NWAOBDDNodeHandle(nUA);
			
			nLower->AConnection[0] = NWA_OBDD::Connection(t2, m01_11);
			nLower->AConnection[1] = NWA_OBDD::Connection(t2, m10_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
			nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

			nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
			nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

			nLower->numExits = 2;
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		} 
		else { //level > 2
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			NWA_OBDD::NWAOBDDInternalNode *nUB;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level-1);
			nUB = new NWA_OBDD::NWAOBDDInternalNode(level-1);
			ReturnMapHandle<intpair> m11, m01_11, m10_11, m00_11;

			m11.AddToEnd(intpair(1,1));
			m11.Canonicalize();

			m01_11.AddToEnd(intpair(0,1));
			m01_11.AddToEnd(intpair(1,1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1,0));
			m10_11.AddToEnd(intpair(1,1));
			m10_11.Canonicalize();

			m00_11.AddToEnd(intpair(0,0));
			m00_11.AddToEnd(intpair(1,1));
			m00_11.Canonicalize();

			NWA_OBDD::NWAOBDDNodeHandle temp = createTensorOne(level-2, orderType);
			nUA->AConnection[0] = NWA_OBDD::Connection(temp,m01_11);
			nUA->AConnection[1] = NWA_OBDD::Connection(temp,m10_11);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
			nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = NWA_OBDD::Connection(temp, m00_11);
			nUA->BConnection[1][0] = NWA_OBDD::Connection(temp, m00_11);
			nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUA->numExits = 2; 

			NWA_OBDD::NWAOBDDNodeHandle t2 = NWA_OBDD::NWAOBDDNodeHandle(nUA);
			
			nLower->AConnection[0] = NWA_OBDD::Connection(t2, m01_11);
			nLower->AConnection[1] = NWA_OBDD::Connection(t2, m10_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
			nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];


			nUB->AConnection[0] = NWA_OBDD::Connection(temp,m00_11);
			nUB->AConnection[1] = NWA_OBDD::Connection(temp,m00_11);
			nUB->numBConnections = 2;
			
			nUB->BConnection[0] = new NWA_OBDD::Connection[nUB->numBConnections];
			nUB->BConnection[1] = new NWA_OBDD::Connection[nUB->numBConnections];

			nUB->BConnection[0][0] = NWA_OBDD::Connection(temp, m00_11);
			nUB->BConnection[1][0] = NWA_OBDD::Connection(temp, m00_11);
			nUB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);
			nUB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-2],m11);

			nUB->numExits = 2;

			NWA_OBDD::NWAOBDDNodeHandle t3 = NWA_OBDD::NWAOBDDNodeHandle(nUB);

			nLower->BConnection[0][0] = NWA_OBDD::Connection(t3,m00_11);
			nLower->BConnection[1][0] = NWA_OBDD::Connection(t3,m00_11);
			nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
			nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);

			nLower->numExits = 2;
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		break;
	case SEPERATE_LEVELS:
		if (level == 0)
		{
			return NWA_OBDD::NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
		}
		else if (level == 2)
		{
			NWA_OBDD::NWAOBDDInternalNode *nUA, *nUB;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			ReturnMapHandle<intpair> m00, m01, m10, m11, m01_11, m10_11, m00_11;

			m00.AddToEnd(intpair(0, 0));
			m00.Canonicalize();

			m01.AddToEnd(intpair(0, 1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1, 0));
			m10.Canonicalize();

			m11.AddToEnd(intpair(1, 1));
			m11.Canonicalize();

			m00_11.AddToEnd(intpair(0, 0));
			m00_11.AddToEnd(intpair(1, 1));
			m00.Canonicalize();

			m01_11.AddToEnd(intpair(0, 1));
			m01_11.AddToEnd(intpair(1, 1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1, 0));
			m10_11.AddToEnd(intpair(1, 1));
			m10_11.Canonicalize();

			NWA_OBDD::NWAOBDDNodeHandle temp = createTensorOne(level - 2, orderType);
			nUA->AConnection[0] = NWA_OBDD::Connection(temp, m01);
			nUA->AConnection[1] = NWA_OBDD::Connection(temp, m10);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
			nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
			nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
			nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
			nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

			nUA->numExits = 2;

			NWA_OBDD::NWAOBDDNodeHandle t2 = NWA_OBDD::NWAOBDDNodeHandle(nUA);

			nLower->AConnection[0] = NWA_OBDD::Connection(t2, m00_11);
			nLower->AConnection[1] = NWA_OBDD::Connection(t2, m00_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
			nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

			nLower->BConnection[0][0] = NWA_OBDD::Connection(t2, m00_11);
			nLower->BConnection[1][0] = NWA_OBDD::Connection(t2, m00_11);
			nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

			nLower->numExits = 2;
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		else { //level > 2
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			NWA_OBDD::NWAOBDDInternalNode *nUB;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			nUB = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			ReturnMapHandle<intpair> m11, m01_11, m10_11, m00_11;

			m11.AddToEnd(intpair(1, 1));
			m11.Canonicalize();

			m01_11.AddToEnd(intpair(0, 1));
			m01_11.AddToEnd(intpair(1, 1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1, 0));
			m10_11.AddToEnd(intpair(1, 1));
			m10_11.Canonicalize();

			m00_11.AddToEnd(intpair(0, 0));
			m00_11.AddToEnd(intpair(1, 1));
			m00_11.Canonicalize();

			NWA_OBDD::NWAOBDDNodeHandle temp = createTensorOne(level - 2, orderType);
			nUA->AConnection[0] = NWA_OBDD::Connection(temp, m01_11);
			nUA->AConnection[1] = NWA_OBDD::Connection(temp, m10_11);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
			nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = NWA_OBDD::Connection(temp, m00_11);
			nUA->BConnection[1][0] = NWA_OBDD::Connection(temp, m00_11);
			nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
			nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

			nUA->numExits = 2;

			NWA_OBDD::NWAOBDDNodeHandle t2 = NWA_OBDD::NWAOBDDNodeHandle(nUA);

			nLower->AConnection[0] = NWA_OBDD::Connection(t2, m00_11);
			nLower->AConnection[1] = NWA_OBDD::Connection(t2, m00_11);

			nLower->numBConnections = 2;

			nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
			nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];


			nUB->AConnection[0] = NWA_OBDD::Connection(temp, m01_11);
			nUB->AConnection[1] = NWA_OBDD::Connection(temp, m10_11);
			nUB->numBConnections = 2;

			nUB->BConnection[0] = new NWA_OBDD::Connection[nUB->numBConnections];
			nUB->BConnection[1] = new NWA_OBDD::Connection[nUB->numBConnections];

			nUB->BConnection[0][0] = NWA_OBDD::Connection(temp, m00_11);
			nUB->BConnection[1][0] = NWA_OBDD::Connection(temp, m00_11);
			nUB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
			nUB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

			nUB->numExits = 2;

			NWA_OBDD::NWAOBDDNodeHandle t3 = NWA_OBDD::NWAOBDDNodeHandle(nUB);

			nLower->BConnection[0][0] = NWA_OBDD::Connection(t3, m00_11);
			nLower->BConnection[1][0] = NWA_OBDD::Connection(t3, m00_11);
			nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

			nLower->numExits = 2;
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		break;
	case AFFINITY:
		if (level == 0)
		{
			return NWA_OBDD::NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
		}
		else if (level == 2)
		{
			NWA_OBDD::NWAOBDDInternalNode *nUA, *nUB;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			ReturnMapHandle<intpair> m00, m01, m10, m11, m01_11, m10_11, m00_11, m22_01, m01_22;

			m00.AddToEnd(intpair(0, 0));
			m00.Canonicalize();

			m01.AddToEnd(intpair(0, 1));
			m01.Canonicalize();

			m10.AddToEnd(intpair(1, 0));
			m10.Canonicalize();

			m11.AddToEnd(intpair(1, 1));
			m11.Canonicalize();

			m00_11.AddToEnd(intpair(0, 0));
			m00_11.AddToEnd(intpair(1, 1));
			m00.Canonicalize();

			m01_11.AddToEnd(intpair(0, 1));
			m01_11.AddToEnd(intpair(1, 1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1, 0));
			m10_11.AddToEnd(intpair(1, 1));
			m10_11.Canonicalize();

			m22_01.AddToEnd(intpair(2, 2));
			m22_01.AddToEnd(intpair(0, 1));
			m22_01.Canonicalize();

			m01_22.AddToEnd(intpair(0, 1));
			m01_22.AddToEnd(intpair(2, 2));
			m01_22.Canonicalize();

			NWA_OBDD::NWAOBDDNodeHandle temp = createTensorOne(level - 2, orderType);
			nUA->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
			nUA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);

			nUA->numBConnections = 1;
			nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
			nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m00);
			nUA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
			nUA->numExits = 2;

			NWA_OBDD::NWAOBDDNodeHandle t2 = NWA_OBDD::NWAOBDDNodeHandle(nUA);

			nLower->AConnection[0] = NWA_OBDD::Connection(t2, m01_22);
			nLower->AConnection[1] = NWA_OBDD::Connection(t2, m22_01);

			nLower->numBConnections = 3;

			nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
			nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];

			nLower->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
			nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m00);
			nLower->BConnection[1][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[0][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);


			nLower->numExits = 2;
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		else { //level > 2
			NWA_OBDD::NWAOBDDInternalNode *nUA;
			NWA_OBDD::NWAOBDDInternalNode *nUB;
			nLower = new NWA_OBDD::NWAOBDDInternalNode(level);
			nUA = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			nUB = new NWA_OBDD::NWAOBDDInternalNode(level - 1);
			ReturnMapHandle<intpair> m11, m01_11, m10_11, m00_11, m01_22_22, m22_22_01, m22_11, m22_01, m01_22;

			m11.AddToEnd(intpair(1, 1));
			m11.Canonicalize();

			m01_11.AddToEnd(intpair(0, 1));
			m01_11.AddToEnd(intpair(1, 1));
			m01_11.Canonicalize();

			m10_11.AddToEnd(intpair(1, 0));
			m10_11.AddToEnd(intpair(1, 1));
			m10_11.Canonicalize();

			m00_11.AddToEnd(intpair(0, 0));
			m00_11.AddToEnd(intpair(1, 1));
			m00_11.Canonicalize();

			m22_22_01.AddToEnd(intpair(2, 2));
			m22_22_01.AddToEnd(intpair(2, 2));
			m22_22_01.AddToEnd(intpair(0, 1));
			m22_22_01.Canonicalize();

			m22_11.AddToEnd(intpair(2, 2));
			m22_11.AddToEnd(intpair(1, 1));
			m22_11.Canonicalize();

			m22_01.AddToEnd(intpair(2, 2));
			m22_01.AddToEnd(intpair(0, 1));
			m22_01.Canonicalize();

			m01_22.AddToEnd(intpair(0, 1));
			m01_22.AddToEnd(intpair(2, 2));
			m01_22.Canonicalize();

			m01_22_22.AddToEnd(intpair(0, 1));
			m01_22_22.AddToEnd(intpair(2, 2));
			m01_22_22.AddToEnd(intpair(2, 2));
			m01_22_22.Canonicalize();

			NWA_OBDD::NWAOBDDNodeHandle temp = createTensorOne(level - 2, orderType);
			nUA->AConnection[0] = NWA_OBDD::Connection(temp, m00_11);
			nUA->AConnection[1] = NWA_OBDD::Connection(temp, m00_11);

			nUA->numBConnections = 2;
			nUA->BConnection[0] = new NWA_OBDD::Connection[nUA->numBConnections];
			nUA->BConnection[1] = new NWA_OBDD::Connection[nUA->numBConnections];

			nUA->BConnection[0][0] = NWA_OBDD::Connection(temp, m00_11);
			nUA->BConnection[1][0] = NWA_OBDD::Connection(temp, m22_11);
			nUA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
			nUA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

			nUA->numExits = 3;

			NWA_OBDD::NWAOBDDNodeHandle t2 = NWA_OBDD::NWAOBDDNodeHandle(nUA);

			nLower->AConnection[0] = NWA_OBDD::Connection(t2, m01_22_22);
			nLower->AConnection[1] = NWA_OBDD::Connection(t2, m22_22_01);

			nLower->numBConnections = 3;

			nLower->BConnection[0] = new NWA_OBDD::Connection[nLower->numBConnections];
			nLower->BConnection[1] = new NWA_OBDD::Connection[nLower->numBConnections];


			nUB->AConnection[0] = NWA_OBDD::Connection(temp, m00_11);
			nUB->AConnection[1] = NWA_OBDD::Connection(temp, m00_11);
			nUB->numBConnections = 2;

			nUB->BConnection[0] = new NWA_OBDD::Connection[nUB->numBConnections];
			nUB->BConnection[1] = new NWA_OBDD::Connection[nUB->numBConnections];

			nUB->BConnection[0][0] = NWA_OBDD::Connection(temp, m00_11);
			nUB->BConnection[1][0] = NWA_OBDD::Connection(temp, m00_11);
			nUB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);
			nUB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 2], m11);

			nUB->numExits = 2;

			NWA_OBDD::NWAOBDDNodeHandle t3 = NWA_OBDD::NWAOBDDNodeHandle(nUB);

			nLower->BConnection[0][0] = NWA_OBDD::Connection(t3, m00_11);
			nLower->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[1][1] = NWA_OBDD::Connection(t3, m00_11);
			nLower->BConnection[1][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);
			nLower->BConnection[0][2] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[level - 1], m11);

			nLower->numExits = 2;
			return NWA_OBDD::NWAOBDDNodeHandle(nLower);
		}
		break;
	}
}


nwaobdd_t NWAOBDDContext::getBaseOne() const
{
  return cachedBaseOne; 
}

nwaobdd_t NWAOBDDContext::getBaseZero() const
{
  return cachedBaseZero;
}

nwaobdd_t NWAOBDDContext::getTensorOne() const
{
  return cachedTensorOne;
}

nwaobdd_t NWAOBDDContext::getTensorZero() const
{
  return cachedTensorZero;
}

nwaobdd_t NWAOBDDContext::getBaseTop() const
{
  return cachedBaseTop;
}

nwaobdd_t NWAOBDDContext::getTensorTop() const
{
  return cachedTensorTop;
}

nwaobdd_t NWAOBDDContext::getDetensorId() const
{
	return cachedDetensorId;
}

void NWAOBDDContext::populateCache(ordering o) 
{
	setupCachedNWAOBDDs(o);
}

// ////////////////////////////
// Static
void NWAOBDDRel::reset()
{
}

// ////////////////////////////
// Friends
// ////////////////////////////
// Friends
namespace wali{
	namespace domains{
		namespace nwaobddrel{
			nwaobdd_t operator*(nwaobdd_t a, nwaobdd_t b)
			{
				return a->Compose(b);
			}

			nwaobdd_t operator|(nwaobdd_t a, nwaobdd_t b)
			{
				return a->Union(b);
			}

			nwaobdd_t operator&(nwaobdd_t a, nwaobdd_t b)
			{
				return a->Intersect(b);
			}
		}
	}
}
	  // ////////////////////////////
// Members and Con/Destructors
NWAOBDDRel::NWAOBDDRel(const NWAOBDDRel& that) : 
  wali::SemElemTensor(that),
  con(that.con),
  rel(that.rel),
  isTensored(that.isTensored)
{}

NWAOBDDRel::NWAOBDDRel(NWAOBDDContext const * c, NWA_OBDD::NWAOBDD b, bool it) : 
  con(c),
  rel(b), 
  isTensored(it) 
{}
NWAOBDDRel::~NWAOBDDRel() {}
// ////////////////////////////

nwaobdd_t NWAOBDDRel::Compose( nwaobdd_t that ) const
{
  //We skip this test if you insist
#ifndef BINREL_HASTY
  if(isTensored != that->isTensored || con != that->con){
    *waliErr << "[WARNING] " << "Composing incompatible relations" << endl;
    that->print(print(*waliErr) << endl) << endl;
    assert(false);
    return new NWAOBDDRel(con,NWA_OBDD::MkFalse(),isTensored);
  }
#endif
  if (this->isZero() || that->isZero())
    return static_cast<NWAOBDDRel*>(zero().get_ptr());;
  if (this->isOne())
    return that;
  if (that->isOne())
    return new NWAOBDDRel(*this);

  NWA_OBDD::NWAOBDD c;
	if (!this->isTensored) {
		int BaseRightShiftMap[12];
		this->CreateBaseRightShiftMap(BaseRightShiftMap);
		NWA_OBDD::NWAOBDD temp1 = AdjustedPathSummary(that->rel, BaseRightShiftMap);
		NWA_OBDD::NWAOBDD temp2 = MkAnd(this->rel, temp1);
		vocabulary_t v[1];
		v[0] = BASE_POST;
		NWAOBDDRel * t2 = new NWAOBDDRel(this->con, temp2, false);
		nwaobdd_t temp3 = t2->ExistentialQuantification(v, 1);
		int BaseRestoreMap[12];
		CreateBaseRestoreMap(BaseRestoreMap);
		c = AdjustedPathSummary(temp3->rel, BaseRestoreMap);
	} else
	{
		int TensorRightShiftMap[12];
		CreateTensorRightShiftMap(TensorRightShiftMap);
		NWA_OBDD::NWAOBDD temp1 = AdjustedPathSummary(that->rel, TensorRightShiftMap);
		NWA_OBDD::NWAOBDD temp2 = MkAnd(this->rel, temp1);
		vocabulary_t v[1];
		v[0] = BASE_POST;
		NWAOBDDRel * t2 = new NWAOBDDRel(this->con, temp2, false);
		nwaobdd_t temp3 = t2->ExistentialQuantification(v, 1);
		int TensorRestoreMap[12];
		CreateTensorRestoreMap(TensorRestoreMap);
		c = AdjustedPathSummary(temp2, TensorRestoreMap);
	}

  nwaobdd_t ret = new NWAOBDDRel(con,c,isTensored);
  // Keep zero/one unique.
  if(ret->isZero())
    return static_cast<NWAOBDDRel*>(ret->zero().get_ptr());
  if(ret->isOne())
    return static_cast<NWAOBDDRel*>(ret->one().get_ptr());
  return ret;
}

nwaobdd_t NWAOBDDRel::Union( nwaobdd_t that ) const
{
  //We skip this test if you insist
#ifndef BINREL_HASTY
  if(isTensored != that->isTensored || con != that->con){
    *waliErr << "[WARNING] " << "Unioning incompatible relations" << endl;
    that->print(print(*waliErr) << endl) << endl;
    assert(false);
    return new NWAOBDDRel(con,NWA_OBDD::MkTrue(),isTensored);
  }
#endif
  if (this->isZero())
    return that;
  if (that->isZero())
    return new NWAOBDDRel(*this);

  // Keep zero/one unique
  nwaobdd_t ret = new NWAOBDDRel(con,NWA_OBDD::MkOr(rel,that->rel), isTensored);
  if(ret->isOne())
    return static_cast<NWAOBDDRel*>(ret->one().get_ptr());
  //can't be zero.
  return ret;
}

bool NWAOBDDRel::isZero()
{
	if (!isTensored)
	{
		return (this->rel == this->con->getBaseZero()->rel);
	}
	else
	{
		return (this->rel == this->con->getTensorZero()->rel);
	}
}

bool NWAOBDDRel::isOne()
{
	if (!isTensored)
	{
		return (this->rel == this->con->getBaseOne()->rel);
	}
	else
	{
		return (this->rel == this->con->getTensorOne()->rel);
	}
}


nwaobdd_t NWAOBDDRel::Intersect( nwaobdd_t that ) const
{
#ifdef BINREL_STATS
  con->numIntersect++;
#endif
  //We skip this test if you insist
#ifndef BINREL_HASTY
  if(isTensored != that->isTensored || con != that->con){
    *waliErr << "[WARNING] " << "Intersecting incompatible relations" << endl;
    that->print(print(*waliErr) << endl) << endl;
    assert(false);
    return new NWAOBDDRel(con,NWA_OBDD::MkFalse(),isTensored);
  }
#endif
  if(this->isZero() || that->isZero())
    return static_cast<NWAOBDDRel*>(this->zero().get_ptr());

  // Keep zero/one unique
  nwaobdd_t ret = new NWAOBDDRel(con, NWA_OBDD::MkAnd(rel,that->rel),isTensored);
  if(ret->isZero())
    return static_cast<NWAOBDDRel*>(ret->zero().get_ptr());
  if(ret->isOne())
    return static_cast<NWAOBDDRel*>(ret->one().get_ptr());
  return ret;
}

bool NWAOBDDRel::Equal( nwaobdd_t that) const
{
#ifdef BINREL_STATS
  con->numEqual++;
#endif
  //We skip this test if you insist
#ifndef BINREL_HASTY
  if(isTensored != that->isTensored /*|| con != that->con*/){
    std::cerr << "con: " << con << "\n";
    std::cerr << "that->con: " << that->con << "\n";
    *waliErr << "[WARNING] " << "Compared (Equality) incompatible relations" 
      << endl;
    that->print(print(*waliErr) << endl) << endl;
    assert(false);
    return false;
  }
#endif
  return rel == that->rel;
}

nwaobdd_t NWAOBDDRel::Transpose() const
{
#ifndef BINREL_HASTY
  if(isTensored){
    *waliErr << "[WARNING] " << "Attempted to transpose tensored weight."
      << endl << "Not supported" << endl;
    print(*waliErr) << endl;
    assert(false);
    return new NWAOBDDRel(con, NWA_OBDD::MkFalse(), true);
  }
#endif
  if(this->isZero())
    return static_cast<NWAOBDDRel*>(zero().get_ptr());
  if(this->isOne())
    return static_cast<NWAOBDDRel*>(one().get_ptr());

  	NWA_OBDD::NWAOBDD g;
	int TransposeMap[12];
	CreateTransposeMap(TransposeMap);
	g = AdjustedPathSummary(this->rel, TransposeMap);
  return new NWAOBDDRel(con, g, isTensored);
}

nwaobdd_t NWAOBDDRel::Kronecker(nwaobdd_t that) const
{
#ifdef BINREL_STATS
  con->numKronecker++;
#endif
#ifndef BINREL_HASTY
  if(isTensored || that->isTensored || con != that->con){
    *waliErr << "[WARNING] " << "Attempted to tensor two tensored weights OR attempted to tensor uncompatible relations."
      << endl << "Not supported" << endl;
    that->print(print(*waliErr) << endl) << endl;
    assert(false);
    return new NWAOBDDRel(con, NWA_OBDD::MkFalse(), true);
  }
#endif
  if(rel == NWA_OBDD::MkFalse() || that->rel == NWA_OBDD::MkFalse())
    return con->cachedTensorZero;

	NWA_OBDD::NWAOBDD g;
	int TensorMap[12];
	this->CreateMoveBaseToTensorMap(TensorMap);
	g = AdjustedPathSummary(that->rel, TensorMap);
	NWA_OBDD::NWAOBDD c = MkAnd(this->rel,g);
  nwaobdd_t ret = new NWAOBDDRel(con, c,true);
  if(ret->isZero())
    return static_cast<NWAOBDDRel*>(ret->zero().get_ptr());
  if(ret->isOne())
    return static_cast<NWAOBDDRel*>(ret->one().get_ptr());
  return ret;
}

nwaobdd_t NWAOBDDRel::Merge(int v, int c) const
{
	std::pair<std::vector<std::string>, std::vector<std::string>> sList = con->getLocalVars(std::pair<int, int>(v,c));
	std::vector<std::string> vars1 = sList.first;
	std::vector<std::string> vars2 = sList.second;
	NWA_OBDD::NWAOBDD havocCalleeLocalsNWAOBDD = con->getBaseOne()->getNWAOBDD();
	NWA_OBDD::NWAOBDD constrainLocalsNWAOBDD = con->getBaseTop()->getNWAOBDD();
	for (std::vector<std::string>::const_iterator cit = vars1.begin(); cit != vars1.end(); ++cit){
		if (con->varMap.find(*cit) == con->varMap.end()){
			std::cerr << "Unknown variable " << *cit << endl;
			assert(0);
		}
		// for each local variable:
		// havoc the post vocabulary for that variables
		int bi = con->varMap.find(*cit)->second;
		int varOff = con->getVLoc(BASE_POST);
		int vLoc = con->findVarLoc(bi, 0, con->maxLevel, varOff);
		NWA_OBDD::NWAOBDD t1 = NWA_OBDD::MkDistinction(vLoc);
		havocCalleeLocalsNWAOBDD = NWA_OBDD::MkExists(havocCalleeLocalsNWAOBDD, vLoc);
	}
	for (std::vector<std::string>::const_iterator cit = vars2.begin(); cit != vars2.end(); ++cit)
	{
		// for each local variable:
		// havoc the post vocabulary for that variables
		int bi = con->varMap.find(*cit)->second;
		int varOff = con->getVLoc(BASE_POST);
		int varOff2 = con->getVLoc(BASE_PRE);
		int vLoc2 = con->findVarLoc(bi, 0, con->maxLevel, varOff2);
		int vLoc = con->findVarLoc(bi, 0, con->maxLevel, varOff);
		NWA_OBDD::NWAOBDD t1 = NWA_OBDD::MkDistinction(vLoc);
		NWA_OBDD::NWAOBDD t2 = NWA_OBDD::MkDistinction(vLoc2);
		constrainLocalsNWAOBDD = NWA_OBDD::MkAnd(constrainLocalsNWAOBDD, NWA_OBDD::MkOr(NWA_OBDD::MkAnd(t1, t2), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(t1), NWA_OBDD::MkNot(t2))));
	}
	NWAOBDDRel * hL = new NWAOBDDRel(con, havocCalleeLocalsNWAOBDD, false);
	NWAOBDDRel * cL = new NWAOBDDRel(con, constrainLocalsNWAOBDD, false);
	nwaobdd_t ret;
	ret = this->Compose(hL);
	ret = ret->Intersect(cL);

	return ret;
}


// ////////////////////////////
// SemElem Interface functions

wali::sem_elem_t NWAOBDDRel::star()
{
  StarCacheKey k(getNWAOBDD(), isTensored);
  if (star_cache.find(k) == star_cache.end()) {
    sem_elem_t w = combine(one().get_ptr());
    sem_elem_t wn = w->extend(w);
    while(!w->equal(wn)) {
      w = wn;
      wn = wn->extend(wn);
    }
    star_cache[k] = wn;
  }

  return star_cache[k];
}


wali::sem_elem_t NWAOBDDRel::combine(wali::SemElem* se) 
{
  nwaobdd_t that( convert(se) );
  return Union(that);
}

wali::sem_elem_t NWAOBDDRel::extend(wali::SemElem* se) 
{
  nwaobdd_t that( convert(se) );
  return Compose(that);
}

bool NWAOBDDRel::equal(wali::SemElem* se) const 
{
  nwaobdd_t that( convert(se) );
  return Equal(that);
}

bool NWAOBDDRel::containerLessThan(wali::SemElem const * se) const
{
  NWAOBDDRel const * other = dynamic_cast<NWAOBDDRel const *>(se);
  return this->getNWAOBDD().Hash(256) < other->getNWAOBDD().Hash(256); //ETTODO Fix
}


wali::sem_elem_t NWAOBDDRel::one() const
{
  if (!isTensored)
    return con->cachedBaseOne;
  else
    return con->cachedTensorOne;
}

wali::sem_elem_t NWAOBDDRel::zero() const
{
  if(!isTensored)
    return con->cachedBaseZero;
  else
    return con->cachedTensorZero;
}

std::ostream& NWAOBDDRel::print( std::ostream& o ) const 
{
  if(!isTensored)
    o << "Base relation: ";
  else
    o << "Tensored relation: ";
  
 
  pathSummaryPrint(o);
  return o;
}

std::set<std::vector<int>> NWAOBDDRel::reducePrintOutput(std::set<std::vector<int>> vOut) const
{
	std::set<std::vector<int>> vRec = reducePrintOutputRecursive(vOut);
	return vRec;
}

std::set<std::vector<int>> NWAOBDDRel::reducePrintOutputRecursive(std::set<std::vector<int>> v) const
{
	std::set<std::vector<int>> ret;
	int pSize = v.begin()->size();
	if (pSize == 1)
	{
		std::set<std::vector<int>>::iterator i;
		bool one1 = false;
		bool zero0 = false;
		bool both = false;
		for (i = v.begin(); i != v.end(); ++i)
		{
			if (i->front() == 0)
			{
				zero0 = true;
			}
			else if (i->front() == 1)
			{
				one1 = true;
			}
			else
			{
				both = true;
			}
		}
		if (both || (zero0 && one1))
		{
			std::vector<int> v;
			v.push_back(NO_DISTINCTION);
			ret.insert(v);
		}
		else if (zero0)
		{
			std::vector<int> v;
			v.push_back(0);
			ret.insert(v);
		}
		else
		{
			assert(one1);
			std::vector<int> v;
			v.push_back(1);
			ret.insert(v);
		}
	}
	else
	{
		std::set<std::vector<int>> vOne;
		std::set<std::vector<int>> vZero;
		std::set<std::vector<int>>::iterator vIter;
		for (vIter = v.begin(); vIter != v.end(); ++vIter)
		{
			int val = vIter->back();
			std::vector<int> vectorBack(*vIter);
			vectorBack.pop_back();
			if (val == 0)
			{
				vZero.insert(vectorBack);
			}
			else if (val == 1)
			{
				vOne.insert(vectorBack);
			}
			else
			{
				vZero.insert(vectorBack);
				vOne.insert(vectorBack);
			}
		}
		
		std::set<std::vector<int>> zeroRet; 
		if (vZero.size() != 0)
		{
			zeroRet = reducePrintOutputRecursive(vZero);
		}
		std::set<std::vector<int>> oneRet;
		if (vOne.size() != 0)
		{
			oneRet = reducePrintOutputRecursive(vOne);
		}
		std::set<std::vector<int>>::iterator it;
		std::set<std::vector<int>>::iterator fIt;
		for (it = zeroRet.begin(); it != zeroRet.end(); ++it)
		{
			fIt = oneRet.find(*it);
			if (fIt != oneRet.end())
			{
				std::vector<int> tmp(*it);
				tmp.push_back(NO_DISTINCTION);
				ret.insert(tmp);
				oneRet.erase(fIt);
			}
			else
			{
				std::vector<int> tmp(*it);
				tmp.push_back(0);
				ret.insert(tmp);
			}
		}
		for (it = oneRet.begin(); it != oneRet.end(); ++it)
		{
			std::vector<int> tmp(*it);
			tmp.push_back(1);
			ret.insert(tmp);
		}
	}
	return ret;
}


std::ostream& NWAOBDDRel::pathSummaryPrint(std::ostream& o) const
{
	int exit;
	if (rel.root->rootConnection.entryPointHandle.handleContents == NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[rel.root->level].handleContents)
	{
		if (rel.root->rootConnection.returnMapHandle.Lookup(0).First())
		{
			o << "*";
		}
		else
		{
			o << "F";
		}
	}
	else
	{
		if (rel.root->rootConnection.returnMapHandle.Lookup(0).First())
		{
			exit = 0;
		}
		else
		{
			exit = 1;
		}
		std::vector<int> vOrder;
		std::set<std::vector<int>> vOut = pathSummaryPrintRecursive(rel.root->rootConnection.entryPointHandle, exit, 0);
		if (vOut.size() != 0)
		{
			std::set<std::string>::iterator it;
			std::set<std::vector<int>> vReduced = reducePrintOutput(vOut);
			std::set<std::string> vPrint = getPrintOutput(vReduced);

			for (it = vPrint.begin(); it != vPrint.end(); ++it)
			{
				o << "<" << *it << ">";
			}
		}
		return o;
	}
}

std::set<std::vector<int>> NWAOBDDRel::pathSummaryPrintRecursive(NWA_OBDD::NWAOBDDNodeHandle n, int exit, int offset) const
{
	//ETTODO insert Memoization
	return NWAOBDDRel::pathSummaryPrintRecursive((NWA_OBDD::NWAOBDDInternalNode*)n.handleContents, exit, offset);
}

std::set<std::vector<int>> NWAOBDDRel::pathSummaryPrintRecursive(NWA_OBDD::NWAOBDDInternalNode * n, int exit, int offset) const
{
	std::set<std::vector<int>>  vRet;
	if (n->level == 0)
	{
		return vRet;
	}
	else {
		unsigned int size = ((unsigned int)((((unsigned int)1) << (n->level + 2)) - (unsigned int)4));//The number of possible variables in this level of NWA_OBDD::NWAOBDD
		int numTiers = n->level / 2;
		int numTotalVars = 1;
		int vals[12];
		for (int z = numTiers - 1; z > 0; z--)
		{
			numTotalVars += pow(4, z);
		}
		int varOffset = (numTotalVars - 1) / 4;
		int vars[5];
		vars[0] = offset;
		vars[1] = offset + 1;
		vars[2] = offset + varOffset + 1;
		vars[3] = offset + 2 * varOffset + 1;
		vars[4] = offset + 3 * varOffset + 1;

		for (unsigned int i = 0; i < n->numBConnections; i++)  //For every middle node
		{
			for (int j = 0; j < 2; j++)  //Look at the 0 NWA_OBDD::Connection, then the 1 NWA_OBDD::Connection
			{
				if (n->BConnection[0][i] == n->BConnection[1][i])
				{
					j = 1;
					vals[6] = NO_DISTINCTION;
				}
				else
				{
					vals[6] = j;
				}
				ReturnMapHandle<intpair> BMap = n->BConnection[j][i].returnMapHandle; //Get the Return Map of the B NWA_OBDD::Connection
				unsigned BMapSize = BMap.mapContents->mapArray.size();
				int bExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
				for (unsigned sBI = 0; sBI < BMapSize; sBI++)
				{
					intpair bExit = BMap.mapContents->mapArray[sBI];
					bool path = false;
					if (exit == bExit.First())  //The 0 return of on of the BNWA_OBDD::Connections exits connects to the exit we are looking for
					{
						path = true;
						if (exit == bExit.Second()) //The 1 return of the B-NWAOBDD::Connection exit connects to the exit we are looking for
						{
							vals[11] = NO_DISTINCTION;
						}
						else
						{
							vals[11] = 0;
						}
					}
					else if (exit == bExit.Second()) {
						path = true;
						vals[11] = 1;
					}
					if (path)
					{
						NWA_OBDD::NWAOBDDInternalNode * tempB = (NWA_OBDD::NWAOBDDInternalNode*)n->BConnection[j][i].entryPointHandle.handleContents;
						for (unsigned int k = 0; k < tempB->numBConnections; k++) //Now look at each of the B-NWA_OBDD::Connections of the current node
						{
							for (int jB = 0; jB < 2; jB++) //look at the 0 NWA_OBDD::Connection, then the 1 NWA_OBDD::Connection
							{
								if (tempB->BConnection[0][k] == tempB->BConnection[1][k])
								{
									jB = 1;
									vals[9] = NO_DISTINCTION;
								}
								else
								{
									vals[9] = jB;
								}
								ReturnMapHandle<intpair> BBMap = tempB->BConnection[jB][k].returnMapHandle; //Get the Return Map of the B NWA_OBDD::Connection
								unsigned BBMapSize = BBMap.mapContents->mapArray.size();
								int bbExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
								for (unsigned sBBI = 0; sBBI < BBMapSize; sBBI++)
								{
									intpair bbExit = BBMap.mapContents->mapArray[sBBI];
									bool path2 = false;
									if (bExitVal == bbExit.First())  //The 0 return of on of the BNWA_OBDD::Connections exits connects to the Upper B node's exit we are looking for
									{
										path2 = true;
										if (bExitVal == bbExit.Second())
										{
											vals[10] = NO_DISTINCTION;
										}
										else
										{
											vals[10] = 0;
										}
									}
									else if (bExitVal == bbExit.Second())
									{
										path2 = true;
										vals[10] = 1;
									}
									if (path2)
									{
										std::set<std::vector<int>> v5;
										if (vars[4] < this->con->numTotalVars)
										{
											v5 = pathSummaryPrintRecursive(tempB->BConnection[jB][k].entryPointHandle, bbExitVal, 3 * varOffset + 1 + offset);
										}
										//We've found a path through a B NWA_OBDD::Connection of the Upper B tier, now we must find all paths from the entrance of the B-NWA_OBDD::Connection to
										//the start of the found B-NWA_OBDD::Connection - which is the kth midpoint
										for (int lB = 0; lB < 2; lB++) //Look at the two A-NWA_OBDD::Connections
										{
											if (tempB->AConnection[0] == tempB->AConnection[1])
											{
												lB = 1;
												vals[7] = NO_DISTINCTION;
											}
											else
											{
												vals[7] = lB;
											}
											ReturnMapHandle<intpair> BAMap = tempB->AConnection[lB].returnMapHandle;
											unsigned BAMapSize = BAMap.mapContents->mapArray.size();
											int baExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
											for (unsigned sBAI = 0; sBAI < BAMapSize; sBAI++)
											{
												intpair baExit = BAMap.mapContents->mapArray[sBAI];
												bool path3 = false;
												if (k == baExit.First()) //The 0 reutrn of an exit of the given ANWA_OBDD::Connection connects to the kth midpoint
												{
													path3 = true;
													if (k == baExit.Second())
													{
														vals[8] = NO_DISTINCTION;
													}
													else {
														vals[8] = 0;
													}
												}
												else if (k == baExit.Second())
												{
													path3 = true;
													vals[8] = 1;
												}
												if (path3)
												{
													std::set<std::vector<int>> v4;
													if (vars[3] < this->con->numTotalVars)
													{
														v4 = pathSummaryPrintRecursive(tempB->AConnection[lB].entryPointHandle, baExitVal, varOffset * 2 + 1 + offset);
													}
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-NWA_OBDD::Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for (int l = 0; l < 2; l++)
													{
														if (n->AConnection[0] == n->AConnection[1])
														{
															l = 1;
															vals[0] = NO_DISTINCTION;
														}
														else
														{
															vals[0] = l;
														}
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														int aExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															bool path4 = false;
															if (i == aExit.First()) //The 0 return of an exit of the given A NWA_OBDD::Connection connects to the ith midpoint
															{
																path4 = true;
																if (i == aExit.Second())
																{
																	vals[5] = NO_DISTINCTION;
																}
																else
																{
																	vals[5] = 0;
																}
															}
															else if (i == aExit.Second())
															{
																path4 = true;
																vals[5] = 1;
															}
															if (path4)
															{
																//Now we know an A-NWA_OBDD::Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWA_OBDD::NWAOBDDInternalNode * tempA = (NWA_OBDD::NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		if (tempA->BConnection[0][f] == tempA->BConnection[1][f])
																		{
																			jA = 1;
																			vals[3] = NO_DISTINCTION;
																		}
																		else
																		{
																			vals[3] = jA;
																		}
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		int abExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			bool path5 = false;
																			if (aExitVal == abExit.First())
																			{
																				path5 = true;
																				if (aExitVal == abExit.Second())
																				{
																					vals[4] = NO_DISTINCTION;
																				}
																				else
																				{
																					vals[4] = 0;
																				}
																			}
																			else if (aExitVal == abExit.Second())
																			{
																				path5 = true;
																				vals[4] = 0;
																			}
																			if (path5)
																			{
																				std::set<std::vector<int>> v3;
																				if (vars[2] < this->con->numTotalVars)
																				{
																					v3 = pathSummaryPrintRecursive(tempA->BConnection[jA][f].entryPointHandle, abExitVal, varOffset + 1 + offset);
																				}
																				//Now we only need to find a path through the A NWA_OBDD::Connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; lA < 2; lA++)
																				{
																					if (tempA->AConnection[0] == tempA->AConnection[1])
																					{
																						lA = 1;
																						vals[1] = NO_DISTINCTION;
																					}
																					else
																					{
																						vals[1] = lA;
																					}
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					int aaExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						bool path6 = false;
																						if (f == aaExit.First()) //The A NWA_OBDD::Connection has a 0 return onto f
																						{
																							path6 = true;
																							if (f == aaExit.Second())
																							{
																								vals[2] = NO_DISTINCTION;
																							}
																							else
																							{
																								vals[2] = 0;
																							}
																						}
																						else if (f == aaExit.Second())
																						{
																							path6 = true;
																							vals[2] = 1;
																						}
																						if (path6)
																						{
																							//Perform another recursive call
																							std::vector<int> v1 = getVariableVals(vals);
																							std::set<std::vector<int>> v2;
																							if (vars[1] < this->con->numTotalVars)
																							{
																								v2 = pathSummaryPrintRecursive(tempA->AConnection[lA].entryPointHandle, aaExitVal, 1 + offset);
																							}
																							std::set<std::vector<int>>::iterator it5, it4, it3, it2, it0, it12, it123, it1234;
																							std::set<std::vector<int>> v12, v123, v1234, v12345;
																							if (v2.size() != 0)
																							{
																								for (it2 = v2.begin(); it2 != v2.end(); it2++)
																								{
																									std::vector<int> tmp(v1);
																									tmp.insert(tmp.end(), it2->begin(), it2->end());
																									v12.insert(tmp);
																								}
																							}
																							else
																							{
																								v12.insert(v1);
																							}

																							if (v3.size() != 0)
																							{
																								for (it3 = v3.begin(); it3 != v3.end(); it3++)
																								{
																									for (it12 = v12.begin(); it12 != v12.end(); it12++)
																									{
																										std::vector<int> tmp(*it12);
																										tmp.insert(tmp.end(), it3->begin(), it3->end());
																										v123.insert(tmp);
																									}
																								}
																							}
																							else
																							{
																								v123 = v12;
																							}

																							if (v4.size() != 0)
																							{
																								for (it4 = v4.begin(); it4 != v4.end(); it4++)
																								{
																									for (it123 = v123.begin(); it123 != v123.end(); it123++)
																									{
																										std::vector<int> tmp(*it123);
																										tmp.insert(tmp.end(), it4->begin(), it4->end());
																										v1234.insert(tmp);
																									}
																								}
																							}
																							else
																							{
																								v1234 = v123;
																							}

																							if (v5.size() != 0)
																							{
																								for (it5 = v5.begin(); it5 != v5.end(); it5++)
																								{
																									for (it1234 = v1234.begin(); it1234 != v1234.end(); it1234++)
																									{
																										std::vector<int> tmp(*it1234);
																										tmp.insert(tmp.end(), it5->begin(), it5->end());
																										v12345.insert(tmp);
																									}
																								}
																							}
																							else
																							{
																								v12345 = v1234;
																							}

																							for (it0 = v12345.begin(); it0 != v12345.end(); it0++)
																							{
																								vRet.insert(*it0);
																							}

																						}
																					//Iterate to the next return on the map and update the exit number
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A NWA_OBDD::Connection
																			}
																			//increment the iterator
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A NWA_OBDD::Connections middle nodes
															}
															aExitVal++;
														}
													}
												}
												baExitVal++;
											}
										}
									}
									bbExitVal++;
								}
							}
						}
						//Now look at the ANWA_OBDD::Connections and see which ones have exits into given BNWA_OBDD::Connection node
					}
					//The BNWA_OBDD::Connection's 1 return connects to the given exit, so repeat the above with the adjusted value
					bExitVal++;
				}
			}
		}
		return vRet;
	}
}

std::vector<int> NWAOBDDRel::getVariableVals(int vals[12]) const
{
	std::vector<int> ret;
	switch (con->order)
	{
	case BASE_1ST_TENSOR_ROOT:
		if (!isTensored)
		{
			ret.push_back(vals[1]);
			ret.push_back(vals[2]);
			ret.push_back(vals[3]);
		}
		else
		{
			ret.push_back(vals[1]);
			ret.push_back(vals[2]);
			ret.push_back(vals[3]);
			ret.push_back(vals[0]);
			ret.push_back(vals[5]);
			ret.push_back(vals[11]);
		}
		break;
	case SEPERATE_LEVELS:
		if (!isTensored)
		{
			ret.push_back(vals[1]);
			ret.push_back(vals[2]);
			ret.push_back(vals[3]);
		}
		else
		{
			ret.push_back(vals[1]);
			ret.push_back(vals[2]);
			ret.push_back(vals[3]);
			ret.push_back(vals[7]);
			ret.push_back(vals[8]);
			ret.push_back(vals[9]);
		}
		break;
	case AFFINITY:
		if (!isTensored)
		{
			ret.push_back(vals[5]);
			ret.push_back(vals[6]);
			ret.push_back(vals[11]);
		}
		else
		{
			ret.push_back(vals[5]);
			ret.push_back(vals[6]);
			ret.push_back(vals[11]);
			ret.push_back(vals[2]);
			ret.push_back(vals[0]);
			ret.push_back(vals[1]);
		}
	break;
	}
	return ret;
}

std::set<std::string> NWAOBDDRel::getPrintOutput(std::set<std::vector<int>> output) const
{
	std::set<std::string> ret;
	std::set<std::vector<int>>::iterator it;
	for (it = output.begin(); it != output.end(); ++it)
	{
		std::string s = convertValsToString(*it);
		ret.insert(s);
	}
	return ret;
}

std::string NWAOBDDRel::convertValsToString(std::vector<int> vals) const
{
	std::string s;
	std::string name = "vName";
	bool visited = false;
	for (int i = 0; i < this->con->numTotalVars; i++)
	{
		for (std::map<std::string, int>::const_iterator ci = con->varMap.begin(); ci != con->varMap.end(); ++ci)
		{
			if (i == (*ci).second)
			{
				name = (*ci).first;
				if (!isTensored)
				{
					if (vals[3 * i] == NO_DISTINCTION)
					{
						if (vals[(3 * i) + 1] == NO_DISTINCTION)
						{
							if (vals[(3 * i) + 2] != NO_DISTINCTION)
							{
								if (visited)
								{
									s.append(", ");
								}
								else
								{
									visited = true;
								}
								s.append(name);
								s.append("_e:");
								s.append(std::to_string(vals[(3 * i) + 2]));
							}
						}
						else
						{
							if (visited)
							{
								s.append(", ");
							}
							else
							{
								visited = true;
							}
							s.append(name);
							s.append("\':");
							s.append(std::to_string(vals[(3 * i) + 1]));
							if (vals[(3 * i) + 2] != NO_DISTINCTION)
							{
								if (visited)
								{
									s.append(", ");
								}
								else
								{
									visited = true;
								}
								s.append(name);
								s.append("_e:");
								s.append(std::to_string(vals[(3 * i) + 2]));
							}
						}
					}
					else
					{
						if (visited)
						{
							s.append(", ");
						}
						else
						{
							visited = true;
						}
						s.append(name);
						s.append(":");
						s.append(std::to_string(vals[(3 * i)]));
						if (vals[(3 * i) + 1] == NO_DISTINCTION)
						{
							if (vals[(3 * i) + 2] != NO_DISTINCTION)
							{
								if (visited)
								{
									s.append(", ");
								}
								else
								{
									visited = true;
								}
								s.append(name);
								s.append("_e:");
								s.append(std::to_string(vals[(3 * i) + 2]));
							}
						}
						else
						{
							if (visited)
							{
								s.append(", ");
							}
							else
							{
								visited = true;
							}
							s.append(name);
							s.append("\':");
							s.append(std::to_string(vals[(3 * i) + 1]));
							if (vals[(3 * i) + 2] != NO_DISTINCTION)
							{
								if (visited)
								{
									s.append(", ");
								}
								else
								{
									visited = true;
								}
								s.append(name);
								s.append("_e:");
								s.append(std::to_string(vals[(3 * i) + 2]));
							}
						}

					}
				}
				else
				{
					if (vals[(6 * i)] == NO_DISTINCTION)
					{
						if (vals[(6 * i) + 1] == NO_DISTINCTION)
						{
							if (vals[(6 * i) + 2] == NO_DISTINCTION)
							{
								if (vals[(6 * i) + 3] == NO_DISTINCTION)
								{
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
								else
								{
									if (visited)
									{
										s.append(", ");
									}
									else
									{
										visited = true;
									}
									s.append(name);
									s.append("_t2:");
									s.append(std::to_string(vals[(6 * i) + 3]));
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
							}
							else
							{
								if (visited)
								{
									s.append(", ");
								}
								else
								{
									visited = true;
								}
								s.append(name);
								s.append("_t1_e:");
								s.append(std::to_string(vals[(6 * i) + 2]));
								if (vals[(6 * i) + 3] == NO_DISTINCTION)
								{
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
								else
								{
									if (visited)
									{
										s.append(", ");
									}
									else
									{
										visited = true;
									}
									s.append(name);
									s.append("_t2:");
									s.append(std::to_string(vals[(6 * i) + 3]));
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
							}
						}
						else
						{
							if (visited)
							{
								s.append(", ");
							}
							else
							{
								visited = true;
							}
							s.append(name);
							s.append("_t1\':");
							s.append(std::to_string(vals[(6 * i) + 1]));
							if (vals[(6 * i) + 2] == NO_DISTINCTION)
							{
								if (vals[(6 * i) + 3] == NO_DISTINCTION)
								{
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
								else
								{
									if (visited)
									{
										s.append(", ");
									}
									else
									{
										visited = true;
									}
									s.append(name);
									s.append("_t2:");
									s.append(std::to_string(vals[(6 * i) + 3]));
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
							}
							else
							{
								if (visited)
								{
									s.append(", ");
								}
								else
								{
									visited = true;
								}
								s.append(name);
								s.append("_t1_e:");
								s.append(std::to_string(vals[(6 * i) + 2]));
								if (vals[(6 * i) + 3] == NO_DISTINCTION)
								{
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
								else
								{
									if (visited)
									{
										s.append(", ");
									}
									else
									{
										visited = true;
									}
									s.append(name);
									s.append("_t2:");
									s.append(std::to_string(vals[(6 * i) + 3]));
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
							}
						}
					}
					else
					{
						if (visited)
						{
							s.append(", ");
						}
						else
						{
							visited = true;
						}
						s.append(name);
						s.append("_t1:");
						s.append(std::to_string(vals[(6 * i)]));
						if (vals[(6 * i) + 1] == NO_DISTINCTION)
						{
							if (vals[(6 * i) + 2] == NO_DISTINCTION)
							{
								if (vals[(6 * i) + 3] == NO_DISTINCTION)
								{
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
								else
								{
									if (visited)
									{
										s.append(", ");
									}
									else
									{
										visited = true;
									}
									s.append(name);
									s.append("_t2:");
									s.append(std::to_string(vals[(6 * i) + 3]));
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
							}
							else
							{
								if (visited)
								{
									s.append(", ");
								}
								else
								{
									visited = true;
								}
								s.append(name);
								s.append("_t1_e:");
								s.append(std::to_string(vals[(6 * i) + 2]));
								if (vals[(6 * i) + 3] == NO_DISTINCTION)
								{
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
								else
								{
									if (visited)
									{
										s.append(", ");
									}
									else
									{
										visited = true;
									}
									s.append(name);
									s.append("_t2:");
									s.append(std::to_string(vals[(6 * i) + 3]));
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
							}
						}
						else
						{
							if (visited)
							{
								s.append(", ");
							}
							else
							{
								visited = true;
							}
							s.append(name);
							s.append("_t1\':");
							s.append(std::to_string(vals[(6 * i) + 1]));
							if (vals[(6 * i) + 2] == NO_DISTINCTION)
							{
								if (vals[(6 * i) + 3] == NO_DISTINCTION)
								{
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
								else
								{
									if (visited)
									{
										s.append(", ");
									}
									else
									{
										visited = true;
									}
									s.append(name);
									s.append("_t2:");
									s.append(std::to_string(vals[(6 * i) + 3]));
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
							}
							else
							{
								if (visited)
								{
									s.append(", ");
								}
								else
								{
									visited = true;
								}
								s.append(name);
								s.append("_t1_e:");
								s.append(std::to_string(vals[(6 * i) + 2]));
								if (vals[(6 * i) + 3] == NO_DISTINCTION)
								{
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
								else
								{
									if (visited)
									{
										s.append(", ");
									}
									else
									{
										visited = true;
									}
									s.append(name);
									s.append("_t2:");
									s.append(std::to_string(vals[(6 * i) + 3]));
									if (vals[(6 * i) + 4] == NO_DISTINCTION)
									{
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											if (visited)
											{
												s.append(", ");
											}
											else
											{
												visited = true;
											}
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));

										}
									}
									else
									{
										if (visited)
										{
											s.append(", ");
										}
										else
										{
											visited = true;
										}
										s.append(name);
										s.append("_t2\':");
										s.append(std::to_string(vals[(6 * i) + 4]));
										if (vals[(6 * i) + 5] != NO_DISTINCTION)
										{
											s.append(", ");
											s.append(name);
											s.append("_t2_e:");
											s.append(std::to_string(vals[(6 * i) + 5]));
										}
									}
								}
							}
						}
					}
				}
			}
		}

	}
	return s;
}
wali::sem_elem_tensor_t NWAOBDDRel::transpose() 
{
  return Transpose();
}

wali::sem_elem_tensor_t NWAOBDDRel::tensor(wali::SemElemTensor* se)
{
  nwaobdd_t that( convert(se) );
  return Kronecker(that);
}

wali::sem_elem_tensor_t NWAOBDDRel::detensor()
{
	NWA_OBDD::NWAOBDD g;
	int DetensorMap[12];
	CreateDetensorMap(DetensorMap);
	nwaobdd_t temp = this->SetEqualVocabs(BASE_POST, TENSOR_PRE); //ETTODO:Check this
	g = AdjustedPathSummary(temp->rel, DetensorMap);
	return new NWAOBDDRel(con, g, false);
}

wali::sem_elem_tensor_t NWAOBDDRel::detensorTranspose()
{
	NWA_OBDD::NWAOBDD g;
	nwaobdd_t temp = this->SetEqualVocabs(BASE_PRE, TENSOR_PRE);// con->getDetensorId();;
	int DetensorTransMap[12];
	CreateDetensorTransposeMap(DetensorTransMap);
	g = AdjustedPathSummary(temp->rel, DetensorTransMap);
	return new NWAOBDDRel(con, g, false);
}

nwaobdd_t NWAOBDDRel::MoveBaseToTensor()
{
	NWA_OBDD::NWAOBDD g;
	int MoveBaseToTensorMap[12];
	CreateMoveBaseToTensorMap(MoveBaseToTensorMap);
	g = AdjustedPathSummary(this->rel, MoveBaseToTensorMap);
	return new NWAOBDDRel(con,g,this->isTensored);
}

/*Performs existential quantifications on a list of vocabularies */
nwaobdd_t NWAOBDDRel::ExistentialQuantification(vocabulary_t vocabs[], int length)
{
	NWA_OBDD::NWAOBDD g;
	int ExistentialMap[12];
	CreateExistentialMap(vocabs, length, ExistentialMap);
	g = AdjustedPathSummary(this->rel, ExistentialMap);
	return new NWAOBDDRel(con,g,this->isTensored);
}


nwaobdd_t NWAOBDDRel::SetEqualVocabs(vocabulary_t t1, vocabulary_t t2)
{
	NWA_OBDD::NWAOBDD g;
	g = NWA_OBDD::MkTrue();
	int level = con->maxLevel;
	int v1 = con->getVLoc(t1);
	int v2 = con->getVLoc(t2);
	for (int j = 0; j < con->numTotalVars; j++) //This is a very innefficient way of doing things ETTODO - FIX!
	{
		int p1 = con->findVarLoc(j, 0, level, v1);
		int p2 = con->findVarLoc(j, 0, level, v2);
		g = NWA_OBDD::MkAnd(g, NWA_OBDD::MkOr(NWA_OBDD::MkAnd(NWA_OBDD::MkDistinction(p1), NWA_OBDD::MkDistinction(p2)), NWA_OBDD::MkNot(NWA_OBDD::MkOr(NWA_OBDD::MkDistinction(p1), NWA_OBDD::MkDistinction(p2)))));
	}
	nwaobdd_t tmp = new NWAOBDDRel(con, g, this->isTensored);
	NWA_OBDD::NWAOBDD result = NWA_OBDD::MkAnd(this->rel, g);
	return new NWAOBDDRel(con, result, this->isTensored);
	//For each tier
	//Get the locations of the two variables
	//ret = ret & (this & v1.1, v2.1 || this & v1.0, v2.0)
	//return ret
}

/*Creates the map needed for AdjustedPathSummary to perform a transpose operation */
void NWAOBDDRel::CreateTransposeMap(int * transposeMap) const
{
	for (int i = 0; i < 12; i++)
	{
		transposeMap[i] = i;
	}
	int v1Loc = con->getBasePreLoc();
	int v2Loc = con->getBasePostLoc();
	transposeMap[v1Loc] = v2Loc;
	transposeMap[v2Loc] = v1Loc;
}

void NWAOBDDRel::CreateMoveBasePreToExtraMap(int * moveBasePreToExtraMap) const
{
	for (int i = 0; i < 12; i++)
	{
		moveBasePreToExtraMap[i] = i;
	}
	int v1Loc = con->getBasePreLoc();
	int v2Loc = con->getBaseExtraLoc();
	moveBasePreToExtraMap[v1Loc] = NO_DISTINCTION;
	moveBasePreToExtraMap[v2Loc] = v1Loc;
}

void NWAOBDDRel::CreateCopyBasePostToExtraMap(int * copyBasePostToExtraMap) const
{
	for (int i = 0; i < 12; i++)
	{
		copyBasePostToExtraMap[i] = i;
	}
	int v1Loc = con->getBasePostLoc();
	int v2Loc = con->getBaseExtraLoc();
	copyBasePostToExtraMap[v2Loc] = v1Loc;
}

void NWAOBDDRel::CreateMoveBasePostToExtraMap(int * moveBasePostToExtraMap) const
{
	for (int i = 0; i < 12; i++)
	{
		moveBasePostToExtraMap[i] = i;
	}
	int v1Loc = con->getBasePostLoc();
	int v2Loc = con->getBaseExtraLoc();
	moveBasePostToExtraMap[v1Loc] = NO_DISTINCTION;
	moveBasePostToExtraMap[v2Loc] = v1Loc;
}

void NWAOBDDRel::CreateMoveBaseExtraToPostMap(int * moveBaseExtraToPostMap) const
{
	for (int i = 0; i < 12; i++)
	{
		moveBaseExtraToPostMap[i] = i;
	}
	int v1Loc = con->getBasePostLoc();
	int v2Loc = con->getBaseExtraLoc();
	moveBaseExtraToPostMap[v2Loc] = NO_DISTINCTION;
	moveBaseExtraToPostMap[v1Loc] = v2Loc;
}

/*Creates the map needed for AdjustedPathSummary to perform a detensor operation */
void NWAOBDDRel::CreateDetensorMap(int * detensorMap)
{
	for (int i = 0; i < 12; i++)
	{
		detensorMap[i] = 12;
	}
	int basePreLoc = con->getBasePreLoc();
	int basePostLoc = con->getBasePostLoc();
	int tensorPostLoc = con->getTensorPostLoc();
	detensorMap[basePreLoc] = basePreLoc;
	detensorMap[basePostLoc] = tensorPostLoc;
}

/*Create a DetensorTransposeMap */
void NWAOBDDRel::CreateDetensorTransposeMap(int * detensorTransposeMap)
{
	for (int i = 0; i < 12; i++)
	{
		detensorTransposeMap[i] = 12;
	}
	int basePreLoc = con->getBasePreLoc();
	int basePostLoc = con->getBasePostLoc();
	int tensorPostLoc = con->getTensorPostLoc();
	detensorTransposeMap[basePreLoc] = basePostLoc;
	detensorTransposeMap[basePostLoc] = tensorPostLoc;
}

/*Create a map to move values in the base vocabularies into the tensored vocabulary*/
void NWAOBDDRel::CreateMoveBaseToTensorMap(int * moveBaseToTensorMap) const
{
	for (int i = 0; i < 12; i++)
	{
		moveBaseToTensorMap[i] = i;
	}
	int basePreLoc = con->getBasePreLoc();
	int basePostLoc = con->getBasePostLoc();
	int tensorPreLoc = con->getTensorPreLoc();
	int tensorPostLoc = con->getTensorPostLoc();
	moveBaseToTensorMap[tensorPreLoc] = basePreLoc;
	moveBaseToTensorMap[tensorPostLoc] = basePostLoc;
	moveBaseToTensorMap[basePreLoc] = NO_DISTINCTION;
	moveBaseToTensorMap[basePostLoc] = NO_DISTINCTION;
}

void NWAOBDDRel::CreateBaseRightShiftMap(int * baseRightShiftMap) const 
{
	for (int i = 0; i < 12; i++)
	{
		baseRightShiftMap[i] = i;
	}
	int basePreLoc = con->getBasePreLoc();
	int basePostLoc = con->getBasePostLoc();
	int baseExtraLoc = con->getBaseExtraLoc();

	baseRightShiftMap[basePreLoc] = NO_DISTINCTION;
	baseRightShiftMap[basePostLoc] = basePreLoc;
	baseRightShiftMap[baseExtraLoc] = basePostLoc;
}

void NWAOBDDRel::CreateTensorRightShiftMap(int * tensorRightShiftMap) const
{
	for (int i = 0; i < 12; i++)
	{
		tensorRightShiftMap[i] = 12;
	}
	int basePreLoc = con->getBasePreLoc();
	int basePostLoc = con->getBasePostLoc();
	int baseExtraLoc = con->getBaseExtraLoc();

	int tensorPreLoc = con->getTensorPreLoc();
	int tensorPostLoc = con->getTensorPostLoc();
	int tensorExtraLoc = con->getTensorExtraLoc();

	tensorRightShiftMap[basePostLoc] = basePreLoc;
	tensorRightShiftMap[baseExtraLoc] = basePostLoc;

	tensorRightShiftMap[tensorPostLoc] = tensorPreLoc;
	tensorRightShiftMap[tensorExtraLoc] = tensorPostLoc;
}

void NWAOBDDRel::CreateBaseRestoreMap(int * baseRestoreMap) const
{
	for (int i = 0; i < 12; i++)
	{
		baseRestoreMap[i] = i;
	}
	int basePostLoc = con->getBasePostLoc();
	int baseExtraLoc = con->getBaseExtraLoc();

	baseRestoreMap[basePostLoc] = baseExtraLoc;
	baseRestoreMap[baseExtraLoc] = NO_DISTINCTION; 
}

void NWAOBDDRel::CreateTensorRestoreMap(int * tensorRestoreMap) const
{
	for (int i = 0; i < 12; i++)
	{
		tensorRestoreMap[i] = i;
	}
	int basePostLoc = con->getBasePostLoc();
	int baseExtraLoc = con->getBaseExtraLoc();

	int tensorPostLoc = con->getTensorPostLoc();
	int tensorExtraLoc = con->getTensorExtraLoc();

	tensorRestoreMap[basePostLoc] = baseExtraLoc;
	tensorRestoreMap[tensorPostLoc] = tensorExtraLoc;
	tensorRestoreMap[baseExtraLoc] = NO_DISTINCTION; 
	tensorRestoreMap[tensorExtraLoc] = NO_DISTINCTION;
}

/*Creates a map for existential quantification*/
void NWAOBDDRel::CreateExistentialMap(vocabulary_t vocabs[], int length, int * newMap)
{
	for (int i = 0; i < 12; i++)
	{
		newMap[i] = i;
	}

	for (int i = 0; i < length; i++)
	{
		int cLoc = con->getVLoc(vocabs[i]);
		newMap[cLoc] = NO_DISTINCTION;
	}
}

//***************************************************************
// ModPathSummaryKey
//***************************************************************

// Constructor
ModPathSummaryKey::ModPathSummaryKey(NWA_OBDD::NWAOBDDNodeHandle nodeHandle1, int exit, const int map[12])
	: nodeHandle1(nodeHandle1), exit(exit)
{
	for (int i = 0; i < 12; i++)
	{
		this->map[i] = map[i];
	}
}

// Hash
unsigned int ModPathSummaryKey::Hash(unsigned int modsize)
{
	unsigned int hvalue = 0;
	hvalue = (997 * nodeHandle1.Hash(modsize) + exit) % modsize;
	return hvalue;
}

// Print
void ModPathSummaryKey::Print(std::ostream & out) const
{
	out << "(" << nodeHandle1 << ", " << exit << ", " << map << ")";
	return;
}

std::ostream& operator<< (std::ostream & out, const ModPathSummaryKey &p)
{
	p.Print(out);
	return(out);
}


ModPathSummaryKey& ModPathSummaryKey::operator= (const ModPathSummaryKey& i)
{
	if (this != &i)      // don't assign to self!
	{
		nodeHandle1 = i.nodeHandle1;
		exit = i.exit;
		for (int j = 0; j < 12; j++)
		{
			map[j] = i.map[j];
		}
	}
	return *this;
}

// Overloaded !=
bool ModPathSummaryKey::operator!=(const ModPathSummaryKey& p)
{
	return (exit != p.exit) || (nodeHandle1 != p.nodeHandle1)  || (map != p.map);
}

// Overloaded ==
bool ModPathSummaryKey::operator==(const ModPathSummaryKey& p)
{
	return (exit == p.exit) && (nodeHandle1 == p.nodeHandle1) && (map == p.map);
}


//***************************************************************
// ModPathSummaryMemo
//***************************************************************

// Default constructor
ModPathSummaryMemo::ModPathSummaryMemo()
	: result(NWA_OBDD::NWAOBDDNodeHandle())
{
}

// Constructor
ModPathSummaryMemo::ModPathSummaryMemo(NWA_OBDD::NWAOBDDNodeHandle result, ReturnMapHandle<intpair> returnMap)
	: result(result), returnMap(returnMap)
{
}

std::ostream& operator<< (std::ostream & out, const ModPathSummaryMemo &p)
{
	out << "(" << p.result << ", " << p.returnMap << ")";
	return(out);
}

ModPathSummaryMemo& ModPathSummaryMemo::operator= (const ModPathSummaryMemo& i)
{
	if (this != &i)      // don't assign to self!
	{
		result = i.result;
		returnMap = i.returnMap;
	}
	return *this;
}

// Overloaded !=
bool ModPathSummaryMemo::operator!=(const ModPathSummaryMemo& p)
{
	return (result != p.result) || (returnMap == p.returnMap);
}

// Overloaded ==
bool ModPathSummaryMemo::operator==(const ModPathSummaryMemo& p)
{
	return (result == p.result) && (returnMap == p.returnMap);
}

static Hashtable<ModPathSummaryKey, ModPathSummaryMemo> *modPathSummaryCache = NULL;
	
/* The nonrecursive entry to the AdjustedPathSummary function */
NWA_OBDD::NWAOBDD NWAOBDDRel::AdjustedPathSummary(NWA_OBDD::NWAOBDD n, const int * map) const
{
	int exit;
	if ((n == NWA_OBDD::MkTrue()) || (n == NWA_OBDD::MkFalse()))
	{
		return n;
	}
	else
	{
		if (n.root->rootConnection.returnMapHandle.Lookup(0).First())
		{
			exit = 0;
		}
		else
		{
			exit = 1;
		}
		//NWA_OBDD::NWAOBDDTopNodeRefPtr v = AdjustedPathSummaryRecursive(n.root->rootConnection.entryPointHandle, exit, map, 0);
		NWA_OBDD::NWAOBDDBaseNodeRefPtr g = ModularAdjustedPathSummaryRecursive(n.root->rootConnection.entryPointHandle, exit, map);
		NWA_OBDD::NWAOBDDTopNodeRefPtr v;
		v = new NWA_OBDD::NWAOBDDTopNode(NWA_OBDD::NWAOBDDNodeHandle(g->rootConnection.entryPointHandle.handleContents), g->rootConnection.returnMapHandle);
		return v;
	}
}

NWA_OBDD::NWAOBDDTopNodeRefPtr NWAOBDDRel::GetWeight(const int map[12], int vals[12], int vLocs[12], int c) const
{
	NWA_OBDD::NWAOBDDTopNodeRefPtr n;
	if (map[c] == NO_DISTINCTION)
	{
		n = NWA_OBDD::MkTrueTop();
	}
	else
	{
		int pos = map[c];
		if (vals[pos] == NO_DISTINCTION)
		{
			n = NWA_OBDD::MkTrueTop();
		}
		else if (vals[pos])
		{
			n = NWA_OBDD::MkDistinction(vLocs[c]);
		} else {
			n = NWA_OBDD::MkNot(NWA_OBDD::MkDistinction(vLocs[c]));
		}
	}
	return n;
}

/*
*  This is a way getting or solving the weighted path problem specifcally for nwaobdds assuming the two
*  tiered distribution of variables.
*/
NWA_OBDD::NWAOBDDTopNodeRefPtr NWAOBDDRel::AdjustedPathSummaryRecursive(NWA_OBDD::NWAOBDDInternalNode * n, int exit, const int map[12], int offset) const
{
	if (n == NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[n->level].handleContents)
	{
		return NWA_OBDD::MkTrueTop();
	}
	
	NWA_OBDD::NWAOBDDTopNodeRefPtr current = NWA_OBDD::MkFalseTop();  //f stores the new NWA_OBDD::NWAOBDD as we build it up, starts as false as it will be a product of Ors  
	if (n->level >= 1)
	{
		int vals[12];  //An array to hold the values of the variables on the incoming and outgoing ANWA_OBDD::Connection and BNWA_OBDD::Connection edges
		int vLocs[12]; //An array to hold the values of the actual location of the variables in the larger NWA_OBDD::NWAOBDD
		unsigned int size = ((unsigned int)((((unsigned int)1) << (n->level + 2)) - (unsigned int)4));//The number of variables in this level of NWA_OBDD::NWAOBDD
		unsigned int size2 = (size - 4)/2; //The size of upper level in the tier
		//The locations in the overall NWA_OBDD::NWAOBDD of the variables in this tier

		//The locations of all the variables in relation to the overall NWA_OBDD::NWAOBDD structure
		vLocs[0] = offset;
		vLocs[1] = offset+1;
		vLocs[2] = offset+(size2/2);
		vLocs[3] = offset+1+(size2/2);
		vLocs[4] = offset-2 + (size/2);
		vLocs[5] = offset-1 + (size/2);
		vLocs[6] = offset + (size/2);
		vLocs[7] = offset + 1 + (size/2);
		vLocs[8] = offset+(size/2) +(size2/2);
		vLocs[9] = offset+1+(size/2) + (size2/2);
		vLocs[10] = offset + size - 2;
		vLocs[11] = offset + size - 1;
		
		//The goal is to find a path from the entry of the node to the given exit of the node
		//First:  Find all the B-NWA_OBDD::Connections of this node which have returns to the given exit

		//Iterate through the B-NWA_OBDD::Connections - For every middle node, there are two B-NWA_OBDD::Connections 
		for (unsigned int i = 0; i < n->numBConnections; i++)  //For every middle node
		{
			for (int j = 0; j < 2; j++)  //Look at the 0 NWA_OBDD::Connection, then the 1 NWA_OBDD::Connection
			{
				if (n->BConnection[0][i] == n->BConnection[1][i])
				{
					j = 1;
					vals[6] = NO_DISTINCTION;
				}
				else
				{
					vals[6] = j;
				}
				ReturnMapHandle<intpair> BMap = n->BConnection[j][i].returnMapHandle; //Get the Return Map of the B NWA_OBDD::Connection
				unsigned BMapSize = BMap.mapContents->mapArray.size();
				int bExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
				for (unsigned sBI = 0; sBI < BMapSize; sBI++)
				{
					intpair bExit = BMap.mapContents->mapArray[sBI];
					bool path = false;
					if (exit == bExit.First())  //The 0 return of on of the BNWA_OBDD::Connections exits connects to the exit we are looking for
					{
						path = true;
						if (exit == bExit.Second()) //The 1 return of the B-NWAOBDD::Connection exit connects to the exit we are looking for
						{
							vals[11] = NO_DISTINCTION;
						}
						else
						{
							vals[11] = 0;
						}
					}
					else if (exit == bExit.Second()) {
						path = true;
						vals[11] = 1;
					}
					if (path)
					{
						NWA_OBDD::NWAOBDDInternalNode * tempB = (NWA_OBDD::NWAOBDDInternalNode*)n->BConnection[j][i].entryPointHandle.handleContents;
						for (unsigned int k = 0; k < tempB->numBConnections; k++) //Now look at each of the B-NWA_OBDD::Connections of the current node
						{
							for (int jB = 0; jB < 2; jB++) //look at the 0 NWA_OBDD::Connection, then the 1 NWA_OBDD::Connection
							{
								if (tempB->BConnection[0][k] == tempB->BConnection[1][k])
								{
									jB = 1;
									vals[9] = NO_DISTINCTION;
								}
								else
								{
									vals[9] = jB;
								}
								ReturnMapHandle<intpair> BBMap = tempB->BConnection[jB][k].returnMapHandle; //Get the Return Map of the B NWA_OBDD::Connection
								unsigned BBMapSize = BBMap.mapContents->mapArray.size();
								int bbExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
								for (unsigned sBBI = 0; sBBI < BBMapSize; sBBI++)
								{
									intpair bbExit = BBMap.mapContents->mapArray[sBBI];
									bool path2 = false;
									if (bExitVal == bbExit.First())  //The 0 return of on of the BNWA_OBDD::Connections exits connects to the Upper B node's exit we are looking for
									{
										path2 = true;
										if (bExitVal == bbExit.Second())
										{
											vals[10] = NO_DISTINCTION;
										}
										else
										{
											vals[10] = 0;
										}
									}
									else if (bExitVal == bbExit.Second())
									{
										path2 = true;
										vals[10] = 1;
									}
									if (path2)
									{
										//We have enough information to do a recursive call on the B-NWA_OBDD::Connection of this node
										NWA_OBDD::NWAOBDDTopNodeRefPtr BBVal = NWAOBDDRel::AdjustedPathSummaryRecursive(tempB->BConnection[jB][k].entryPointHandle,bbExitVal,map,vLocs[9]+1);
										//We've found a path through a B NWA_OBDD::Connection of the Upper B tier, now we must find all paths from the entrance of the B-NWA_OBDD::Connection to
										//the start of the found B-NWA_OBDD::Connection - which is the kth midpoint
										for (int lB = 0; lB < 2; lB++) //Look at the two A-NWA_OBDD::Connections
										{
											if (tempB->AConnection[0] == tempB->AConnection[1])
											{
												lB = 1;
												vals[7] = NO_DISTINCTION;
											}
											else
											{
												vals[7] = lB;
											}
											ReturnMapHandle<intpair> BAMap = tempB->AConnection[lB].returnMapHandle;
											unsigned BAMapSize = BAMap.mapContents->mapArray.size();
											int baExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
											for (unsigned sBAI = 0; sBAI < BAMapSize; sBAI++)
											{
												intpair baExit = BAMap.mapContents->mapArray[sBAI];
												bool path3 = false;
												if (k == baExit.First()) //The 0 reutrn of an exit of the given ANWA_OBDD::Connection connects to the kth midpoint
												{
													path3 = true;
													if (k == baExit.Second())
													{
														vals[8] = NO_DISTINCTION;
													}
													else {
														vals[8] = 0;
													}
												}
												else if (k == baExit.Second())
												{
													path3 = true;
													vals[8] = 1;
												}
												if (path3)
												{
													//This gives us enough information to make the next recursive call on the ANWA_OBDD::Connection of the upper B Node
													NWA_OBDD::NWAOBDDTopNodeRefPtr BAVal = NWAOBDDRel::AdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle,baExitVal,map,vLocs[7]+1);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-NWA_OBDD::Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for(int l = 0; l < 2; l++)
													{
														if (n->AConnection[0] == n->AConnection[1])
														{
															l = 1;
															vals[0] = NO_DISTINCTION;
														}
														else
														{
															vals[0] = l;
														}
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														int aExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															bool path4 = false;
															if (i == aExit.First()) //The 0 return of an exit of the given A NWA_OBDD::Connection connects to the ith midpoint
															{
																path4 = true;
																if (i == aExit.Second())
																{
																	vals[5] = NO_DISTINCTION;
																}
																else
																{
																	vals[5] = 0;
																}
															}
															else if (i == aExit.Second())
															{
																path4 = true;
																vals[5] = 1;
															}
															if (path4)
															{
																//Now we know an A-NWA_OBDD::Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWA_OBDD::NWAOBDDInternalNode * tempA = (NWA_OBDD::NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		if (tempA->BConnection[0][f] == tempA->BConnection[1][f])
																		{
																			jA = 1;
																			vals[3] = NO_DISTINCTION;
																		}
																		else
																		{
																			vals[3] = jA;
																		}
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		int abExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			bool path5 = false;
																			if (aExitVal == abExit.First())
																			{
																				path5 = true;
																				if (aExitVal == abExit.Second())
																				{
																					vals[4] = NO_DISTINCTION;
																				}
																				else
																				{
																					vals[4] = 0;
																				}
																			}
																			else if (aExitVal == abExit.Second())
																			{
																				path5 = true;
																				vals[4] = 0;
																			}
																			if (path5)
																			{
																				NWA_OBDD::NWAOBDDTopNodeRefPtr ABVal = NWAOBDDRel::AdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle,abExitVal,map, vLocs[3]+1);
																				//Now we only need to find a path through the A NWA_OBDD::Connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; lA < 2; lA++)
																				{
																					if (tempA->AConnection[0] == tempA->AConnection[1])
																					{
																						lA = 1;
																						vals[1] = NO_DISTINCTION;
																					}
																					else
																					{
																						vals[1] = lA;
																					}
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					int aaExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						bool path6 = false;
																						if (f == aaExit.First()) //The A NWA_OBDD::Connection has a 0 return onto f
																						{
																							path6 = true;
																							if (f == aaExit.Second())
																							{
																								vals[2] = NO_DISTINCTION;
																							}
																							else
																							{
																								vals[2] = 0;
																							}
																						}
																						else if (f == aaExit.Second())
																						{
																							path6 = true;
																							vals[2] = 1;
																						}
																						if (path6)
																						{
																							//Perform another recursive call
																							NWA_OBDD::NWAOBDDTopNodeRefPtr AAVal = NWAOBDDRel::AdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle,aaExitVal,map,vLocs[1]+1);
																							//We have all four weights so now we can construct the whole path
																							NWA_OBDD::NWAOBDDTopNodeRefPtr AEnt = GetWeight(map,vals,vLocs,0);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr AEntA = GetWeight(map,vals,vLocs,1);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr AExitA = GetWeight(map,vals,vLocs,2);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr AEntB = GetWeight(map,vals,vLocs,3);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr AExitB = GetWeight(map,vals,vLocs,4);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr AExit = GetWeight(map,vals,vLocs,5);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr BEnt = GetWeight(map,vals,vLocs,6);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr BEntA = GetWeight(map,vals,vLocs,7);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr BExitA = GetWeight(map,vals,vLocs,8);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr BEntB = GetWeight(map,vals,vLocs,9);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr BExitB = GetWeight(map,vals,vLocs,10);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr BExit = GetWeight(map,vals,vLocs,11);
																							NWA_OBDD::NWAOBDDTopNodeRefPtr path = MkAnd(16,AEnt,AEntA,AAVal,AExitA,AEntB,ABVal,AExitB,AExit,BEnt,BEntA,BAVal,BExitA,BEntB,BBVal,BExitB,BExit);
																							current = MkOr(path,current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A NWA_OBDD::Connection
																			}
																			//increment the iterator
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A NWA_OBDD::Connections middle nodes
															}
															aExitVal++;
														}
													}
												}
												baExitVal++;
											}
										}
									}
									bbExitVal++;
								}
							}
						}
							//Now look at the ANWA_OBDD::Connections and see which ones have exits into given BNWA_OBDD::Connection node
					}
					//The BNWA_OBDD::Connection's 1 return connects to the given exit, so repeat the above with the adjusted value
					bExitVal++;
				}
			}
		}
		return current;
	}
	return NWA_OBDD::MkTrueTop();
}

NWA_OBDD::NWAOBDDTopNodeRefPtr NWAOBDDRel::AdjustedPathSummaryRecursive(NWA_OBDD::NWAOBDDNodeHandle n, int exit, const int * map, int offset) const
{
		NWA_OBDD::NWAOBDDTopNodeRefPtr topNodeMemo;
		//ETTODO insert Memoization
			NWA_OBDD::NWAOBDDTopNodeRefPtr answer = NWAOBDDRel::AdjustedPathSummaryRecursive((NWA_OBDD::NWAOBDDInternalNode*)n.handleContents, exit, map, offset);
			return answer;
}

NWA_OBDD::NWAOBDDBaseNodeRefPtr NWAOBDDRel::ModularAdjustedPathSummaryRecursive(NWA_OBDD::NWAOBDDInternalNode * n, int exit, const int map[12]) const
{
	if (n == NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[n->level].handleContents)
	{
		NWA_OBDD::NWAOBDDBaseNodeRefPtr res;
		ReturnMapHandle<intpair> m;

		m.AddToEnd(intpair(1, 1));
		m.Canonicalize();

		res = new NWA_OBDD::NWAOBDDBaseNode(n, m);
		return res;
	}
	NWA_OBDD::NWAOBDDBaseNodeRefPtr current;
	ReturnMapHandle<intpair> m;
	m.AddToEnd(intpair(0, 0));
	m.Canonicalize();
	current = new NWA_OBDD::NWAOBDDBaseNode(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[n->level], m);

	if (n->level >= 1)
	{
		int vals[12];  //An array to hold the values of the variables on the incoming and outgoing ANWA_OBDD::Connection and BNWA_OBDD::Connection edges
		//Iterate through the B-NWA_OBDD::Connections - For every middle node, there are two B-NWA_OBDD::Connections 
		for (unsigned int i = 0; i < n->numBConnections; i++)  //For every middle node
		{
			for (int j = 0; j < 2; j++)  //Look at the 0 NWA_OBDD::Connection, then the 1 NWA_OBDD::Connection
			{
				if (n->BConnection[0][i] == n->BConnection[1][i])
				{
					j = 1;
					vals[6] = NO_DISTINCTION;
				}
				else
				{
					vals[6] = j;
				}
				ReturnMapHandle<intpair> BMap = n->BConnection[j][i].returnMapHandle; //Get the Return Map of the B NWA_OBDD::Connection
				unsigned BMapSize = BMap.mapContents->mapArray.size();
				int bExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
				for (unsigned sBI = 0; sBI < BMapSize; sBI++)
				{
					intpair bExit = BMap.mapContents->mapArray[sBI];
					bool path = false;
					if (exit == bExit.First())  //The 0 return of on of the BNWA_OBDD::Connections exits connects to the exit we are looking for
					{
						path = true;
						if (exit == bExit.Second()) //The 1 return of the B-NWAOBDD::Connection exit connects to the exit we are looking for
						{
							vals[11] = NO_DISTINCTION;
						}
						else
						{
							vals[11] = 0;
						}
					}
					else if (exit == bExit.Second()) {
						path = true;
						vals[11] = 1;
					}
					if (path)
					{
						NWA_OBDD::NWAOBDDInternalNode * tempB = (NWA_OBDD::NWAOBDDInternalNode*)n->BConnection[j][i].entryPointHandle.handleContents;
						for (unsigned int k = 0; k < tempB->numBConnections; k++) //Now look at each of the B-NWA_OBDD::Connections of the current node
						{
							for (int jB = 0; jB < 2; jB++) //look at the 0 NWA_OBDD::Connection, then the 1 NWA_OBDD::Connection
							{
								if (tempB->BConnection[0][k] == tempB->BConnection[1][k])
								{
									jB = 1;
									vals[9] = NO_DISTINCTION;
								}
								else
								{
									vals[9] = jB;
								}
								ReturnMapHandle<intpair> BBMap = tempB->BConnection[jB][k].returnMapHandle; //Get the Return Map of the B NWA_OBDD::Connection
								unsigned BBMapSize = BBMap.mapContents->mapArray.size();
								int bbExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
								for (unsigned sBBI = 0; sBBI < BBMapSize; sBBI++)
								{
									intpair bbExit = BBMap.mapContents->mapArray[sBBI];
									bool path2 = false;
									if (bExitVal == bbExit.First())  //The 0 return of on of the BNWA_OBDD::Connections exits connects to the Upper B node's exit we are looking for
									{
										path2 = true;
										if (bExitVal == bbExit.Second())
										{
											vals[10] = NO_DISTINCTION;
										}
										else
										{
											vals[10] = 0;
										}
									}
									else if (bExitVal == bbExit.Second())
									{
										path2 = true;
										vals[10] = 1;
									}
									if (path2)
									{
										//We have enough information to do a recursive call on the B-NWA_OBDD::Connection of this node
										NWA_OBDD::NWAOBDDBaseNodeRefPtr BBVal = NWAOBDDRel::ModularAdjustedPathSummaryRecursive(tempB->BConnection[jB][k].entryPointHandle, bbExitVal, map);
										//We've found a path through a B NWA_OBDD::Connection of the Upper B tier, now we must find all paths from the entrance of the B-NWA_OBDD::Connection to
										//the start of the found B-NWA_OBDD::Connection - which is the kth midpoint
										for (int lB = 0; lB < 2; lB++) //Look at the two A-NWA_OBDD::Connections
										{
											if (tempB->AConnection[0] == tempB->AConnection[1])
											{
												lB = 1;
												vals[7] = NO_DISTINCTION;
											}
											else
											{
												vals[7] = lB;
											}
											ReturnMapHandle<intpair> BAMap = tempB->AConnection[lB].returnMapHandle;
											unsigned BAMapSize = BAMap.mapContents->mapArray.size();
											int baExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
											for (unsigned sBAI = 0; sBAI < BAMapSize; sBAI++)
											{
												intpair baExit = BAMap.mapContents->mapArray[sBAI];
												bool path3 = false;
												if (k == baExit.First()) //The 0 reutrn of an exit of the given ANWA_OBDD::Connection connects to the kth midpoint
												{
													path3 = true;
													if (k == baExit.Second())
													{
														vals[8] = NO_DISTINCTION;
													}
													else {
														vals[8] = 0;
													}
												}
												else if (k == baExit.Second())
												{
													path3 = true;
													vals[8] = 1;
												}
												if (path3)
												{
													//This gives us enough information to make the next recursive call on the ANWA_OBDD::Connection of the upper B Node
													NWA_OBDD::NWAOBDDBaseNodeRefPtr BAVal = NWAOBDDRel::ModularAdjustedPathSummaryRecursive(tempB->AConnection[lB].entryPointHandle, baExitVal, map);
													//We've now found a path from the ith midpoint in the lower tier node through the upper tier B-NWA_OBDD::Connection and into
													//our starting exit - now find a path from the entry of the tier to the ith midpoint
													for (int l = 0; l < 2; l++)
													{
														if (n->AConnection[0] == n->AConnection[1])
														{
															l = 1;
															vals[0] = NO_DISTINCTION;
														}
														else
														{
															vals[0] = l;
														}
														ReturnMapHandle<intpair> AMap = n->AConnection[l].returnMapHandle;
														unsigned AMapSize = AMap.mapContents->mapArray.size();
														int aExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
														for (unsigned sAI = 0; sAI < AMapSize; sAI++)
														{
															intpair aExit = AMap.mapContents->mapArray[sAI];
															bool path4 = false;
															if (i == aExit.First()) //The 0 return of an exit of the given A NWA_OBDD::Connection connects to the ith midpoint
															{
																path4 = true;
																if (i == aExit.Second())
																{
																	vals[5] = NO_DISTINCTION;
																}
																else
																{
																	vals[5] = 0;
																}
															}
															else if (i == aExit.Second())
															{
																path4 = true;
																vals[5] = 1;
															}
															if (path4)
															{
																//Now we know an A-NWA_OBDD::Connection which has a path from the entrance to the ith midpoint, we must now find
																//All such paths
																NWA_OBDD::NWAOBDDInternalNode * tempA = (NWA_OBDD::NWAOBDDInternalNode*)n->AConnection[l].entryPointHandle.handleContents;
																for (unsigned int f = 0; f < tempA->numBConnections; f++) //iterate through all the midpoints of the upper A node
																{
																	for (int jA = 0; jA < 2; jA++)
																	{
																		if (tempA->BConnection[0][f] == tempA->BConnection[1][f])
																		{
																			jA = 1;
																			vals[3] = NO_DISTINCTION;
																		}
																		else
																		{
																			vals[3] = jA;
																		}
																		ReturnMapHandle<intpair> ABMap = tempA->BConnection[jA][f].returnMapHandle;
																		unsigned ABMapSize = ABMap.mapContents->mapArray.size();
																		int abExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
																		for (unsigned sABI = 0; sABI < ABMapSize; sABI++)
																		{
																			intpair abExit = ABMap.mapContents->mapArray[sABI];
																			bool path5 = false;
																			if (aExitVal == abExit.First())
																			{
																				path5 = true;
																				if (aExitVal == abExit.Second())
																				{
																					vals[4] = NO_DISTINCTION;
																				}
																				else
																				{
																					vals[4] = 0;
																				}
																			}
																			else if (aExitVal == abExit.Second())
																			{
																				path5 = true;
																				vals[4] = 0;
																			}
																			if (path5)
																			{
																				NWA_OBDD::NWAOBDDBaseNodeRefPtr ABVal = NWAOBDDRel::ModularAdjustedPathSummaryRecursive(tempA->BConnection[jA][f].entryPointHandle, abExitVal, map);
																				//Now we only need to find a path through the A NWA_OBDD::Connection of the upper A tier from its entrance to the the midpoint f.
																				for (int lA = 0; lA < 2; lA++)
																				{
																					if (tempA->AConnection[0] == tempA->AConnection[1])
																					{
																						lA = 1;
																						vals[1] = NO_DISTINCTION;
																					}
																					else
																					{
																						vals[1] = lA;
																					}
																					ReturnMapHandle<intpair> AAMap = tempA->AConnection[lA].returnMapHandle;
																					unsigned AAMapSize = AAMap.mapContents->mapArray.size();
																					int aaExitVal = 0; //This is the current exit of the B-NWA_OBDD::Connection we are looking at
																					for (unsigned sAAI = 0; sAAI < AAMapSize; sAAI++)
																					{
																						intpair aaExit = AAMap.mapContents->mapArray[sAAI];
																						bool path6 = false;
																						if (f == aaExit.First()) //The A NWA_OBDD::Connection has a 0 return onto f
																						{
																							path6 = true;
																							if (f == aaExit.Second())
																							{
																								vals[2] = NO_DISTINCTION;
																							}
																							else
																							{
																								vals[2] = 0;
																							}
																						}
																						else if (f == aaExit.Second())
																						{
																							path6 = true;
																							vals[2] = 1;
																						}
																						if (path6)
																						{
																							//Perform another recursive call
																							NWA_OBDD::NWAOBDDBaseNodeRefPtr AAVal = NWAOBDDRel::ModularAdjustedPathSummaryRecursive(tempA->AConnection[lA].entryPointHandle, aaExitVal, map);
																							//We have all four weights so now we can construct the whole path
																							NWA_OBDD::NWAOBDDBaseNodeRefPtr pathThrough = GetCurrentPath(AAVal, ABVal, BAVal, BBVal, map, vals, n->level);
																							current = MkOr(pathThrough, current);
																						}
																						//Iterate to the next return on the map and update the exit number
																						aaExitVal++;
																					}
																				} //End of checking both the 0 and 1 A NWA_OBDD::Connection
																			}
																			//increment the iterator
																			abExitVal++;
																		}
																	} //End of block to check both 0 or 1 entry
																} //End of iteration through all of the A NWA_OBDD::Connections middle nodes
															}
															aExitVal++;
														}
													}
												}
												baExitVal++;
											}
										}
									}
									bbExitVal++;
								}
							}
						}
						//Now look at the ANWA_OBDD::Connections and see which ones have exits into given BNWA_OBDD::Connection node
					}
					//The BNWA_OBDD::Connection's 1 return connects to the given exit, so repeat the above with the adjusted value
					bExitVal++;
				}
			}
		}
		return current;
	}
	NWA_OBDD::NWAOBDDBaseNodeRefPtr answer;
	ReturnMapHandle<intpair> m0;

	m0.AddToEnd(intpair(1, 1));
	m0.Canonicalize();
	answer = new NWA_OBDD::NWAOBDDBaseNode(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[n->level], m0);
	return answer;
}

NWA_OBDD::NWAOBDDBaseNodeRefPtr NWAOBDDRel::GetCurrentPath(NWA_OBDD::NWAOBDDBaseNodeRefPtr AAVal, NWA_OBDD::NWAOBDDBaseNodeRefPtr ABVal, NWA_OBDD::NWAOBDDBaseNodeRefPtr BAVal, NWA_OBDD::NWAOBDDBaseNodeRefPtr BBVal, const int map[12], int vals[12], int nLevel) const
{
	//Create a NWAOBDDNode that represents the path to the exits we wanted
	NWA_OBDD::NWAOBDDInternalNode * n;
	n = new NWA_OBDD::NWAOBDDInternalNode(nLevel);

	//Create all the returnMaps we will need
	ReturnMapHandle<intpair> m11, m00, m01, m10, m11_00, m00_11, m00_01, m00_10, m01_00, m01_11, m10_00;
	m11.AddToEnd(intpair(1, 1));
	m11.Canonicalize();

	m00.AddToEnd(intpair(0, 0));
	m00.Canonicalize();

	m01.AddToEnd(intpair(0, 1));
	m01.Canonicalize();

	m10.AddToEnd(intpair(1, 0));
	m10.Canonicalize();

	m00_11.AddToEnd(intpair(0, 0));
	m00_11.AddToEnd(intpair(1, 1));
	m00_11.Canonicalize();

	m11_00.AddToEnd(intpair(1, 1));
	m11_00.AddToEnd(intpair(0, 0));
	m11_00.Canonicalize();

	m00_01.AddToEnd(intpair(0, 0));
	m00_01.AddToEnd(intpair(0, 1));
	m00_01.Canonicalize();

	m00_10.AddToEnd(intpair(0, 0));
	m00_10.AddToEnd(intpair(1, 0));
	m00_10.Canonicalize();

	m01_00.AddToEnd(intpair(0, 1));
	m01_00.AddToEnd(intpair(0, 0));
	m01_00.Canonicalize();

	m01_11.AddToEnd(intpair(0, 1));
	m01_11.AddToEnd(intpair(1, 1));
	m01_11.Canonicalize();

	m10_00.AddToEnd(intpair(1, 0));
	m10_00.AddToEnd(intpair(0, 0));
	m10_00.Canonicalize();

	//boolean values used by the function
	bool intND = false;
	bool int1 = false;
	bool outND = false;
	bool outTLeft = false;
	bool retND = false;
	bool retT = false;

	bool leftFalse = false;

	//The 2nd leve AConnections
	NWA_OBDD::NWAOBDDInternalNode * AA = new NWA_OBDD::NWAOBDDInternalNode(nLevel-1);
	
	//Look up the value of AA intro connection along the path
	if (map[1] == NO_DISTINCTION || vals[map[1]] == NO_DISTINCTION)
	{
		intND = true;
	}
	else if (vals[map[1]])
	{
		int1 = true;
	}

	//Look up the value of the AA out connection along the path
	if (map[2] == NO_DISTINCTION || vals[map[2]] == NO_DISTINCTION)
	{
		retND = true;
	}
	else if (vals[map[2]])
	{
		retT = true;
	}

	//Look at the exits of the AA node (should only have 1 or 2)
	//There is only one true exit
	if (AAVal->rootConnection.returnMapHandle == m11)
	{
		outND = true;
	}
	//There are two exits and the true exit is on the left
	else if (AAVal->rootConnection.returnMapHandle == m11_00)
	{
		outTLeft = true;
	}

	//The value on the intro path is ND
	if (intND)
	{
		if (retND)  //The value on the outro path is ND
		{
			if (outND)  //The node, AAVal, only has one exit
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00);
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00);
				AA->numBConnections = 1;
			}
			else //There are two exits, since the outVal is ND, in both cases it should be split 00_11
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_11);
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_11);
				AA->numBConnections = 2;
				if (!outTLeft)
				{
					leftFalse = true;
				}
			}
		}
		else if (retT) //The return value on the path is 1
		{
			if (outND) //Map the 1 return to true and the 0 to false
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01);
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01);
				AA->numBConnections = 2;
				leftFalse = true;
			}
			else if (outTLeft) //Map the left 1 return to true and the rest to false
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01_00);
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01_00);
				AA->numBConnections = 2;
				leftFalse = true;
			}
			else //Map the right 1 return to true and the rest to false
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_01);
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_01);
				AA->numBConnections = 2;
				leftFalse = true;
			}
		}
		else  //The return value is 0 on the path
		{
			if (outND) //Map the 0 return to true
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01);
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01);
				AA->numBConnections = 2;
			}
			else if (outTLeft) //Map the left 0 return to true
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01_11);
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01_11);
				AA->numBConnections = 2;
			}
			else //map the right 0 return to true
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_10);
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_10);
				AA->numBConnections = 2;
				leftFalse = true;
			}
		}
	}
	else if (int1) //The intro value on the path is 1, so the 0 connection is a false one
	{
		AA->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
		leftFalse = true;
		if (retND)
		{
			if (outND)  //The out connection is ND, so the value should be 11
			{
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m11);
				AA->numBConnections = 2;
			}
			else if (outTLeft)  //The true exit is the left one, so map the first exit to true and the second to false
			{
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m11_00);
				AA->numBConnections = 2;
			}
			else  //Do the reverse of above
			{
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_11);
				AA->numBConnections = 2;
			}
		}
		else if (retT) //The return value is 1, so map 1 connections from the true exit to the true B-connection (1)
		{
			if (outND)
			{
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01);
				AA->numBConnections = 2;
			}
			else if (outTLeft)
			{
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01_00);
				AA->numBConnections = 2;
			}
			else
			{
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_01);
				AA->numBConnections = 2;
			}
		}
		else //retValue is 0, so map 0 returns from true exit to the
		{
			if (outND) //Map should be 10
			{
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m10);
				AA->numBConnections = 2;
			}
			else if (outTLeft)
			{
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m10_00);
				AA->numBConnections = 2;
			}
			else
			{
				AA->AConnection[1] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_10);
				AA->numBConnections = 2;
			}
		}
	}
	else //incoming value is 0
	{
		if (retND)
		{
			if (outND)
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00);
				AA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				AA->numBConnections = 2;
			}
			else if (outTLeft)
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_11);
				AA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				AA->numBConnections = 2;
			}
			else
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_11);
				AA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				AA->numBConnections = 2;
				leftFalse = true;
			}
		}
		else if (retT) //The return value on the path is 1
		{
			if (outND)
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01);
				AA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				AA->numBConnections = 2;
				leftFalse = true;
			}
			else if (outTLeft)
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01_00);
				AA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				AA->numBConnections = 2;
				leftFalse = true;
			}
			else
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_01);
				AA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				AA->numBConnections = 2;
				leftFalse = true;
			}
		}
		else //return value on the path is 0
		{
			if (outND)
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01);
				AA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				AA->numBConnections = 2;
			}
			else if (outTLeft)
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m01_11);
				AA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				AA->numBConnections = 2;
			}
			else
			{
				AA->AConnection[0] = NWA_OBDD::Connection(AAVal->rootConnection.entryPointHandle, m00_10); 
				AA->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				leftFalse = true;
				AA->numBConnections = 2;
			}
		}
	}

	//Refresh the boolean values
	intND = false;
	int1 = false;
	outND = false;
	outTLeft = false;
	retND = false;
	retT = false;

	//Construct BConnection of the 2nd tier BNode
	if (map[3] == NO_DISTINCTION || vals[map[3]] == NO_DISTINCTION)
	{
		intND = true;
	}
	else if (vals[map[3]])
	{
		int1 = true;
	}

	if (map[4] == NO_DISTINCTION || vals[map[4]] == NO_DISTINCTION)
	{
		retND = true;
	}
	else if (vals[map[4]])
	{
		retT = true;
	}

	if (ABVal->rootConnection.returnMapHandle == m11)
	{
		outND = true;
	}
	else if (ABVal->rootConnection.returnMapHandle == m11_00)
	{
		outTLeft = true;
	}

	int workingB;
	
	if (AA->numBConnections == 1)
	{
		workingB = 0;
	}
	else
	{
		if (leftFalse)
		{
			workingB = 1;
		}
		else
		{
			workingB = 0;
		}
	}

	leftFalse = false;

	AA->BConnection[0] = new NWA_OBDD::Connection[AA->numBConnections];
	AA->BConnection[1] = new NWA_OBDD::Connection[AA->numBConnections];
	if (workingB == 0) //The left B->Connection is the true one (possible the only one
	{
		if (intND)
		{
			if (retND)
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00);
					AA->numExits = 1;
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_11);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_11);
					if (!outTLeft)
					{
						leftFalse = true;
					}
					AA->numExits = 2;
				}
			}
			else if (retT)
			{
				AA->numExits = 2;
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
					leftFalse = true;
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_00);
					leftFalse = true;
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_01);
					leftFalse = true;
				}
			}
			else
			{
				AA->numExits = 2;
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_11);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_11);
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_10);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_10);
					leftFalse = true;
				}
			}
		}
		else if (int1)
		{
			AA->numExits = 2;
			AA->BConnection[0][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
			leftFalse = true;
			if (retND)
			{
				if (outND)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11);
				}
				else if (outTLeft)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11_00);
				}
				else
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_11);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
				}
				else if (outTLeft)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_00);
				}
				else
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_01);
				}
			}
			else
			{
				if (outND)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10);
				}
				else if (outTLeft)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10_00);
				}
				else
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_10);
				}
			}
		}
		else //incoming value is 0
		{
			AA->numExits = 2;
			if (retND)
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_11);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_11);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
			}
			else if (retT)
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
			}
			else
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_11);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_10);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
			}
		}

		if (AA->numBConnections == 2)
		{
			AA->numExits = 2;
			if (leftFalse)
			{
				AA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				AA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
			}
			else
			{
				AA->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				AA->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
			}
		}
	}
	else //The 0 B-Connection is the false one
	{
		AA->numExits = 2;
		AA->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
		AA->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
		leftFalse = true;
		
		if (intND)
		{
			if (retND)
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11);
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11_00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11_00);
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_11);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_11);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_00);
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_01);
				}
			}
			else
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10);
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10_00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10_00);
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_10);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_10);
				}
			}
		}
		else if (int1)
		{
			AA->BConnection[0][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
			if (retND)
			{
				if (outND)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11);
				}
				else if (outTLeft)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11_00);
				}
				else
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_11);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
				}
				else if (outTLeft)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_00);
				}
				else
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_01);
				}
			}
			else
			{
				if (outND)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10);
				}
				else if (outTLeft)
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10_00);
				}
				else
				{
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_10);
				}
			}
		}
		else //incoming value is 0
		{
			if (retND)
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m11_00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_11);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m01_00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_01);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
			}
			else
			{
				if (outND)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else if (outTLeft)
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m10_00);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else
				{
					AA->BConnection[0][workingB] = NWA_OBDD::Connection(ABVal->rootConnection.entryPointHandle, m00_10);
					AA->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
			}
		}
	}
	//Now work on the higher tier into the AConnection that was just built

	int aNumExits = AA->numExits;
	NWA_OBDD::NWAOBDDNodeHandle TA = NWA_OBDD::NWAOBDDNodeHandle(AA);

	intND = false;
	int1 = false;
	outND = false;
	outTLeft = false;
	retND = false;
	retT = false;

	//Construct the Connections into the AAVal
	if (map[0] == NO_DISTINCTION || vals[map[0]] == NO_DISTINCTION)
	{
		intND = true;
	}
	else if (vals[map[0]])
	{
		int1 = true;
	}

	if (map[5] == NO_DISTINCTION || vals[map[5]] == NO_DISTINCTION)
	{
		retND = true;
	}
	else if (vals[map[5]])
	{
		retT = true;
	}

	if (aNumExits == 1)
	{
		outND = true;
	}
	else if (!leftFalse)
	{
		outTLeft = true;
	}

	leftFalse = false;
	bool bLeftFalse = false;

	if (intND)
	{
		if (retND)
		{
			if (outND)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m00);
				n->AConnection[1] = NWA_OBDD::Connection(TA, m00);
				n->numBConnections = 1;
			}
			else
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m00_11);
				n->AConnection[1] = NWA_OBDD::Connection(TA, m00_11);
				n->numBConnections = 2;
				if (!outTLeft)
				{
					bLeftFalse = true;
				}
			}
		}
		else if (retT)
		{
			if (outND)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m01);
				n->AConnection[1] = NWA_OBDD::Connection(TA, m01);
				n->numBConnections = 2;
				bLeftFalse = true;
			}
			else if (outTLeft)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m01_00);
				n->AConnection[1] = NWA_OBDD::Connection(TA, m01_00);
				n->numBConnections = 2;
				bLeftFalse = true;
			}
			else
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m00_01);
				n->AConnection[1] = NWA_OBDD::Connection(TA, m00_01);
				n->numBConnections = 2;
				bLeftFalse = true;
			}
		}
		else
		{
			if (outND)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m01);
				n->AConnection[1] = NWA_OBDD::Connection(TA, m01);
				n->numBConnections = 2;
			}
			else if (outTLeft)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m01_11);
				n->AConnection[1] = NWA_OBDD::Connection(TA, m01_11);
				n->numBConnections = 2;
			}
			else
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m00_10);
				n->AConnection[1] = NWA_OBDD::Connection(TA, m00_10);
				n->numBConnections = 2;
				bLeftFalse = true;
			}
		}
	}
	else if (int1)
	{
		n->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
		bLeftFalse = true;
		if (retND)
		{
			if (outND)
			{
				n->AConnection[1] = NWA_OBDD::Connection(TA, m11);
				n->numBConnections = 2;
			}
			else if (outTLeft)
			{
				n->AConnection[1] = NWA_OBDD::Connection(TA, m11_00);
				n->numBConnections = 2;
			}
			else
			{
				n->AConnection[1] = NWA_OBDD::Connection(TA, m00_11);
				n->numBConnections = 2;
			}
		}
		else if (retT)
		{
			if (outND)
			{
				n->AConnection[1] = NWA_OBDD::Connection(TA, m01);
				n->numBConnections = 2;
			}
			else if (outTLeft)
			{
				n->AConnection[1] = NWA_OBDD::Connection(TA, m01_00);
				n->numBConnections = 2;
			}
			else
			{
				n->AConnection[1] = NWA_OBDD::Connection(TA, m00_01);
				n->numBConnections = 2;
			}
		}
		else
		{
			if (outND)
			{
				n->AConnection[1] = NWA_OBDD::Connection(TA, m10);
				n->numBConnections = 2;
			}
			else if (outTLeft)
			{
				n->AConnection[1] = NWA_OBDD::Connection(TA, m10_00);
				n->numBConnections = 2;
			}
			else
			{
				n->AConnection[1] = NWA_OBDD::Connection(TA, m00_10);
				n->numBConnections = 2;
			}
		}
	}
	else //incoming value is 0
	{
		if (retND)
		{
			if (outND)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m00);
				n->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
				n->numBConnections = 2;
			}
			else if (outTLeft)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m00_11);
				n->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
				n->numBConnections = 2;
			}
			else
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m00_11);
				n->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				n->numBConnections = 2;
				bLeftFalse = true;
			}
		}
		else if (retT)
		{
			if (outND)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m01);
				n->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				n->numBConnections = 2;
				bLeftFalse = true;
			}
			else if (outTLeft)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m01_00);
				n->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				n->numBConnections = 2;
				bLeftFalse = true;
			}
			else
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m00_01);
				n->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				n->numBConnections = 2;
				bLeftFalse = true;
			}
		}
		else
		{
			if (outND)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m01);
				n->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
				n->numBConnections = 2;
			}
			else if (outTLeft)
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m01_11);
				n->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
				n->numBConnections = 2;
			}
			else
			{
				n->AConnection[0] = NWA_OBDD::Connection(TA, m00_10);
				n->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				bLeftFalse = true;
				n->numBConnections = 2;
			}
		}
	}

	intND = false;
	int1 = false;
	outND = false;
	outTLeft = false;
	retND = false;
	retT = false;

	leftFalse = false;

	NWA_OBDD::NWAOBDDInternalNode * BB = new NWA_OBDD::NWAOBDDInternalNode(nLevel - 1);
	
	//Construct the Connections into the AAVal
	if (map[7] == NO_DISTINCTION || vals[map[7]] == NO_DISTINCTION)
	{
		intND = true;
	}
	else if (vals[map[7]])
	{
		int1 = true;
	}

	if (map[8] == NO_DISTINCTION || vals[map[8]] == NO_DISTINCTION)
	{
		retND = true;
	}
	else if (vals[map[8]])
	{
		retT = true;
	}

	if (BAVal->rootConnection.returnMapHandle == m11)
	{
		outND = true;
	}
	else if (BAVal->rootConnection.returnMapHandle == m11_00)
	{
		outTLeft = true;
	}


	if (intND)
	{
		if (retND)
		{
			if (outND)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00);
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00);
				BB->numBConnections = 1;
			}
			else
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_11);
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_11);
				BB->numBConnections = 2;
				if (!outTLeft)
				{
					leftFalse = true;
				}
			}
		}
		else if (retT)
		{
			if (outND)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01);
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01);
				BB->numBConnections = 2;
				leftFalse = true;
			}
			else if (outTLeft)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01_00);
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01_00);
				BB->numBConnections = 2;
				leftFalse = true;
			}
			else
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_01);
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_01);
				BB->numBConnections = 2;
				leftFalse = true;
			}
		}
		else
		{
			if (outND)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01);
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01);
				BB->numBConnections = 2;
			}
			else if (outTLeft)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01_11);
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01_11);
				BB->numBConnections = 2;
			}
			else
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_10);
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_10);
				BB->numBConnections = 2;
				leftFalse = true;
			}
		}
	}
	else if (int1)
	{
		BB->AConnection[0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
		leftFalse = true;
		if (retND)
		{
			if (outND)
			{
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m11);
				BB->numBConnections = 2;
			}
			else if (outTLeft)
			{
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m11_00);
				BB->numBConnections = 2;
			}
			else
			{
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_11);
				BB->numBConnections = 2;
			}
		}
		else if (retT)
		{
			if (outND)
			{
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01);
				BB->numBConnections = 2;
			}
			else if (outTLeft)
			{
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01_00);
				BB->numBConnections = 2;
			}
			else
			{
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_01);
				BB->numBConnections = 2;
			}
		}
		else
		{
			if (outND)
			{
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m10);
				BB->numBConnections = 2;
			}
			else if (outTLeft)
			{
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m10_00);
				BB->numBConnections = 2;
			}
			else
			{
				BB->AConnection[1] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_10);
				BB->numBConnections = 2;
			}
		}
	}
	else //incoming value is 0
	{
		if (retND)
		{
			if (outND)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00);
				BB->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				BB->numBConnections = 2;
			}
			else if (outTLeft)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_11);
				BB->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				BB->numBConnections = 2;
			}
			else
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_11);
				BB->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				BB->numBConnections = 2;
				leftFalse = true;
			}
		}
		else if (retT)
		{
			if (outND)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01);
				BB->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				BB->numBConnections = 2;
				leftFalse = true;
			}
			else if (outTLeft)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01_00);
				BB->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				BB->numBConnections = 2;
				leftFalse = true;
			}
			else
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_01);
				BB->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				BB->numBConnections = 2;
				leftFalse = true;
			}
		}
		else
		{
			if (outND)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01);
				BB->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				BB->numBConnections = 2;
			}
			else if (outTLeft)
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m01_11);
				BB->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				BB->numBConnections = 2;
			}
			else
			{
				BB->AConnection[0] = NWA_OBDD::Connection(BAVal->rootConnection.entryPointHandle, m00_10);
				BB->AConnection[1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				leftFalse = true;
				BB->numBConnections = 2;
			}
		}
	}

	intND = false;
	int1 = false;
	outND = false;
	outTLeft = false;
	retND = false;
	retT = false;

	//Construct the Connections into the AAVal
	if (map[9] == NO_DISTINCTION || vals[map[9]] == NO_DISTINCTION)
	{
		intND = true;
	}
	else if (vals[map[9]])
	{
		int1 = true;
	}

	if (map[10] == NO_DISTINCTION || vals[map[10]] == NO_DISTINCTION)
	{
		retND = true;
	}
	else if (vals[map[10]])
	{
		retT = true;
	}

	if (BBVal->rootConnection.returnMapHandle == m11)
	{
		outND = true;
	}
	else if (BBVal->rootConnection.returnMapHandle == m11_00)
	{
		outTLeft = true;
	}

	workingB;

	if (BB->numBConnections == 1)
	{
		workingB = 0;
	}
	else
	{
		if (leftFalse)
		{
			workingB = 1;
		}
		else
		{
			workingB = 0;
		}
	}

	BB->BConnection[0] = new NWA_OBDD::Connection[BB->numBConnections];
	BB->BConnection[1] = new NWA_OBDD::Connection[BB->numBConnections];
	leftFalse = false;

	if (workingB == 0)
	{
		if (intND)
		{
			if (retND)
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00);
					BB->numExits = 1;
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_11);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_11);
					if (!outTLeft)
					{
						leftFalse = true;
					}
					BB->numExits = 2;
				}
			}
			else if (retT)
			{
				BB->numExits = 2;
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
					leftFalse = true;
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_00);
					leftFalse = true;
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_01);
					leftFalse = true;
				}
			}
			else
			{
				BB->numExits = 2;
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_11);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_11);
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_10);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_10);
					leftFalse = true;
				}
			}
		}
		else if (int1)
		{
			BB->numExits = 2;
			BB->BConnection[0][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
			leftFalse = true;
			if (retND)
			{
				if (outND)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11);
				}
				else if (outTLeft)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11_00);
				}
				else
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_11);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
				}
				else if (outTLeft)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_00);
				}
				else
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_01);
				}
			}
			else
			{
				if (outND)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10);
				}
				else if (outTLeft)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10_00);
				}
				else
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_10);
				}
			}
		}
		else //incoming value is 0
		{
			BB->numExits = 2;
			if (retND)
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_11);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_11);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
			}
			else if (retT)
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
			}
			else
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_11);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_10);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
					leftFalse = true;
				}
			}
		}
		if (BB->numBConnections == 2)
		{
			BB->numExits = 2;
			if (leftFalse)
			{
				BB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				BB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
			}
			else
			{
				BB->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
				BB->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m11);
			}
		}
	}
	else
	{
		BB->numExits = 2;
		BB->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
		BB->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
		leftFalse = true;

		if (intND)
		{
			if (retND)
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11);
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11_00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11_00);
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_11);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_11);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_00);
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_01);
				}
			}
			else
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10);
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10_00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10_00);
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_10);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_10);
				}
			}
		}
		else if (int1)
		{
			BB->BConnection[0][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
			if (retND)
			{
				if (outND)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11);
				}
				else if (outTLeft)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11_00);
				}
				else
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_11);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
				}
				else if (outTLeft)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_00);
				}
				else
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_01);
				}
			}
			else
			{
				if (outND)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10);
				}
				else if (outTLeft)
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10_00);
				}
				else
				{
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_10);
				}
			}
		}
		else //incoming value is 0
		{
			if (retND)
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m11_00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_11);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m01_00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_01);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
			}
			else
			{
				if (outND)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else if (outTLeft)
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m10_00);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
				else
				{
					BB->BConnection[0][workingB] = NWA_OBDD::Connection(BBVal->rootConnection.entryPointHandle, m00_10);
					BB->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 2], m00);
				}
			}
		}
	}

	int bNumExits = BB->numExits;
	NWA_OBDD::NWAOBDDNodeHandle TB = NWA_OBDD::NWAOBDDNodeHandle(BB);
	intND = false;
	int1 = false;
	outND = false;
	outTLeft = false;
	retND = false;
	retT = false;

	//Construct the Connections into the BBVal
	if (map[6] == NO_DISTINCTION || vals[map[6]] == NO_DISTINCTION)
	{
		intND = true;
	}
	else if (vals[map[6]])
	{
		int1 = true;
	}

	if (map[11] == NO_DISTINCTION || vals[map[11]] == NO_DISTINCTION)
	{
		retND = true;
	}
	else if (vals[map[11]])
	{
		retT = true;
	}

	if (bNumExits == 1)
	{
		outND = true;
	}
	else if (!leftFalse)
	{
		outTLeft = true;
	}

	if (n->numBConnections == 1)
	{
		workingB = 0;
	}
	else
	{
		if (bLeftFalse)
		{
			workingB = 1;
		}
		else
		{
			workingB = 0;
		}
	}

	n->BConnection[0] = new NWA_OBDD::Connection[n->numBConnections];
	n->BConnection[1] = new NWA_OBDD::Connection[n->numBConnections];

	leftFalse = false;

	if (workingB == 0)
	{
		if (intND)
		{
			if (retND)
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00);
					n->numExits = 1;
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_11);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_11);
					if (!outTLeft)
					{
						leftFalse = true;
					}
					n->numExits = 2;
				}
			}
			else if (retT)
			{
				n->numExits = 2;
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01);
					leftFalse = true;
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01_00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01_00);
					leftFalse = true;
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_01);
					leftFalse = true;
				}
			}
			else
			{
				n->numExits = 2;
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01);
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01_11);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01_11);
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_10);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_10);
					leftFalse = true;
				}
			}
		}
		else if (int1)
		{
			n->numExits = 2;
			n->BConnection[0][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
			leftFalse = true;
			if (retND)
			{
				if (outND)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m11);
				}
				else if (outTLeft)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m11_00);
				}
				else
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_11);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01);
				}
				else if (outTLeft)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01_00);
				}
				else
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_01);
				}
			}
			else
			{
				if (outND)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m10);
				}
				else if (outTLeft)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m10_00);
				}
				else
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_10);
				}
			}
		}
		else //incoming value is 0
		{
			n->numExits = 2;
			if (retND)
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_11);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_11);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
					leftFalse = true;
				}
			}
			else if (retT)
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
					leftFalse = true;
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01_00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
					leftFalse = true;
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
					leftFalse = true;
				}
			}
			else
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01_11);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_10);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
					leftFalse = true;
				}
			}
		}
		if (n->numBConnections == 2)
		{
			n->numExits = 2;
			if (leftFalse)
			{
				n->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				n->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
			}
			else
			{
				n->BConnection[0][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
				n->BConnection[1][1] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m11);
			}
		}
	}
	else
	{
		n->numExits = 2;
		n->BConnection[0][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
		n->BConnection[1][0] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
		leftFalse = true;

		if (intND)
		{
			if (retND)
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m11);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m11);
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m11_00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m11_00);
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_11);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_11);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01);
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01_00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01_00);
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_01);
				}
			}
			else
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m10);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m10);
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m10_00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m10_00);
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_10);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_10);
				}
			}
		}
		else if (int1)
		{
			n->BConnection[0][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
			if (retND)
			{
				if (outND)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m11);
				}
				else if (outTLeft)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m11_00);
				}
				else
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_11);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01);
				}
				else if (outTLeft)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m01_00);
				}
				else
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_01);
				}
			}
			else
			{
				if (outND)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m10);
				}
				else if (outTLeft)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m10_00);
				}
				else
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m00_10);
				}
			}
		}
		else //incoming value is 0
		{
			if (retND)
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m11);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m11_00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_11);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				}
			}
			else if (retT)
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				}
				else if (outTLeft)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m01_00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_01);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				}
			}
			else
			{
				if (outND)
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m10);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				}
				else if (outTLeft)
				{
					n->BConnection[1][workingB] = NWA_OBDD::Connection(TB, m10_00);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				}
				else
				{
					n->BConnection[0][workingB] = NWA_OBDD::Connection(TB, m00_10);
					n->BConnection[1][workingB] = NWA_OBDD::Connection(NWA_OBDD::NWAOBDDNodeHandle::NoDistinctionNode[nLevel - 1], m00);
				}
			}
		}
	}

	NWA_OBDD::NWAOBDDBaseNodeRefPtr v;
	if (n->numExits == 1)
	{
		v = new NWA_OBDD::NWAOBDDBaseNode(n, m11);
	}
	else if (leftFalse)
	{
		v = new NWA_OBDD::NWAOBDDBaseNode(n, m00_11);
	}
	else
	{
		v = new NWA_OBDD::NWAOBDDBaseNode(n, m11_00);
	}

	return v;
}


NWA_OBDD::NWAOBDDBaseNodeRefPtr NWAOBDDRel::ModularAdjustedPathSummaryRecursive(NWA_OBDD::NWAOBDDNodeHandle n, int exit, const int * map) const
{
	ModPathSummaryMemo cachedModPathSummaryMemo;
	bool isCached = modPathSummaryCache->Fetch(ModPathSummaryKey(n, exit, map), cachedModPathSummaryMemo);
	if (isCached) {
		return new NWA_OBDD::NWAOBDDBaseNode(cachedModPathSummaryMemo.result,cachedModPathSummaryMemo.returnMap);
	}
	else {
		NWA_OBDD::NWAOBDDBaseNodeRefPtr answer = NWAOBDDRel::ModularAdjustedPathSummaryRecursive((NWA_OBDD::NWAOBDDInternalNode*)n.handleContents, exit, map);
		//The goal is to find a path from the entry of the node to the given exit of the node
		//First:  Find all the B-NWA_OBDD::Connections of this node which have returns to the given exit
		modPathSummaryCache->Insert(ModPathSummaryKey(n, exit, map), ModPathSummaryMemo(NWA_OBDD::NWAOBDDNodeHandle(answer->rootConnection.entryPointHandle), answer->rootConnection.returnMapHandle));
		return answer;
	}
}


void wali::domains::nwaobddrel::InitModPathSummaryCache()
{
	modPathSummaryCache = new Hashtable<ModPathSummaryKey, ModPathSummaryMemo>(40000);
}

void wali::domains::nwaobddrel::DisposeOfModPathSummaryCache()
{
	modPathSummaryCache = NULL;
}
#ifdef BINREL_STATS
std::ostream& BddContext::printStats( std::ostream& o) const
{
  o << "NWAOBDDRel Statistics:" <<endl;
  o << "#Compose: " << numCompose << endl; 
  o << "#Union: " << numUnion << endl; 
  o << "#Intersect: " << numIntersect << endl; 
  o << "#Equal: " << numEqual << endl; 
  o << "#Kronecker: " << numKronecker << endl;
  o << "#Transpose: " << numTranspose << endl;
  o << "#Eq23Project: " << numDetensor << endl;
  o << "#Eq13Project: " << numDetensorTranspose << endl;
  return o;
}

void BddContext::resetStats()
{
  numCompose = 0;
  numUnion = 0;
  numIntersect = 0;
  numEqual = 0;
  numKronecker = 0;
  numReverse = 0;
  numTranspose = 0;
  numDetensor = 0;
  numDetensorTranspose = 0;
}
#endif //BINREL_STATS
