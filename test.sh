#!/bin/bash

test_count=0
TLC="build/src/tlc"

function run_test() {
    for entry in $1/*.tl
    do
    	name=`basename $entry .tl`
    	EXIT=0
    	
    	echo ""
    	echo $name
    	
    	#if [[ $name == "op_pred" ]] ; then
    	#    valgrind $TLC $entry -o $name
    	#fi
    	$TLC $entry -o $name
    	#echo "$TLC $entry -o $name"
    	./$name > /tmp/$name.actual
    	CODE=$?
    	cat $1/out/$name.out > /tmp/$name.exp
    	
    	diff -wB /tmp/$name.actual /tmp/$name.exp
    	if [[ $? == 0 ]] ; then
    	    echo $CODE > /tmp/$name.actual
    	    cat $1/out/$name.code > /tmp/$name.exp
    	    diff -wB /tmp/$name.actual /tmp/$name.exp
    	    if [[ $? == 0 ]] ; then
    	        echo "Pass"
    	    else
    	        EXIT=1
    	        echo "Fail: Code"
    	        echo "Expected: `cat $1/out/$name.code`"
    	        echo "Actual: $CODE"
                echo ""
    	    fi
    	else
    	    EXIT=1
    	    echo "Fail: Output"
    	    echo "Expected:"
    	    echo `cat /tmp/$name.exp`
    	    echo "Actual:"
    	    echo `cat /tmp/$name.actual`
    	    echo ""
    	fi
    	
    	rm ./$name
    	rm /tmp/$name.o
    	rm /tmp/$name.asm
    	rm /tmp/$name.actual
    	rm /tmp/$name.exp
    	
    	test_count=$((test_count+1))
    	
    	if [[ $EXIT = 1 ]] ; then
    	    exit 1
    	fi
    done
}

flags=""

echo "Running all tests..."
echo ""

run_test 'test/basic'
run_test 'test/syntax'
run_test 'test/cond'
run_test 'test/loop'
run_test 'test/array'
run_test 'test/func'
run_test 'test/str'
run_test 'test/struct'

echo ""
echo "$test_count tests passed successfully."
echo "Done"

