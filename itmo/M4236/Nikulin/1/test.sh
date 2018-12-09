#!/bin/bash

if [ $# -ne 1 ]; then
    echo "usage: $0 <main.out>"
    exit
fi

tmpfile=$(mktemp /tmp/pthread_test-script.XXXXXX)
declare -a left_numbers

upper_amount=10000
max_number=10000
half_number=$(( max_number / 2 ))
for i in $(seq 0 $upper_amount); do
    left_numbers[i]=$(( -$half_number + RANDOM % $max_number ))
done

echo "0 " > "$tmpfile"
for number in "${left_numbers[@]}"; do
    echo "$number" >> "$tmpfile"
done
for number in "${left_numbers[@]}"; do
    echo "$((-$number))" >> "$tmpfile"
done


for consumers_amount in 1 2 10 100 500 1000; do
    res=$(cat "$tmpfile" | "$1" "$consumers_amount" 150)
    if [ $res -ne 0 ]; then
        echo "test failed"
        exit
    fi
    echo "passed: amount consumers = $consumers_amount"
done
echo "all test passed"
rm "$tmpfile"


