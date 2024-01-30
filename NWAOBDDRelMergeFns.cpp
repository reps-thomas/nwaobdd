

#include "NWAOBDDRelMergeFns.hpp"
#include <boost/cast.hpp>


using namespace std;
using namespace wali;
using namespace wali::domains;
using namespace wali::domains::nwaobddrel;

// //////////////////////////////////////////////////////
// Class MeetMergeFn
// //////////////////////////////////////////////////////
MeetMergeFnNWAOBDD::MeetMergeFnNWAOBDD(NWAOBDDContext * con, std::vector<std::string> const& localVars, std::vector<std::string> const& localVars2)
{
	// start with Id in the base domain
	NWA_OBDD::NWAOBDD havocCalleeLocalsNWAOBDD = con->getBaseOne()->getNWAOBDD();
	// start with top
	NWA_OBDD::NWAOBDD constrainLocalsNWAOBDD = con->getBaseTop()->getNWAOBDD();
	for (std::vector<std::string>::const_iterator cit = localVars.begin(); cit != localVars.end(); ++cit){
		if (con->varMap.find(*cit) == con->varMap.end()){
			cerr << "Unknown variable " << *cit << endl;
			assert(0);
		}
		// for each local variable:
		// havoc the post vocabulary for that variables
		int bi = con->varMap.find(*cit)->second;
		int varOff = con->getVLoc(BASE_POST);
		int vLoc = con->findVarLoc(bi, 0, con->maxLevel, varOff);
		havocCalleeLocalsNWAOBDD = NWA_OBDD::MkExists(havocCalleeLocalsNWAOBDD, vLoc);
	}
	for (std::vector<std::string>::const_iterator cit2 = localVars2.begin(); cit2 != localVars2.end(); ++cit2){
		if (con->varMap.find(*cit2) == con->varMap.end()){
			cerr << "Unknown variable " << *cit2 << endl;
			assert(0);
		}
		int bi = con->varMap.find(*cit2)->second;
		int varOff1 = con->getVLoc(BASE_PRE);
		int varOff2 = con->getVLoc(BASE_POST);
		int vLoc1 = con->findVarLoc(bi, 0, con->maxLevel, varOff1);
		int vLoc2 = con->findVarLoc(bi, 0, con->maxLevel, varOff2);
		NWA_OBDD::NWAOBDD t1 = NWA_OBDD::MkDistinction(vLoc1);
		NWA_OBDD::NWAOBDD t2 = NWA_OBDD::MkDistinction(vLoc2);
		// enforce id across that variables
		constrainLocalsNWAOBDD = NWA_OBDD::MkAnd(constrainLocalsNWAOBDD, NWA_OBDD::MkOr(NWA_OBDD::MkAnd(t1, t2), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(t1), NWA_OBDD::MkNot(t2))));
	}

	havocCalleeLocals = new NWAOBDDRel(con, havocCalleeLocalsNWAOBDD);
	constrainLocals = new NWAOBDDRel(con, constrainLocalsNWAOBDD);
}

bool MeetMergeFnNWAOBDD::equal(merge_fn_t mf)
{
	MeetMergeFnNWAOBDD * other = dynamic_cast<MeetMergeFnNWAOBDD*>(mf.get_ptr());
	if (other == NULL)
		return false;
	return
		(havocCalleeLocals == other->havocCalleeLocals)
		&& (constrainLocals == other->constrainLocals);
}

ostream& MeetMergeFnNWAOBDD::print(ostream& o) const
{
	o << "MeetMergeFn [" << endl;
	havocCalleeLocals->print(o << "havocCalleeLocals: ") << endl;
	constrainLocals->print(o << "constrainLocals: ") << endl;
	o << "]";
	return o;
}


// Replaces the default merge fun: w1 x w2
sem_elem_t MeetMergeFnNWAOBDD::apply_f(sem_elem_t w1, sem_elem_t w2)
{
	nwaobdd_t b1 = boost::polymorphic_downcast<NWAOBDDRel*>(w1.get_ptr());
	nwaobdd_t b2 = boost::polymorphic_downcast<NWAOBDDRel*>(w2.get_ptr());

	nwaobdd_t ret;
	ret = b2 * havocCalleeLocals;
	ret = ret & constrainLocals;
	ret = b1 * ret;

	return ret;
}


// //////////////////////////////////////////////////////
// Class MeetMergeFn
// //////////////////////////////////////////////////////
TensorMergeFnNWAOBDD::TensorMergeFnNWAOBDD(NWAOBDDContext * con, std::vector<std::string> const& localVars, std::vector<std::string> const& localVars2)
{
	id = con->getBaseOne();
	// start with Id in the base domain
	NWA_OBDD::NWAOBDD havocCalleelocalsNWAOBDD = con->getBaseOne()->getNWAOBDD();
	// start with Id in the tensor domain
	NWA_OBDD::NWAOBDD idWithEquallocalsNWAOBDD = con->getTensorOne()->getNWAOBDD();
	for (std::vector<std::string>::const_iterator cit = localVars.begin(); cit != localVars.end(); ++cit){
		if (con->varMap.find(*cit) == con->varMap.end()){
			cerr << "Unknown variable " << *cit << endl;
			assert(0);
		}
		// for each local variable:
		// havoc the post vocabulary for that variables
		int bi = con->varMap.find(*cit)->second;
		int varOff = con->getVLoc(BASE_POST);
		int vLoc = con->findVarLoc(bi, 0, con->maxLevel, varOff);
		havocCalleelocalsNWAOBDD = NWA_OBDD::MkExists(havocCalleelocalsNWAOBDD, vLoc);
	}
	for (std::vector<std::string>::const_iterator cit = localVars2.begin(); cit != localVars2.end(); ++cit){
		if (con->varMap.find(*cit) == con->varMap.end()){
			cerr << "Unknown variable " << *cit << endl;
			assert(0);
		}
		int bi = con->varMap.find(*cit)->second;
		int varOff1 = con->getVLoc(TENSOR_PRE);
		int varOff2 = con->getVLoc(TENSOR_POST);
		int vLoc1 = con->findVarLoc(bi, 0, con->maxLevel, varOff1);
		int vLoc2 = con->findVarLoc(bi, 0, con->maxLevel, varOff2);
		NWA_OBDD::NWAOBDD t1 = NWA_OBDD::MkDistinction(vLoc1);
		NWA_OBDD::NWAOBDD t2 = NWA_OBDD::MkDistinction(vLoc2);
	    // enforce equality across the two tensor levels
		idWithEquallocalsNWAOBDD = NWA_OBDD::MkAnd(idWithEquallocalsNWAOBDD, NWA_OBDD::MkOr(NWA_OBDD::MkAnd(t1, t2), NWA_OBDD::MkAnd(NWA_OBDD::MkNot(t1), NWA_OBDD::MkNot(t2))));
	}

	havocCalleeLocals = new NWAOBDDRel(con, havocCalleelocalsNWAOBDD, false);
	idWithEqualLocals = new NWAOBDDRel(con, idWithEquallocalsNWAOBDD, true);
}

bool TensorMergeFnNWAOBDD::equal(merge_fn_t mf)
{
	TensorMergeFnNWAOBDD * other = dynamic_cast<TensorMergeFnNWAOBDD*>(mf.get_ptr());
	if (other == NULL)
		return false;
	return
		havocCalleeLocals == other->havocCalleeLocals
		&& id == other->id
		&& idWithEqualLocals == other->idWithEqualLocals;
}

ostream& TensorMergeFnNWAOBDD::print(ostream& o) const
{
	o << "TensorMergeFn [" << endl;
	havocCalleeLocals->print(o << "havocCalleeLocals: ") << endl;
	id->print(o << "id: ") << endl;
	idWithEqualLocals->print(o << "idWithEqualLocals: ") << endl;
	o << "]";
	return o;
}

// Replaces the default merge fun: w1 x w2
sem_elem_t TensorMergeFnNWAOBDD::apply_f(sem_elem_t w1, sem_elem_t w2)
{
	sem_elem_tensor_t b1 = boost::polymorphic_downcast<SemElemTensor*>(w1.get_ptr());
	sem_elem_tensor_t b2 = boost::polymorphic_downcast<SemElemTensor*>(w2.get_ptr());

	sem_elem_tensor_t ret = b2;
	ret = boost::polymorphic_downcast<SemElemTensor*>(ret->extend(havocCalleeLocals.get_ptr()).get_ptr());
	ret = ret->tensor(id.get_ptr());
	ret = boost::polymorphic_downcast<SemElemTensor*>(idWithEqualLocals->extend(ret.get_ptr()).get_ptr());
	ret = ret->detensor();
	ret = boost::polymorphic_downcast<SemElemTensor*>(b1->extend(ret.get_ptr()).get_ptr());
	return ret;
}