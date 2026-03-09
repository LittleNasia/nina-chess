#!/usr/bin/env sh

commit_name=$(cat $1)
echo "Commit-msg hook: The commit name is " "$commit_name"

pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Debug /p:Platform=x64-SSE3 /t:Rebuild" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Debug /p:Platform=x64-AVX2" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Debug /p:Platform=x64-AVX512" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Release /p:Platform=x64-SSE3" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Release /p:Platform=x64-AVX2" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Release /p:Platform=x64-AVX512" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Test /p:Platform=x64-SSE3" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Test /p:Platform=x64-AVX2" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Test /p:Platform=x64-AVX512" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Bench /p:Platform=x64-SSE3" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Bench /p:Platform=x64-AVX2" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Bench /p:Platform=x64-AVX512" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Gamegen /p:Platform=x64-SSE3" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Gamegen /p:Platform=x64-AVX2" &&
pwsh.exe -Command "msbuild nina-chess.sln /p:Configuration=Gamegen /p:Platform=x64-AVX512" &&
python.exe ./test/run_benchmark.py "$commit_name"