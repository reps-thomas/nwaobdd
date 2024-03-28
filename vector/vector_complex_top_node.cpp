#include"vector_complex_top_node.h"
#include"vector_node.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <chrono>
#include <random>

using namespace SH_OBDD;
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

        NWAOBDDTopNodeComplexFloatBoostRefPtr VectorWithAmplitudeTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n) {
            ReturnMapHandle<BIG_COMPLEX_FLOAT> rhandle;
            for(unsigned i = 0; i < n->rootConnection.returnMapHandle.Size(); ++i) {
                BIG_COMPLEX_FLOAT val = n->rootConnection.returnMapHandle[i];
                auto real_val = val.real();
                auto imag_val = val.imag();
                auto abs_val = real_val * real_val + imag_val * imag_val;
                rhandle.AddToEnd(abs_val);
            }
            ReductionMapHandle inducedReductionMapHandle;
			ReturnMapHandle<BIG_COMPLEX_FLOAT> inducedReturnMap;
			rhandle.InducedReductionAndReturnMap(inducedReductionMapHandle, inducedReturnMap);
            NWAOBDDNodeHandle reduced_n = n->rootConnection.entryPointHandle->Reduce(inducedReductionMapHandle, inducedReturnMap.Size());
            return (new NWAOBDDTopNodeComplexFloatBoost(reduced_n, inducedReturnMap));
        }
        
		unsigned long long int GetPathCountTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, long double p)
		{
			for (unsigned int i = 0; i < n->rootConnection.returnMapHandle.Size(); i++)
			{
				if (n->rootConnection.returnMapHandle.Lookup(i) != 0){
					long double v = n->rootConnection.returnMapHandle.Lookup(i).real().convert_to<long double>();
					if (abs(v - p) < std::numeric_limits<double>::epsilon())
					{
						long double logNumPaths = n->rootConnection.entryPointHandle->handleContents->numPathsToExit[i];
						return std::pow(2, logNumPaths);
					}
				}
			}
			return 0;
		}

        long double getNonZeroProbabilityTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n) {
			long double prob = 0;
			for (unsigned int i = 0; i < n->rootConnection.returnMapHandle.Size(); i++) {
				if (n->rootConnection.returnMapHandle.Lookup(i) != 0){
					long double v = n->rootConnection.returnMapHandle.Lookup(i).real().convert_to<long double>();
					long double logNumPaths = n->rootConnection.entryPointHandle->handleContents->numPathsToExit[i];
					prob += v * std::pow(2, logNumPaths);
				}
			}
			return prob;
		}

		std::string SamplingTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, bool VocTwo) {
			std::vector<std::pair<BIG_FLOAT, unsigned int>> values;
			long double prob = -1 * std::numeric_limits<long double>::infinity();

			for (unsigned int i = 0; i < n->rootConnection.returnMapHandle.Size(); i++)
			{
				if (n->rootConnection.returnMapHandle.Lookup(i) == 0){
					values.push_back(std::make_pair(-1 * std::numeric_limits<BIG_FLOAT>::infinity(), i));
				}
				else{
					BIG_FLOAT v = n->rootConnection.returnMapHandle.Lookup(i).real().convert_to<BIG_FLOAT>();
					BIG_FLOAT amplitude = mp::log2(v);
					long double logNumPaths = n->rootConnection.entryPointHandle->handleContents->numPathsToExit[i];
					values.push_back(std::make_pair(amplitude + logNumPaths, i));
				}
			}


			// sort(values.begin(), values.end(), sortNumPathPairs<BIG_FLOAT>);
			sort(values.begin(), values.end());

			prob = getLogSumNumPaths(values, values.size()).convert_to<long double>();

			BIG_FLOAT val = -1 * std::numeric_limits<BIG_FLOAT>::infinity();
			long double random_value = 0.0;
			if (prob >= 64){
				std::random_device rd;
				std::default_random_engine generator(rd());
				std::uniform_int_distribution<long long unsigned> distribution(0, 0xFFFFFFFFFFFFFFFF);
				random_value = log2l(distribution(generator)) + prob - 64;
			}
			else{
				auto rand_val = rand();
				random_value = log2l((((double)rand_val) / RAND_MAX)*pow(2, prob));
			}

			unsigned int index = 0;
			for (unsigned int i = 0; i < values.size(); i++)
			{
				val = getLogSumNumPaths(values, i + 1);
				if (val >= random_value)
				{
					index = values[i].second;
					break;
				}
			}
			return SamplingNode(*(n->rootConnection.entryPointHandle), index, VocTwo);
		}
		void pad_string(AssignmentIterator &ai, std::string s, unsigned level) {
			assert(level > 0);
			if(level == 1) {
				assert(s.length() == 2);
				ai.Current() = (s[0] == '1'), ai.Next();
				ai.Current() = 0, ai.Next();
				ai.Current() = (s[1] == '1'), ai.Next();
				ai.Current() = 0, ai.Next();
			}
			else {
				ai.Current() = 0, ai.Next();
				pad_string(ai,s.substr(0, s.length() / 2), level - 1);
				ai.Current() = 0, ai.Next();
				ai.Current() = 0, ai.Next();
				pad_string(ai,s.substr(s.length() / 2), level - 1);
				ai.Current() = 0, ai.Next();
			}
		}
		Assignment index2assignment(std::string s, unsigned level) {
			Assignment a( (1 << (level + 2)) - 4 );
			AssignmentIterator ai(a);
			pad_string(ai, s, level);
			return a;
		}
		void DumpVectorTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n) {
			unsigned level = n -> level;
			unsigned vars = 1 << level;
			std::cout << '(';
			for(unsigned index = 0; index < (1 << vars); ++index) {
				std::string s;
				for(unsigned j = 0; j < vars; ++j)
					if(index & (1 << j)) s.push_back('1');
					else s.push_back('0');
				Assignment a = index2assignment(s, level);
				auto r = n->Evaluate(a);
				std::cout << r << ", ";
			}
			std::cout << ")\n";
		}
    }

}