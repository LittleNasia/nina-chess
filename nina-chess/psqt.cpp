#include "psqt.h"
#include "weights.h"

PSQT::PSQT(std::ifstream& weights_file):
	depth{ 0 },
	accumulator_context{},
	accumulators{},
	board_features{},
	moves_misc{}
{
	accumulator_context.accumulator_weights.SetWeights(weights_file);

	for (auto& accumulators : accumulators)
	{
		accumulators.SetWeights(accumulator_context.accumulator_weights);
	}
}
