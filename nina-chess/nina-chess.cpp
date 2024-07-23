#include <iostream>
#include <random>

#include "move_gen.h"
#include "perft.h"
#include "position.h"
#include "search.h"
#include "targets.h"
#include "utils.h"

#if _UCI
int main()
{
	// globals
	std::unique_ptr<Evaluator> evaluator = std::make_unique<Evaluator>();
    TranspositionTable tt(128);

	Position position;// = position::ParseFen("4Qnk1/p4ppp/8/7n/2P5/2B1P3/PP3q1P/6RK b - - 0 1");
    const size_t depth = 10;

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	const auto& result = start_search(position, depth, tt, *evaluator);
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

	for (int pv_move_index = 0; pv_move_index < result.pv_length; pv_move_index++)
	{
		std::cout << "pv move: " << pv_move_index + 1 << " " << square_names[bit_index(result.pv[pv_move_index].from())] << " " << square_names[bit_index(result.pv[pv_move_index].to())] << "\n";
	}
	std::cout << "score " << result.score << "\n";
    std::cout << "nodes " << result.nodes << "\n";
	std::cout << "nps " << size_t(result.nodes / duration.count()) << "\n";
}
#endif
