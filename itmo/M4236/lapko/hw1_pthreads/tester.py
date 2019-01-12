import subprocess as sp

n_consumers = 10
max_delay = 1  # ms
data_in = b'1 ' * 4200 + b'\n'

cmd = ["cmake-build-debug/hw1_pthreads", str(n_consumers), str(max_delay)]
p = sp.Popen(cmd, stdout=sp.PIPE, stdin=sp.PIPE, stderr=sp.STDOUT)
grep_stdout = p.communicate(input=data_in)[0]
print(grep_stdout.decode())
