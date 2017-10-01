#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

fail=false
for (( i = 0; i < 1000; i++ )); do
    result=`./main <<< "10 20 30 40 50 60 70 80"`
    if [[ $result != 360 ]]; then
        echo -e "${RED}TEST FAILED${NC}"
        echo "expected 360, found ${result}"
        fail=true
        break
    fi
done

if ! $fail; then
    echo -e "${GREEN}TEST PASSED${NC}"
fi
