
test_correctness() {
    ./nwaobdd.out dj 500 1
    ./nwaobdd.out dj 500 2
    ./nwaobdd.out dj 500 3

    ./nwaobdd.out bv 500 1
    ./nwaobdd.out bv 500 2
    ./nwaobdd.out bv 500 3

    ./nwaobdd.out ghz 500 1
    ./nwaobdd.out ghz 500 2
    ./nwaobdd.out ghz 500 3

    ./nwaobdd.out grovers 5 1
    ./nwaobdd.out grovers 6 2
    ./nwaobdd.out grovers 7 3

    ./nwaobdd.out qft 8 1
    ./nwaobdd.out qft 9 2
    ./nwaobdd.out qft 10 3

    ./nwaobdd.out simons 8 1
    ./nwaobdd.out simons 9 2
    ./nwaobdd.out simons 10 3
}

run() {
    echo "$1 $2:"
    echo "running $1 $2" >&2
    echo -n "  "
    ./nwaobdd.out $1 $2
}

test_performance(){
    run bv 10000
    run bv 20000
    run bv 40000
    run bv 80000
    # run bv 160000

    echo ""

    run dj 10000
    run dj 20000
    run dj 40000
    run dj 80000
    # run dj 160000

    echo ""

    run ghz 10000
    run ghz 20000
    run ghz 40000
    run ghz 80000
    # run ghz 160000

    echo ""

    run grovers 9
    run grovers 10
    run grovers 11
    run grovers 12

    echo ""

    run simons 13
    run simons 14
    run simons 15

    echo ""

    run qft 20
    run qft 21
    run qft 22
}


if [ "$1" = "perf" ]; then
    echo "Running Performance Test" >&2
    test_performance
else
    echo "Running Correctness Test" >&2
    test_correctness
fi