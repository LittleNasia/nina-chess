import numpy as np
from scipy import stats
import subprocess
import glob
import random
import os
import statistics


MIN_RUNS = 5

EXECUTABLE_REJECTED = 0
EXECUTABLE_NOT_REJECTED_NOR_ACCEPTED = 1
EXECUTABLE_ACCEPTED = 2

FIRST_IS_BETTER = 1
SECOND_IS_BETTER = -1
NO_MEANINGFUL_DIFFERENCE = 0

ALPHA = 0.02

def isExecutableRatingFinished(executableStatus):
    for metric in executableStatus:
        if executableStatus[metric] == EXECUTABLE_NOT_REJECTED_NOR_ACCEPTED:
            return False
    return True

def compareSpeeds(firstName, secondName, speedsFirst, speedsSecond, alpha):
    meanFirst = np.mean(speedsFirst)
    meanSecond = np.mean(speedsSecond)
    stdFirst = np.std(speedsFirst, ddof=1)
    stdSecond = np.std(speedsSecond, ddof=1)

    t_stat, p_value = stats.ttest_ind(speedsFirst, speedsSecond, equal_var=False)

    print(f"\t{firstName}: Mean = {meanFirst:.2f}, Std = {stdFirst:.2f}, N = {len(speedsFirst)}")
    print(f"\t{secondName}: Mean = {meanSecond:.2f}, Std = {stdSecond:.2f}, N = {len(speedsSecond)}")
    print(f"\tT-statistic = {t_stat:.2f}, P-value = {p_value:.2f}")

    if p_value < alpha:
        if meanFirst > meanSecond:
            return FIRST_IS_BETTER;
        else:
            return SECOND_IS_BETTER;
    return NO_MEANINGFUL_DIFFERENCE



benchmarkedMetrics = ["perft", "search"]


def benchmarkFiles(runs, defaultFile, filenames):
    speeds = {}
    statuses = {}

    if runs < MIN_RUNS:
        raise Exception(f"The number of bench runs cannot be lower than MIN_RUNS ({MIN_RUNS})")

    files = [ defaultFile ]
    for file in filenames:
        files.append(file)

    for file in files:
        speeds[file] = {}
        statuses[file] = {}
        for metric in benchmarkedMetrics:
            speeds[file][metric] = []
            statuses[file][metric] = EXECUTABLE_NOT_REJECTED_NOR_ACCEPTED

    run = 0
    while len(files) > 1:
        run += 1
        print("Run number", run)
        random.shuffle(files)

        for file in files:
            cmd = f"\"{file}\""
            print("\tRunning", cmd)
            benchProcess = subprocess.run(cmd, stdout=subprocess.PIPE)
            benchOutput = benchProcess.stdout.splitlines()
            for metricIndex in range(len(benchmarkedMetrics)):
                metric = benchmarkedMetrics[metricIndex]
                metricResult = int(benchOutput[metricIndex])
                speeds[file][metric].append(metricResult);

                avg = statistics.fmean(speeds[file][metric])
                print(f"\t\tachieved speed in metric {metric}", speeds[file][metric], f" average value is {avg}")


        for file in files:
            if file == defaultFile:
                continue
            if run < MIN_RUNS:
                break
            if len(file) <= 1:
                break


            for metricIndex in range(len(benchmarkedMetrics)):
                metric = benchmarkedMetrics[metricIndex]

                defaultScores = speeds[defaultFile][metric]
                comparedScores = speeds[file][metric]

                compareResult = compareSpeeds(defaultFile, file, defaultScores, comparedScores, ALPHA)

                if(compareResult == FIRST_IS_BETTER):
                    statuses[file][metric] = EXECUTABLE_REJECTED
                elif(compareResult == SECOND_IS_BETTER):
                    statuses[file][metric] = EXECUTABLE_ACCEPTED
                else:
                    statuses[file][metric] = EXECUTABLE_NOT_REJECTED_NOR_ACCEPTED



        if run >= runs:
            break

    return statuses
