import subprocess
import os

from random import randint

# little hack: http://stackoverflow.com/questions/287871/print-in-terminal-with-colors-using-python
class bcolors:
    HEADER      = '\033[95m'
    OKBLUE      = '\033[94m'
    OKGREEN     = '\033[92m'
    WARNING     = '\033[93m'
    FAIL        = '\033[91m'
    ENDC        = '\033[0m'
    BOLD        = '\033[1m'
    UNDERLINE   = '\033[4m'


BIN_DIR     = 'bin/'
BIN_NAME    = 'pthread_task'
BIN_PATH    = os.path.join(BIN_DIR, BIN_NAME)

sequences = [
            ([], 'empty'),
            ([1,2,3,4,5], 'simple'),
            ([1,2,3,4,5,-4,-3,-2,-1], 'with negative'),
            ([randint(0, 100) for _ in range(1000)], 'random')
            ]

def run_test():
    for data, test_name in sequences:
        answer = sum(data)

        popen = subprocess.Popen([BIN_PATH], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
        output, err = popen.communicate(input=' '.join(map(str, data)).encode())
        output = int(output)

        print('Test name: {} {} {}'.format(bcolors.OKBLUE, test_name, bcolors.ENDC))
        print('Expected answer: {}'.format(answer))
        if answer == output:
            print('Answer: {}*** CORRECT ***{}'.format(bcolors.OKGREEN, bcolors.ENDC))
        else:
            print('Answer: {}*** INCORRECT ***{}'.format(bcolors.FAIL, bcolors.ENDC))
            print('Expected: {}, Got: {}'.format(answer, output))

        print('-'*20)


if __name__ == '__main__':
    run_test()