#include "nwaobdd_top_node.cpp"
#include"vector/vector_complex.h"

namespace NWA_OBDD {
    template class NWAOBDDTopNode<int>;
    template std::ostream& operator<< (std::ostream & out, const NWAOBDDTopNode<int> &d);
    template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr ApplyAndReduce<int>(
        NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr n1,
        NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr n2,
        BoolOp op);

    template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr MkTimesTopNode<int>(
        NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f,
        NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);
    template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr MkPlusTopNode<int>(
        NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f,
        NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);


    template class NWAOBDDTopNode<BIG_COMPLEX_FLOAT>;
    template std::ostream& operator<< (std::ostream & out, const NWAOBDDTopNode<BIG_COMPLEX_FLOAT> &d);


    template typename NWAOBDDTopNode<BIG_COMPLEX_FLOAT>::NWAOBDDTopNodeTRefPtr
    MkTimesTopNode<BIG_COMPLEX_FLOAT>(typename NWAOBDDTopNode<BIG_COMPLEX_FLOAT>::NWAOBDDTopNodeTRefPtr f,
                        typename NWAOBDDTopNode<BIG_COMPLEX_FLOAT>::NWAOBDDTopNodeTRefPtr g);

    template typename NWAOBDDTopNode<BIG_COMPLEX_FLOAT>::NWAOBDDTopNodeTRefPtr
    MkPlusTopNode<BIG_COMPLEX_FLOAT>(typename NWAOBDDTopNode<BIG_COMPLEX_FLOAT>::NWAOBDDTopNodeTRefPtr f,
                    typename NWAOBDDTopNode<BIG_COMPLEX_FLOAT>::NWAOBDDTopNodeTRefPtr g);
    
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr ApplyAndReduce<int>(
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr n1,
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr n2,
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr n3,
    //     BoolOp3 op);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr MkPlusTopNode<int>(
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f,
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr operator+<int>(
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f, 
	//     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr MkExorTopNode<int>(
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f,
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr operator^<int>(
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f, 
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr MkLeftScalarTimesTopNode<int, int>(
    //     int c, NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr operator*<int, int>(
    //     int c, NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr MkRightScalarTimesTopNode<int>(
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f, int c);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr operator*<int>(
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f, int c);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr MkTimesTopNode<int>(
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f,
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);
    // template NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr operator*<int>(
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr f,
    //     NWAOBDDTopNode<int>::NWAOBDDTopNodeTRefPtr g);
}