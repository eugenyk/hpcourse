#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

fail=false
for (( i = 0; i < 1000; i++ )); do
    result=`./main <<< "10 20 30 40"`
    if [[ $result != 100 ]]; then
        echo -e "${RED}TEST FAILED${NC}"
        echo "expected 100, found ${result}"
        fail=true
        break
    fi
done

if ! $fail; then
    echo -e "${GREEN}TEST PASSED${NC}"
fi
