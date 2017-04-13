#!/bin/bash

N=1000
ANS=$((N * (N + 1) / 2))

input=$(seq -s " " $N)
output=`../bin/main <<EOF
$input
EOF`

if (( $output != $ANS ));
then
    echo -e "FAIL\nExpected value: $ANS\nActual value: $output"
    exit 1
fi

echo -e "PASS"
