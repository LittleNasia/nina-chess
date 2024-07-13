import glob
import random
import os
import stat
import subprocess
import numpy as np
from scipy import stats
import shutil

files = glob.glob("C:\\Users\\Nasia and Ilusia\\Desktop\\test folder\\*.exe")
prev_version_clone_dest = "./test/clone"

def deleteReadOnlyFile(action, name, exc):
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

def prepareExecutables():
    if not os.path.exists(prev_version_clone_dest):
        os.mkdir(prev_version_clone_dest)
    subprocess.run("git clone . " + prev_version_clone_dest)
    subprocess.run("msbuild ./test/clone/nina-chess.sln /p:OutDir=../test/ /p:Configuration=Bench /p:Platform=x64 /p:TargetName=default")
    subprocess.run("msbuild nina-chess.sln /p:OutDir=../test/ /p:Configuration=Bench /p:Platform=x64 /p:TargetName=tested")

def cleanEnvironment():
    if os.path.exists(prev_version_clone_dest):
        shutil.rmtree(prev_version_clone_dest, onerror=deleteReadOnlyFile)
    #os.remove("./test/tested.exe")
    #os.remove("./test/default.exe")

cleanEnvironment()

prepareExecutables()

cleanEnvironment()

exit()




speeds = {}

for file in files:
    speeds[file] = []

defaultFile = [x for x in files if "default" in x][0]
print("Default", defaultFile)

betterFiles = []
worseFiles = []





run = 0
while len(files) > 1:
    run += 1
    print("Run number", run)
    random.shuffle(files)
    for file in files:
        txt_file_name = file + ".txt"
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

        alpha = 0.05
        if p_value < alpha:
            if meanCurrent > meanDefault:
                print(f"{file} is statistically better than default")
                betterFiles.append(file)
            else:
                print(f"{file} is statistically worse than default")
                worseFiles.append(file)
            files.remove(file)
        

    if run > 200:
        break
        
        

    
