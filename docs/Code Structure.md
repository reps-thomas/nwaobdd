# Code Structure

## ./

currently the core of the code is in this directory

### bool_op.\*

the basic definitions of unary, binary, and ternary Boolean-functions

### assignment.\*

##### class SH_OBDD::Assignment

maintains an assignment for the variables, like $[x_0\mapsto 1,x_1\mapsto0,\cdots,x_i\mapsto a_i,\cdots]$. 

This class is mainly used in the following four functions:

**bool Evaluate(Assignment &assignment)** in *nwaobdd.h*

evaluate the value of Boolean function, returning "bool" by coercing the "int" into a "bool".

**int Traverse(AssignmentIterator &ai)** in *nwaobdd_node.h*

The evaluating process of a NWAOBDD as an automata, this shows the operational semantics of the NWAOBDDs.

**bool FindOneSatisfyingAssignment(Assignment * &assignment)** in *nwaobdd_node.h*

If a satisfying assignment exists, allocate and place such an assignment in variable "assignment" and return true. Otherwise, do nothing to variable "assignment" and return false.

Linear time in the number of variables

[To do: is it the lexicographically-least answer]

**bool FillSatisfyingAssignment(Assignment * &assignment)** in *nwaobdd_node.h*

sub-procedure that is called by find one here.

number of Satisfying assignment. Path counting: the linear time of the NWAOBDD size.

### traverse_state_nwa.\*   cross_product_nwa.\*  ruduction_map.\*

recording the information about pair-product.

### nwaobdd_node.*

"node" has the meaning of "grouping" in our docs. Currently 7 main classes about nodes here.

TBD: no template-code for NWAOBDDTopNode

TBD: possibly rearranging the code here (and renaming the code)

```cpp
class NWAOBDDTopNode;
class NWAOBDDBaseNode;
class NWAOBDDNode;
class NWAOBDDInternalNode;   //  : public NWAOBDDNode
class NWAOBDDLeafNode;       //  : public NWAOBDDNode
class NWAOBDDEpsilonNode;   //  : public NWAOBDDLeafNode
class NWAOBDDNodeHandle;
```

and an ***class Connection***. 

TBD: no template-code for this part

TBD: possibly should take it to a separate file

### return_map.*

the return_map, used in the connection part.

### nwaobdd.\*

The standard APIs of our code.

## ./infra/

This directory implements some basic "infrastructures". We could make the code **more readable** if we remove the dependency of this part. (TBD: will we obtain a better performance, then?)

### intpair.*

##### class intpair

like *pair<int, int>*. we would need a careful choice of hash-function.

### inttriple.*

##### class inttriple

like *tuple<int, int, int>*. 

### hash.*

##### class Hashtable<Key, Value>

like *unordered_map<Key, Value>*. The *modsize* might be another fine-tuning problem.

### hashset.*

##### class Hashset\<T\>

I'd like to call it ***CanonicalTable\<T\>***. It maintains a set of canonical items of **T**, but interacting through the pointer. It behaves like ***unordered_set<T\*, Hash_by_Pointee, Equality_by_Pointee>***.

### ref_ptr.h

##### class RefCounter

just an encapsulated "unsigned int", but explicitly stating the variable usage with the type-name

Could we obtain a better performance if we just do "#define RefCounter unsigned" here?

##### class ref_ptr\<T\>

like std::***shared_ptr\<T\>***, but possibly we will have a more efficient implementation because ours is **non-thread-safe**. We need there to be a member "RefCounter count" in type T.

### conscell.*

##### class ConsCell\<T\>

a **singly linked list** type, seems to be used as **stack**

only instantiated into ***ConsCell\<TraverseState\>***

### list_T.h 

##### class List\<T\>

**singly linked list** type, can be iteratively accessed.

### list_TPtr.h

It seems that we can compile without this file. Need further check.



