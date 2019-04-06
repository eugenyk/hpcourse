#!/bin/bash

# test.sh num_threads milliseconds values

test_input=$(seq $3)

# echo "Testing:\n" $test_input

result=$(echo $test_input | ./pthread-api $1 $2)
answer=$(echo $test_input | sed -e "s/ /+/g" | bc)

if [ "$result" -eq "$answer" ]; then
    echo OK
else
    echo FAILED
    echo "Result: $result"
    echo "Test: $test_input"
    exit 1
fi