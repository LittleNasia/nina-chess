import glob
import random
import os
import stat
import subprocess
import numpy as np
from scipy import stats
import shutil
import bench_executables
import sys

cwd = os.getcwd()

PREV_VERSION_CLONE_DEST = "./test/clone"

NEW_EXECUTABLE_NAME_WITHOUT_EXTENSION = "tested"
OLD_EXECUTABLE_NAME_WITHOUT_EXTENSION = "default"

NEW_EXECUTABLE_NAME = f"{NEW_EXECUTABLE_NAME_WITHOUT_EXTENSION}.exe"
OLD_EXECUTABLE_NAME = f"{OLD_EXECUTABLE_NAME_WITHOUT_EXTENSION}.exe"
NEW_EXECUTABLE_PDB_NAME = f"{NEW_EXECUTABLE_NAME_WITHOUT_EXTENSION}.exe"
OLD_EXECUTABLE_PDB_NAME = f"{OLD_EXECUTABLE_NAME_WITHOUT_EXTENSION}.exe"

NEW_EXECUTABLE_PATH = f"./test/{NEW_EXECUTABLE_NAME}"
OLD_EXECUTABLE_PATH = f"./test/{OLD_EXECUTABLE_NAME}"

NUM_BENCHMARK_RUNS = 10

def deleteReadOnlyFile(action, name, exc):
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

def prepareExecutables():
    
    if not os.path.exists(PREV_VERSION_CLONE_DEST):
        os.mkdir(PREV_VERSION_CLONE_DEST)
    subprocess.run("git clone . " + PREV_VERSION_CLONE_DEST)
    subprocess.run(f"msbuild nina-chess.sln /p:OutDir=\"{cwd}/test/\" /p:Configuration=Bench /p:Platform=x64 /p:TargetName={NEW_EXECUTABLE_NAME_WITHOUT_EXTENSION}")
    subprocess.run(f"msbuild ./test/clone/nina-chess.sln /p:OutDir=\"{cwd}/test/\" /p:Configuration=Bench /p:Platform=x64 /p:TargetName={OLD_EXECUTABLE_NAME_WITHOUT_EXTENSION}")

def cleanEnvironment():
    if os.path.exists(PREV_VERSION_CLONE_DEST):
        shutil.rmtree(PREV_VERSION_CLONE_DEST, onerror=deleteReadOnlyFile)
    files = os.listdir(f"{cwd}/test/")
    for file in files:
        if file.endswith(".exe") or file.endswith(".pdb"):
            os.remove(f"{cwd}/test/{file}")

def runBenchmark(numRuns = NUM_BENCHMARK_RUNS):
    cleanEnvironment()
    prepareExecutables()

    try:
        bench_results = bench_executables.benchmarkFiles(NUM_BENCHMARK_RUNS, OLD_EXECUTABLE_PATH, NEW_EXECUTABLE_PATH)
        if bench_results[NEW_EXECUTABLE_PATH] == bench_executables.EXECUTABLE_REJECTED:
            raise Exception("Changes rejected because of significant move generation speed degradation")
    except:
        cleanEnvironment()
        raise

    cleanEnvironment()

def runTests():
    subprocess.run(f'"{cwd}/x64/Test/nina-chess.exe"', check=True)


commitName = sys.argv[1]
if not "[NO-TEST]" in commitName:
    runTests()
if not "[NO-BENCH]" in commitName:
    runBenchmark()
if "[PRECISE-BENCH]" in commitName:
    runBenchmark(100)

    
