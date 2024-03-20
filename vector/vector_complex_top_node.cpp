#include"vector_complex_top_node.h"
#include"vector_node.h"

namespace NWA_OBDD {


    template class NWAOBDDTopNode<BIG_COMPLEX_FLOAT>;

    namespace VectorComplex {

        void VectorInitializerTop() {
            VectorInitializerNode();
        }

        NWAOBDDTopNodeComplexFloatBoostRefPtr MkBasisVectorTop(unsigned int level, unsigned int index) {
            
            auto top_node = MkBasisVectorNode(level, index);
            ComplexFloatBoostReturnMapHandle value_tuple;
            if(index == 0) {
                value_tuple.AddToEnd(1);
                value_tuple.AddToEnd(0);
            }
            else {
                value_tuple.AddToEnd(0);
                value_tuple.AddToEnd(1);
            }
            value_tuple.Canonicalize();
            auto ptr = new NWAOBDDTopNodeComplexFloatBoost(top_node, value_tuple);
            return ptr;
        }
        NWAOBDDTopNodeComplexFloatBoostRefPtr MkBasisVectorTop(unsigned int level, std::string index) {
            
            auto top_node = MkBasisVectorNode(level, index);
            ComplexFloatBoostReturnMapHandle value_tuple;
            if(index.find('1') == std::string::npos) {
                value_tuple.AddToEnd(1);
                value_tuple.AddToEnd(0);
            }
            else {
                value_tuple.AddToEnd(0);
                value_tuple.AddToEnd(1);
            }
            value_tuple.Canonicalize();
            auto ptr = new NWAOBDDTopNodeComplexFloatBoost(top_node, value_tuple);
            return ptr;
        }

        
    }
}