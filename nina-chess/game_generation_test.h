#pragma once
#include "game_generation.h"
#include <cstdio>
#include <filesystem>
#include <iostream>

inline bool TestGameGeneration()
{
	constexpr int NUM_GAMES = 10;
	const std::string testOutputFile = "test_gamegen.bin";

	std::cout << "Running game generation test: " << NUM_GAMES << " games\n";

	GameGenerationSettings settings;
	settings.NumGames = NUM_GAMES;
	settings.NumThreads = 1;
	settings.MaxDepth = 2;
	settings.NumRandomMovesInOpening = 4;
	settings.MaxGameLength = 50;
	settings.OutputFile = testOutputFile;

	try
	{
		RunGameGeneration(settings);
	}
	catch (const std::exception& ex)
	{
		std::cout << "Game generation failed: " << ex.what() << std::endl;
		std::remove(testOutputFile.c_str());
		return false;
	}

	if (!std::filesystem::exists(testOutputFile))
	{
		std::cout << "Game generation test failed: output file not found" << std::endl;
		return false;
	}

	const auto fileSize = std::filesystem::file_size(testOutputFile);
	std::remove(testOutputFile.c_str());

	if (fileSize == 0)
	{
		std::cout << "Game generation test failed: output file is empty" << std::endl;
		return false;
	}

	if (fileSize % sizeof(PositionEntry) != 0)
	{
		std::cout << "Game generation test failed: file size " << fileSize
			<< " is not a multiple of entry size " << sizeof(PositionEntry) << std::endl;
		return false;
	}

	const auto numPositions = fileSize / sizeof(PositionEntry);
	std::cout << "Game generation test passed: " << numPositions << " positions in " << NUM_GAMES << " games" << std::endl;
	return true;
}
