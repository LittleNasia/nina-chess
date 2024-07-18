
template<Color side_to_move>
forceinline Board Board::MakeMove(const Move move) const
{
	return Board(evaluator, position::MakeMove<side_to_move>(position, move));
}