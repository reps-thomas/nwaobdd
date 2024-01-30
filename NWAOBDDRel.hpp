#ifndef NWAOBDD_REL_HPP_GUARD
#define NWAOBDD_REL_HPP_GUARD
#include <map>
#include <vector>
#include <utility>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp> // It'd be nice to include the standard version but there are too many ways to get it.
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>


#include "wali/wfa/WeightMaker.hpp"
#include "wali/Countable.hpp"
#include "wali/ref_ptr.hpp"
#include "wali/SemElemTensor.hpp"
#include "fdd.h"
#include "bdd.h"
#include "wali/Key.hpp"


#include "nwaobdd.h"

namespace wali
{
	namespace domains
	{
		namespace nwaobddrel
		{
	enum vocabulary_t {BASE_PRE, BASE_POST, BASE_EXTRA, TENSOR_PRE, TENSOR_POST, TENSOR_EXTRA, REG_A_INFO, REG_B_INFO, REG_C_INFO};
#define NO_DISTINCTION 12

	enum ordering {BASE_1ST_TENSOR_ROOT, SEPERATE_LEVELS, AFFINITY};

	class NWAOBDDRel;
	typedef::wali::ref_ptr<NWAOBDDRel> nwaobdd_t;

	class NWAOBDDContext
      {
        friend class NWAOBDDRel;
        public:
          NWAOBDDContext(ordering o);
          NWAOBDDContext(const NWAOBDDContext& other);
          NWAOBDDContext& operator = (const NWAOBDDContext& other);
          virtual ~NWAOBDDContext();

		  NWA_OBDD::NWAOBDD NWAOBDDContext::HavocVar(std::string var, NWA_OBDD::NWAOBDD current);
		  NWA_OBDD::NWAOBDD NWAOBDDContext::HavocExtra(NWA_OBDD::NWAOBDD current) const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::AssignGen(std::string var, NWA_OBDD::NWAOBDD expr, NWA_OBDD::NWAOBDD aType) const;
		  void NWAOBDDContext::setIntVars(const std::map<std::string, int>& inflatvars, int level);
		  std::vector<std::string> NWAOBDDContext::CreateMergeList(int type);
		  NWA_OBDD::NWAOBDD NWAOBDDTCreate(int var, vocabulary_t vocab);
		  NWA_OBDD::NWAOBDD NWAOBDDContext::From(std::string lhs, std::string var);
		  NWA_OBDD::NWAOBDD NWAOBDDContext::From(std::string var);
		  int findVarLoc(int var, int offset, int level, int vocabOff) const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::True() const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::False() const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::Not(NWA_OBDD::NWAOBDD expr) const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::And(NWA_OBDD::NWAOBDD lexpr, NWA_OBDD::NWAOBDD rexpr) const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::Or(NWA_OBDD::NWAOBDD lexpr, NWA_OBDD::NWAOBDD rexpr) const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::NWANot() const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::NWAOr() const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::NWAAnd() const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::Assume(NWA_OBDD::NWAOBDD lexpr, NWA_OBDD::NWAOBDD rexpr) const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::applyUnOp(NWA_OBDD::NWAOBDD expr, NWA_OBDD::NWAOBDD op) const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::applyBinOp(NWA_OBDD::NWAOBDD lexpr, NWA_OBDD::NWAOBDD rexpr, NWA_OBDD::NWAOBDD op) const;
		  NWA_OBDD::NWAOBDD NWAOBDDContext::True(std::string var);
		  NWA_OBDD::NWAOBDD NWAOBDDContext::False(std::string var);
		  NWA_OBDD::NWAOBDD NWAOBDDContext::createBaseID(ordering o);
		  void NWAOBDDContext::addVarList(std::pair<int, int>, std::vector<std::string> vars1, std::vector<std::string> vars2);
		  std::pair<std::vector<std::string>,std::vector<std::string>> NWAOBDDContext::getLocalVars(std::pair<int,int> loc) const;


          /**
           * Accessor functions for cached id and zero semiring elements.
           * The cached copy is created at the time the
           * vocabulary is set up. So it's safe to call these
           * after that.
           *
           * Always usfe these to access any of these special elements. All
           * semiring operations take care to not create duplicates.
           **/

		  	//Get the two maps
	    int * getUnchangedMap()const ;
		int * getNoDistinctionMap() const;

	//Get the locations of the vocabularies
	int getVLoc(vocabulary_t v) const;
          nwaobdd_t getBaseOne() const;
          nwaobdd_t getBaseZero() const;
          nwaobdd_t getTensorOne() const;
          nwaobdd_t getTensorZero() const;
          nwaobdd_t getBaseTop() const;
          nwaobdd_t getTensorTop() const;
		  nwaobdd_t getDetensorId() const;
		  int getRegAInfo() const;
		  	int getBasePreLoc() const;
	int getBasePostLoc() const;
	int getBaseExtraLoc() const;
	int getTensorPreLoc() const;
	int getTensorPostLoc() const;
	int getTensorExtraLoc() const;
	void setUpLocations();
	NWA_OBDD::NWAOBDD BaseID() const;
	ordering getOrdering();
	int numTotalVars;
	int maxLevel;

        protected:
          virtual void setupCachedNWAOBDDs(ordering o);
        public:
          //using wali::Countable::count;
          int count;
		  std::map< const std::string, int > varMap;
        private:
          /** caches zero/one objects for this context **/
          void populateCache(ordering o);

		  std::map<std::pair<int,int>, std::pair<std::vector<std::string>,std::vector<std::string>>> mergeVars;

          //We cache zero and one BinRel objects, since they are used so much
          nwaobdd_t cachedBaseOne;
          nwaobdd_t cachedBaseZero;
          nwaobdd_t cachedTensorOne;
          nwaobdd_t cachedTensorZero;
          nwaobdd_t cachedBaseTop;
          nwaobdd_t cachedTensorTop;
		  nwaobdd_t cachedDetensorId;
		  ordering order;
		  int unchangedMap[12];
		  int noDistinctionMap[12];
		  int basePreLoc;
		  int basePostLoc;
		  int baseExtraLoc;
		  int tensorPreLoc;
		  int tensorPostLoc;
		  int tensorExtraLoc;
		  int regAInfo;
		  int regBInfo;
		  int regCInfo;

        private:
          //Initialization of buddy is taken care of opaquely 
          //by keeping track of the number of BddContext objects alive
			NWA_OBDD::NWAOBDD baseId;
          static int numNWAOBDDContexts;
		  NWA_OBDD::NWAOBDD createBaseOneTop(ordering orderType);
	NWA_OBDD::NWAOBDD createBaseZeroTop(ordering orderType);
	NWA_OBDD::NWAOBDD createBaseTopTop(ordering orderType);
	NWA_OBDD::NWAOBDD createTensorOneTop(ordering orderType);
	NWA_OBDD::NWAOBDD createTensorZeroTop(ordering orderType);
	NWA_OBDD::NWAOBDD createTensorTopTop(ordering orderType);
	NWA_OBDD::NWAOBDDNodeHandle createBaseOne(int level, ordering orderType, int offset);
	NWA_OBDD::NWAOBDDNodeHandle createBaseZero(int level, ordering orderType);
	NWA_OBDD::NWAOBDDNodeHandle createBaseTop(int level, ordering orderType);
	NWA_OBDD::NWAOBDDNodeHandle createTensorOne(int level, ordering orderType);
	NWA_OBDD::NWAOBDDNodeHandle createTensorZero(int level, ordering orderType);
	NWA_OBDD::NWAOBDDNodeHandle createTensorTop(int level, ordering orderType);
	NWA_OBDD::NWAOBDD NWAOBDDContext::createDetensorId();
      };
	
	class NWAOBDDRel : public wali::SemElemTensor
    {
		public:
          static void reset();
        public:
          /** @see BinRel::Compose */
          friend nwaobdd_t operator*(nwaobdd_t a, nwaobdd_t b);
          /** @see BinRel::Union */
          friend nwaobdd_t operator|(nwaobdd_t a, nwaobdd_t b);
          /** @see BinRel::Intersect */
          friend nwaobdd_t operator&(nwaobdd_t a, nwaobdd_t b);
        public:
          NWAOBDDRel(const NWAOBDDRel& that);
          NWAOBDDRel(NWAOBDDContext const * con, NWA_OBDD::NWAOBDD n, bool is_tensored=false);
          virtual ~NWAOBDDRel();
        public:
          nwaobdd_t Compose( nwaobdd_t that ) const;
          nwaobdd_t Union( nwaobdd_t that ) const;
          nwaobdd_t Intersect( nwaobdd_t that ) const;
          bool Equal(nwaobdd_t se) const;
          nwaobdd_t Transpose() const;
          nwaobdd_t Kronecker( nwaobdd_t that) const;
          nwaobdd_t DetensorTranspose() const;
		  nwaobdd_t Detensor() const;
		  nwaobdd_t TensorTranspose() const;
		  nwaobdd_t MoveBaseToTensor();
		  nwaobdd_t ExistentialQuantification(vocabulary_t vocabs[], int length);
		  nwaobdd_t Merge(int v, int c) const;
 
		bool isZero();
		bool isOne();
		void CreateTransposeMap(int * map) const;
		void  CreateDetensorMap(int * map);
		void  CreateDetensorTransposeMap(int * map);
		void  CreateMoveBaseToTensorMap(int * map) const;
		void  CreateExistentialMap(vocabulary_t * vocabs, int length, int * map);
		void  CreateBaseRightShiftMap(int * map) const;
		void  CreateTensorRightShiftMap(int * map) const;
		void  CreateBaseRestoreMap(int * map) const;
		void  CreateTensorRestoreMap(int * map) const;
		void  CreateMoveBasePreToExtraMap(int * map) const;
		void  CreateMoveBasePostToExtraMap(int * map) const;
		void  CreateMoveBaseExtraToPostMap(int * map) const;
		void  CreateCopyBasePostToExtraMap(int * map) const;
		std::vector<int> NWAOBDDRel::getVariableVals(int vals[12]) const;
		std::ostream& printBase1stTensorRoot(std::ostream& o) const;
		std::set<std::vector<int>> NWAOBDDRel::pathSummaryPrintRecursive(NWA_OBDD::NWAOBDDNodeHandle n, int exit, int offset) const;
		std::set<std::vector<int>> NWAOBDDRel::pathSummaryPrintRecursive(NWA_OBDD::NWAOBDDInternalNode * n, int exit, int offset) const;
		std::ostream& NWAOBDDRel::pathSummaryPrint(std::ostream& o) const;
		std::string NWAOBDDRel::convertValsToString(std::vector<int> vals) const;
		std::set<std::string> NWAOBDDRel::getPrintOutput(std::set<std::vector<int>> output) const;
		std::set<std::vector<int>> NWAOBDDRel::reducePrintOutputRecursive(std::set<std::vector<int>> v) const;
		std::set<std::vector<int>> NWAOBDDRel::reducePrintOutput(std::set<std::vector<int>> vOut) const;
		NWA_OBDD::NWAOBDDTopNodeRefPtr GetWeight(const int map[12], int vals[12], int vLocs[12], int c) const;
		NWA_OBDD::NWAOBDDBaseNodeRefPtr GetCurrentPath(NWA_OBDD::NWAOBDDBaseNodeRefPtr AAVal, NWA_OBDD::NWAOBDDBaseNodeRefPtr ABVal, NWA_OBDD::NWAOBDDBaseNodeRefPtr BAVal, NWA_OBDD::NWAOBDDBaseNodeRefPtr BBVal, const int map[12], int vals[12], int nLevel) const;
		NWA_OBDD::NWAOBDDTopNodeRefPtr AdjustedPathSummaryRecursive(NWA_OBDD::NWAOBDDInternalNode * n, int exit, const int map[12], int offset) const;
		NWA_OBDD::NWAOBDDBaseNodeRefPtr ModularAdjustedPathSummaryRecursive(NWA_OBDD::NWAOBDDInternalNode * n, int exit, const int map[12]) const;
		NWA_OBDD::NWAOBDDTopNodeRefPtr AdjustedPathSummaryRecursive(NWA_OBDD::NWAOBDDNodeHandle n, int exit, const int map[12], int offset) const;
		NWA_OBDD::NWAOBDDBaseNodeRefPtr ModularAdjustedPathSummaryRecursive(NWA_OBDD::NWAOBDDNodeHandle n, int exit, const int map[12]) const;
		nwaobdd_t SetEqualVocabs(vocabulary_t v1, vocabulary_t v2);
		NWA_OBDD::NWAOBDD AdjustedPathSummary(NWA_OBDD::NWAOBDD n, const int * map) const;

        public: 
	  // ////////////////////////////////
          // SemElem methods
          sem_elem_t one() const;
          sem_elem_t zero() const;

          bool isOne() const {
            if(isTensored)
              return getNWAOBDD() == con->cachedTensorOne->getNWAOBDD();
            else
              return getNWAOBDD() == con->cachedBaseOne->getNWAOBDD();
          }

          bool isZero() const {
            if(isTensored)
              return getNWAOBDD() == con->cachedTensorZero->getNWAOBDD();
            else
              return getNWAOBDD() == con->cachedBaseZero->getNWAOBDD();
          }
        
          /** @return [this]->Union( cast<BinRel*>(se) ) */
          sem_elem_t combine(SemElem* se);

          /** @return [this]->Compose( cast<BinRel*>(se) ) */
          sem_elem_t extend(SemElem* se);

          sem_elem_t star();

          /** @return [this]->Equal( cast<BinRel*>(se) ) */
          bool equal(SemElem* se) const;

          std::ostream& print( std::ostream& o ) const;

          // ////////////////////////////////
          // SemElemTensor methods

          /** @return [this]->Transpose() */
          sem_elem_tensor_t transpose();

          /** @return [this]->Kronecker( cast<BinRel*>(se) ) */
          sem_elem_tensor_t tensor(SemElemTensor* se);

          /** @return [this]->Eq23Project() */
          sem_elem_tensor_t detensor();

          /** @return [this]->Eq34Project() */
          sem_elem_tensor_t detensorTranspose();

          /** @return The backing BDD */
          NWA_OBDD::NWAOBDD getNWAOBDD() const {
            return rel;
          }

          /** @return Get the vocabulary the relation is over */
        // TODO: change name -eed May 14 2012
          NWAOBDDContext const & getVocabulary() const {
            return *con;
          }

          virtual bool containerLessThan(SemElem const * other) const;

          virtual size_t hash() const {
            return static_cast<size_t>(getNWAOBDD().Hash(256));//ETTODO - Fix HASH
          }

	  bool isWTensored()
	  {
	    return isTensored;
	  }

	  void setTensored(bool tens)
	  {
	    isTensored = tens;
	  }
	  // ////////////////////////////////
          // Printing functions
          //static void printHandler(FILE *o, int var);
        protected:
          //This has to be a raw/weak pointer.
          //BddContext caches some BinRel objects. It is not BinRel's responsibility to
          //manage memory for BddContext. 
          NWAOBDDContext const * con;
          NWA_OBDD::NWAOBDD rel;
	  bool isTensored;
      };

	  class ModPathSummaryMemo {

	  public:
		  ModPathSummaryMemo();                                 // Default constructor
		  ModPathSummaryMemo(NWA_OBDD::NWAOBDDNodeHandle result, ReturnMapHandle<intpair> returnMap); // Constructor
		  ModPathSummaryMemo& operator= (const ModPathSummaryMemo& p);  // Overloaded assignment
		  bool operator!= (const ModPathSummaryMemo& p);        // Overloaded !=
		  bool operator== (const ModPathSummaryMemo& p);        // Overloaded ==

		  NWA_OBDD::NWAOBDDNodeHandle result;
		  ReturnMapHandle<intpair>  returnMap;
	  };


	  class ModPathSummaryKey {
	  public:
		  ModPathSummaryKey(NWA_OBDD::NWAOBDDNodeHandle nodeHandle1, int exit, const int map[12]); // Constructor
		  unsigned int Hash(unsigned int modsize);
		  ModPathSummaryKey& operator= (const ModPathSummaryKey& p);  // Overloaded assignment
		  bool operator!= (const ModPathSummaryKey& p);        // Overloaded !=
		  bool operator== (const ModPathSummaryKey& p);        // Overloaded ==
		  NWA_OBDD::NWAOBDDNodeHandle NodeHandle1() const { return nodeHandle1; }      // Access function
		  int Exit() const { return this->exit; } //Access function
		  void ModPathSummaryKey::Print(std::ostream & out) const;

	  private:
		  NWA_OBDD::NWAOBDDNodeHandle nodeHandle1;
		  int exit;
		  int map[12];
		  ModPathSummaryKey();                                 // Default constructor (hidden)

	  };

	  void InitModPathSummaryCache();
	  void DisposeOfModPathSummaryCache();
}
}
}

#endif