#ifdef _GAMEGEN

#include "GameGeneration/game_generation.h"
#include <iostream>
#include <string>

inline GameGenerationSettings ParseArgs(const int argc, char* argv[])
{
	GameGenerationSettings settings;

	for (int argIndex = 1; argIndex + 1 < argc; argIndex += 2)
	{
		const std::string arg = argv[argIndex];
		const std::string value = argv[argIndex + 1];

		if (arg == "--threads")
			settings.NumThreads = std::stoi(value);
		else if (arg == "--games")
			settings.NumGames = std::stoi(value);
		else if (arg == "--depth")
			settings.MaxDepth = std::stoi(value);
		else if (arg == "--movetime")
			settings.MaxMovetime = std::stoi(value);
		else if (arg == "--opening-random-moves")
			settings.NumRandomMovesInOpening = std::stoi(value);
		else if (arg == "--max-random-moves")
			settings.MaxRandomMoves = std::stoi(value);
		else if (arg == "--random-chance")
			settings.RandomMoveChance = std::stof(value);
		else if (arg == "--random-chance-decay")
			settings.RandomMoveChanceDecay = std::stof(value);
		else if (arg == "--reply-random")
			settings.ReplyRandomWithRandom = (value == "true" || value == "1");
		else if (arg == "--max-length")
			settings.MaxGameLength = std::stoi(value);
		else if (arg == "--score-type")
			settings.ScoreBySearchEval = (value == "eval");
		else if (arg == "--output")
			settings.OutputFile = value;
		else if (arg == "--book")
			settings.BookFile = value;
		else if (arg == "--book-ply")
			settings.BookPly = std::stoi(value);
	}

	return settings;
}

int main(const int argc, char* argv[])
{
	try
	{
		const GameGenerationSettings settings = ParseArgs(argc, argv);
		RunGameGeneration(settings);
	}
	catch (const std::exception& exception)
	{
		std::cerr << "Error: " << exception.what() << std::endl;
		return 1;
	}
	return 0;
}

#endif
