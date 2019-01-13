#!/usr/bin/env python3
from subprocess import Popen, PIPE
import os, time

def run_threads(args, inp):
    p = Popen(args, stdin=PIPE, stdout=PIPE)
    time.sleep(2)
    output = p.communicate(str.encode(inp))[0]
    return output.decode("utf-8")
	

if __name__ == '__main__':
    assert run_threads(["./main", "12", "13"], '1\n2\n') == "3\n"
    print("test 1 passed!")
    assert run_threads(["./main", "50", "20"], '1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n') == "55\n"
    print("test 2 passed!")
	
	