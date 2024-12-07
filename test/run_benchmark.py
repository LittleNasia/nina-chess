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

def createBuildExecutableCommand(solutionPath, outputPath, outputExecutableName, target, isRebuild, parameters = ()):
    command  = f"msbuild {solutionPath} "
    command += f"/p:Platform=x64 "
    command += f"/p:Configuration={target} "

    if(outputPath != ""):
        command += f"/p:OutDir=\"{outputPath}/\" "
    if(outputExecutableName != ""):
        command += f"/p:TargetName={outputExecutableName} "
    if(isRebuild):
        command += f"/t:Rebuild "
    for parameter in parameters:
        command += f"/p:{parameter[0]}={parameter[1]} "

    print("Created msbuild command", command)
    return command
    

NUM_BENCHMARK_RUNS = 10

def deleteReadOnlyFile(action, name, exc):
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

def prepareExecutables():
    
    if not os.path.exists(PREV_VERSION_CLONE_DEST):
        os.mkdir(PREV_VERSION_CLONE_DEST)
    subprocess.run("git clone . " + PREV_VERSION_CLONE_DEST)
    subprocess.run(createBuildExecutableCommand(solutionPath="nina-chess.sln", outputPath=f"{cwd}/test", outputExecutableName=NEW_EXECUTABLE_NAME_WITHOUT_EXTENSION, target="Bench", isRebuild=False))
    subprocess.run(createBuildExecutableCommand(solutionPath="./test/clone/nina-chess.sln", outputPath=f"{cwd}/test", outputExecutableName=OLD_EXECUTABLE_NAME_WITHOUT_EXTENSION, target="Bench", isRebuild=True))

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
        for metric in bench_results[NEW_EXECUTABLE_PATH]:
            if bench_results[NEW_EXECUTABLE_PATH][metric] == bench_executables.EXECUTABLE_REJECTED:
                raise Exception(f"Changes rejected because of significant {metric} speed degradation")
    except:
        cleanEnvironment()
        raise

    cleanEnvironment()

def runTests():
    # run in release all tests
    subprocess.run(createBuildExecutableCommand(solutionPath="nina-chess.sln", outputPath="", outputExecutableName="", target="Test", isRebuild=True))
    subprocess.run(f'"{cwd}/x64/Test/nina-chess.exe"', check=True)
    # run in debug with assertions, with a node limit so it doesn't take forever
    subprocess.run(createBuildExecutableCommand(solutionPath="nina-chess.sln", outputPath="", outputExecutableName="", target="Test", isRebuild=True, 
                                                parameters=(("RunAssertions", "true"), ("TestPerftNodeLimit", 1000000))))
    subprocess.run(f'"{cwd}/x64/Test/nina-chess.exe"', check=True)

commitName = sys.argv[1]
if not "[NO-TEST]" in commitName:
    runTests()
if not "[NO-BENCH]" in commitName and not "[PRECISE-BENCH]" in commitName:
    runBenchmark()
if not "[NO-BENCH]" in commitName and "[PRECISE-BENCH]" in commitName:
    runBenchmark(200)