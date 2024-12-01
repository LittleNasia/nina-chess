#!/usr/bin/env sh

commit_name=$(cat $1)
echo "Commit-msg hook: The commit name is " "$commit_name"

pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Debug /p:Platform=x64 /t:Rebuild" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Release /p:Platform=x64" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Test /p:Platform=x64" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Bench /p:Platform=x64" &&
python.exe ./test/run_benchmark.py "$commit_name"