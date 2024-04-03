
run() {
    echo "$1 $2:"
    echo -n "  "
    ./nwaobdd.out $1 $2
}

run bv 10000
run bv 20000
run bv 40000
run bv 80000
run bv 160000

echo ""

run dj 10000
run dj 20000
run dj 40000
run dj 80000
run dj 160000

echo ""

run ghz 10000
run ghz 20000
run ghz 40000
run ghz 80000
run ghz 160000

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