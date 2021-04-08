import subprocess
import os
import glob
import argparse


def run_parallel_test(paths):
    outs = [subprocess.Popen(f'cd {path}; make .PHONY', 
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             shell=True) for path in paths]

    for out in outs:
        out.wait()

    for j, out in enumerate(outs):
        output = str(out.stdout.read())
        if "tests passed!" not in output:
            print(f"\033[91mTest for {os.path.basename(paths[j])} failed!\033[m")
        else:
            print(f"\033[92mTest for {os.path.basename(paths[j])} passed!\033[m")


parser = argparse.ArgumentParser(description="Chibicc commit tests")
parser.add_argument("--num_parallel", type=int, default=4, help="Number of parallel processes to run tests in")
args = parser.parse_args()

dirs = glob.glob("commit-*")
dirs = sorted(dirs, key=lambda x: int(x.split("-")[1]))
base_path = os.getcwd()
executed_tests = 0
NUM_PARALLEL_TESTS = args.num_parallel

for i in range(0, len(dirs), NUM_PARALLEL_TESTS):
    try:
        paths = [os.path.join(base_path, dirs[i+j]) for j in range(NUM_PARALLEL_TESTS)]
    except:
        break

    executed_tests += len(paths)
    
    run_parallel_test(paths=paths)

remaining_paths = dirs[executed_tests:] if executed_tests != len(dirs) else []
run_parallel_test(remaining_paths)