import numpy as np
from scipy import stats
import subprocess
import glob
import random
import os


MIN_RUNS = 5

EXECUTABLE_REJECTED = 0
EXECUTABLE_NOT_REJECTED_NOR_ACCEPTED = 1
EXECUTABLE_ACCEPTED = 2

def benchmarkFiles(runs, defaultFile, *filenames):
    speeds = {}
    statuses = {}

    if runs < MIN_RUNS:
        raise Exception(f"The number of bench runs cannot be lower than MIN_RUNS ({MIN_RUNS})")

    files = [ defaultFile ]
    for file in filenames:
        files.append(file)
        
    for file in files:
        speeds[file] = []
        statuses[file] = EXECUTABLE_NOT_REJECTED_NOR_ACCEPTED

    run = 0
    while len(files) > 1:
        run += 1
        print("Run number", run)
        random.shuffle(files)

        for file in files:
            cmd = f"\"{file}\""
            print("\tRunning", cmd)
            speed = int(subprocess.run(cmd, stdout=subprocess.PIPE).stdout.splitlines()[0])
            speeds[file].append(speed)
            print("\t\tachieved speed", speed)


        defaultFileScores = speeds[defaultFile]
        for file in files:
            if file == defaultFile:
                continue

            if run < 5:
                continue

            if len(file) <= 1:
                break

            currFileScores = speeds[file]

            meanCurrent = np.mean(currFileScores)
            meanDefault = np.mean(defaultFileScores)
            std1 = np.std(currFileScores, ddof=1)
            std2 = np.std(defaultFileScores, ddof=1)

            t_stat, p_value = stats.ttest_ind(currFileScores, defaultFileScores, equal_var=False)

            print(f"\t{file}: Mean = {meanCurrent:.2f}, Std = {std1:.2f}, N = {len(currFileScores)}")
            print(f"\t{defaultFile}: Mean = {meanDefault:.2f}, Std = {std2:.2f}, N = {len(defaultFileScores)}")
            print(f"\tT-statistic = {t_stat:.2f}, P-value = {p_value:.2f}")

            alpha = 0.02
            if p_value < alpha:
                if meanCurrent > meanDefault:
                    statuses[file] = EXECUTABLE_ACCEPTED
                else:
                    statuses[file] = EXECUTABLE_REJECTED
                files.remove(file)
            

        if run >= runs:
            break

    return statuses
