import subprocess
import os
import glob


dirs = glob.glob("commit-*")
dirs = sorted(dirs, key=lambda x: int(x.split("-")[1]))
base_path = os.getcwd()

for dir in dirs:
    full_dir_path = os.path.join(base_path, dir)
    os.chdir(full_dir_path)

    output = subprocess.getoutput("make .PHONY")
    output = output.split("\n")[-2]
    if output == "OK":
        print(f"\033[92mTest for {dir.replace('-', ' ')} passed\033[m")
    else:
        print(f"\033[91mTest for {dir.replace('-', ' ')} failed\033[m")