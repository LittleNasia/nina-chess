#pragma once
#include "utils.h"

#include "attacks.h"
#include "bitmasks.h"
#include "move_list.h"
#include "position.h"

forceinline void FillPinmask(const size_t square, Bitboard& pinmask, Bitboard pinners)
{
	// Evil branch
	while (pinners)
	{
		const Bitboard pinner = PopBit(pinners);
		pinmask |= PIN_BETWEEN_TABLE[square][BitIndex(pinner)];
	}
}

forceinline void FillCheckmask(const size_t square, Bitboard& checkmask, Bitboard checkers)
{
	// More evil branches
	while (checkers)
	{
		const Bitboard checker = PopBit(checkers);
		checkmask |= PIN_BETWEEN_TABLE[square][BitIndex(checker)] | (1ULL<<square);
	}
}
template<Color color>
forceinline constexpr Bitboard GetLegalPawnCapturesLeft(const Bitboard pawns, const Bitboard bishopPinmask, const Bitboard rookPinmask, const Bitboard enemyPieces)
{
	const Bitboard pinmask = (bishopPinmask | rookPinmask);
	const Bitboard unpinnedPawns = pawns & ~pinmask;
	const Bitboard pinnedPawns = pawns & pinmask;
	const Bitboard unpinnedPawnCaptures = GetPawnsLeftAttacks<color>(unpinnedPawns) & enemyPieces;
	// pawns pinned by rooks can't capture, pawns pinned by bishops can only capture in the direction of a bishop
	const Bitboard pinnedPawnCaptures = (GetPawnsLeftAttacks<color>(pinnedPawns & ~rookPinmask) & bishopPinmask) & enemyPieces;
	return unpinnedPawnCaptures | pinnedPawnCaptures;
}

template<Color color>
forceinline constexpr Bitboard GetLegalPawnCapturesRight(const Bitboard pawns, const Bitboard bishopPinmask, const Bitboard rookPinmask, const Bitboard enemyPieces)
{
	const Bitboard pinmask = (bishopPinmask | rookPinmask);
	const Bitboard unpinnedPawns = pawns & ~pinmask;
	const Bitboard pinnedPawns = pawns & pinmask;
	const Bitboard unpinnedPawnCaptures = GetPawnsRightAttacks<color>(unpinnedPawns) & enemyPieces;
	// pawns pinned by rooks can't capture, pawns pinned by bishops can only capture in the direction of a bishop
	const Bitboard pinnedPawnCaptures = (GetPawnsRightAttacks<color>(pinnedPawns & ~rookPinmask) & bishopPinmask) & enemyPieces;
	return unpinnedPawnCaptures | pinnedPawnCaptures;
}

template<Color color>
forceinline constexpr Bitboard GetLegalPawnAdvances(const Bitboard pawns, const Bitboard bishopPinmask, const Bitboard rookPinmask, const Bitboard occupied)
{
	const Bitboard pinmask = bishopPinmask | rookPinmask;
	const Bitboard unpinnedPawns = pawns & ~pinmask;
	const Bitboard pinnedPawns = pawns & pinmask;
	const Bitboard unpinnedPawnAdvances = GetPawnAdvances<color>(unpinnedPawns) & ~occupied;
	const Bitboard pinnedPawnAdvances = (GetPawnAdvances<color>(pinnedPawns & ~bishopPinmask) & rookPinmask) & ~occupied;
	return unpinnedPawnAdvances | pinnedPawnAdvances;
}

template<Color color>
forceinline constexpr Bitboard GetPawnDoubleAdvances(const Bitboard singleAdvancePawnMoves, const Bitboard occupied)
{
	return GetPawnAdvances<color>(singleAdvancePawnMoves & GetDoubleAdvancesCandidates<color>()) & ~occupied;
}

forceinline constexpr Bitboard GetKingMoves(const size_t kingIndex, const Bitboard attackedSquares)
{
	return KING_MOVE_BITMASKS[kingIndex] & ~attackedSquares;
}

template<PieceType pieceType, Color color, bool isCastling = false, bool isEnPassant = false>
forceinline void WriteMoves(MoveList& moveList, Bitboard movesMask, const uint32_t pieceIndex)
{
	constexpr PieceType movingPieceType = pieceType;
	while (movesMask)
	{
		const uint32_t moveTargetIndex = PopBitAndGetIndex(movesMask);
		if constexpr (isCastling)
			moveList.PushMove({ pieceIndex, moveTargetIndex, movingPieceType, PIECE_TYPE_NONE, isCastling });
		else if constexpr (isEnPassant)
			moveList.PushMove({ pieceIndex, moveTargetIndex, movingPieceType, PIECE_TYPE_NONE, isCastling, isEnPassant });
		else
			moveList.PushMove({ pieceIndex, moveTargetIndex, movingPieceType });
	}
	moveList.MoveListMisc.PieceMoves[pieceType] |= movesMask;
}

template<Color color>
forceinline void WritePawnMoves(MoveList& moveList, const Bitboard leftPawnCaptures, const Bitboard rightPawnCaptures,
	const Bitboard legalPawnAdvances, const Bitboard pawnDoubleAdvances, Bitboard pawns)
{
	ValidateColor<color>();
	constexpr PieceType movingPieceType = PAWN;
	while (pawns)
	{
		const auto pawn = PopBit(pawns);
		const auto pawnIndex = BitIndex(pawn);
		Bitboard currentPawnMoves = 0ULL;
		currentPawnMoves |= GetPawnsLeftAttacks<color>(pawn) & leftPawnCaptures;
		currentPawnMoves |= GetPawnsRightAttacks<color>(pawn) & rightPawnCaptures;
		currentPawnMoves |= GetPawnAdvances<color>(pawn) & legalPawnAdvances;
		currentPawnMoves |= GetDoubleAdvances<color>(pawn) & pawnDoubleAdvances;
		while (currentPawnMoves)
		{
			const Bitboard move = PopBit(currentPawnMoves);
			const uint32_t moveIndex = BitIndex(move);

			if (move & PromotionRankBitmask<color>())
			{
				for (const auto promotion_piece : PROMOTION_PIECE_TYPES)
				{
					moveList.PushMove({ pawnIndex, moveIndex, movingPieceType, promotion_piece });
				}
			}
			else
				moveList.PushMove({ pawnIndex, moveIndex, movingPieceType });
		}
		moveList.MoveListMisc.PieceMoves[movingPieceType] |= currentPawnMoves;
	}
}  

template<Color color, bool isCheck, bool isUnblockableCheck>
forceinline void WriteSliderMoves(MoveList& moveList, Bitboard movableBishops, Bitboard movableRooks, Bitboard queens, const Bitboard occupied, const Bitboard allies,
	const Bitboard bishopPinmask, const Bitboard rookPinmask, const Bitboard checkers, const Bitboard bishopCheckmask, const Bitboard rookCheckmask)
{
	while (movableBishops)
	{
		const Bitboard currentBishop = PopBit(movableBishops);
		Bitboard currentBishopMoves = GetSingleBishopAttacks(currentBishop, occupied);
		if constexpr (isCheck && isUnblockableCheck)
		{
			currentBishopMoves &= checkers;
		}
		else if constexpr (isCheck && !isUnblockableCheck)
		{
			currentBishopMoves &= (rookCheckmask | bishopCheckmask);
		}
		if(currentBishop & bishopPinmask)
			currentBishopMoves &= bishopPinmask;
		currentBishopMoves &= ~allies;
		if(currentBishopMoves)
			WriteMoves<BISHOP, color>(moveList, currentBishopMoves, BitIndex(currentBishop));
		moveList.MoveListMisc.PieceMoves[BISHOP] |= currentBishopMoves;
	}
	while (movableRooks)
	{
		const Bitboard currentRook = PopBit(movableRooks);
		Bitboard currentRookMoves = GetSingleRookAttacks(currentRook, occupied);
		if constexpr (isCheck && isUnblockableCheck)
		{
			currentRookMoves &= checkers;
		}
		else if constexpr (isCheck && !isUnblockableCheck)
		{
			currentRookMoves &= (rookCheckmask | bishopCheckmask);
		}
		if (currentRook & rookPinmask)
			currentRookMoves &= rookPinmask;
		currentRookMoves &= ~allies;
		if(currentRookMoves)
			WriteMoves<ROOK, color>(moveList, currentRookMoves, BitIndex(currentRook));
		moveList.MoveListMisc.PieceMoves[ROOK] |= currentRookMoves;
	}
	while (queens)
	{
		const Bitboard currentQueen = PopBit(queens);
		const Bitboard currentQueenRookMoves = GetSingleRookAttacks(currentQueen, occupied);
		const Bitboard currentQueenBishopMoves = GetSingleBishopAttacks(currentQueen, occupied);
		Bitboard currentQueenMoves = currentQueenRookMoves | currentQueenBishopMoves;

		if(currentQueen & rookPinmask)
			currentQueenMoves &= (rookPinmask & currentQueenRookMoves);
		if (currentQueen & bishopPinmask)
			currentQueenMoves &= (bishopPinmask & currentQueenBishopMoves);
		currentQueenMoves &= ~allies;
		if constexpr (isCheck && isUnblockableCheck)
		{
			currentQueenMoves &= checkers;
		}
		else if constexpr (isCheck && !isUnblockableCheck)
		{
			currentQueenMoves &= (rookCheckmask | bishopCheckmask);
		}
		if (currentQueenMoves)
			WriteMoves<QUEEN, color>(moveList, currentQueenMoves, BitIndex(currentQueen));
		moveList.MoveListMisc.PieceMoves[QUEEN] |= currentQueenMoves;
	}
}

template<Color color, bool isCheck, bool isUnblockableCheck>
forceinline void WriteKnightMoves(MoveList& moveList, Bitboard movableKnights, const Bitboard allies,
	const Bitboard checkers, const Bitboard bishopCheckmask, const Bitboard rookCheckmask)
{
	constexpr PieceType movingPieceType = KNIGHT;
	while (movableKnights)
	{
		const uint32_t KnightIndex = PopBitAndGetIndex(movableKnights);
		Bitboard currentKnightMoves = KNIGHT_MOVE_BITMASKS[KnightIndex];
		if constexpr (isCheck && isUnblockableCheck)
		{
			currentKnightMoves &= checkers;
		}
		else if constexpr (isCheck && !isUnblockableCheck)
		{
			currentKnightMoves &= (bishopCheckmask | rookCheckmask);
		}
		currentKnightMoves &= ~allies;
		
		while (currentKnightMoves)
		{
			const Bitboard move = PopBit(currentKnightMoves);
			const uint32_t moveTarget = BitIndex(move);

			moveList.PushMove({ KnightIndex, moveTarget, movingPieceType });
		}
		moveList.MoveListMisc.PieceMoves[movingPieceType] |= currentKnightMoves;
	}
}

template<Color color, size_t castlingPermissions, bool hasEP>
forceinline MoveList& GenerateMoves(MoveList& moveList, const Position& position)
{
	moveList.Reset();
	moveList.SetHashOfPosition(position.Hash);

	constexpr auto oppositeColor = GetOppositeColor<color>();
	const auto& currPieces = position.GetSide<color>();
	const auto& oppositePieces = position.GetSide<oppositeColor>();
	const auto& king = currPieces.King;
	const auto& kingIndex = BitIndex(currPieces.King);
	const Bitboard bishopXrayFromKing = BISHOP_XRAY_BITMASKS[kingIndex];
	const Bitboard rookXrayFromKing = ROOK_XRAY_BITMASKS[kingIndex];
	// king can't move to these squares
	const Bitboard attackedSquares = GetAllAttacks<oppositeColor>(oppositePieces, position.OccupiedBitmask ^ king);

	Bitboard bishopCheckers = 0ULL;
	Bitboard bishopPinners = 0ULL;
	Bitboard bishopPinmask = 0ULL;
	Bitboard bishopCheckmask = 0ULL;

	Bitboard rookCheckers = 0ULL;
	Bitboard rookPinners = 0ULL;
	Bitboard rookPinmask = 0ULL;
	Bitboard rookCheckmask = 0ULL;

	const Bitboard knightCheckers = KNIGHT_MOVE_BITMASKS[kingIndex] & oppositePieces.Knights;
	const Bitboard pawnCheckers = GetAllPawnAttacks<color>(king) & oppositePieces.Pawns;

	const Bitboard enemyRookQueen = oppositePieces.Rooks | oppositePieces.Queens;
	const Bitboard enemyBishopQueen = oppositePieces.Bishops | oppositePieces.Queens;

	// quick check to save computation in most cases
	// a bishop can't pin a piece if it's not on the same diagonal with king etc.
	if (bishopXrayFromKing & enemyBishopQueen)
	{
		// bitboard consisting of all pieces attacked by the king if it was a bishop
		const Bitboard bishopKingAttacks = GetSingleBishopAttacks(king, position.OccupiedBitmask);
		bishopCheckers |= enemyBishopQueen & bishopKingAttacks;
		FillCheckmask(kingIndex, bishopCheckmask, bishopCheckers);
		const Bitboard attackedPiecesByBishopKing = position.OccupiedBitmask & bishopKingAttacks;
		const Bitboard occupiedBitmaskWithoutBishopKingVictims = position.OccupiedBitmask ^ attackedPiecesByBishopKing;
		const Bitboard piecesBehindBishopKingAttackedPieces = GetSingleBishopAttacks(king, occupiedBitmaskWithoutBishopKingVictims) & occupiedBitmaskWithoutBishopKingVictims;
		bishopPinners |= piecesBehindBishopKingAttackedPieces & enemyBishopQueen;
		FillPinmask(kingIndex, bishopPinmask, bishopPinners);
	}
	if (rookXrayFromKing & enemyRookQueen)
	{
		const Bitboard rookKingAttacks = GetSingleRookAttacks(king, position.OccupiedBitmask);
		rookCheckers |= enemyRookQueen & rookKingAttacks;
		FillCheckmask(kingIndex, rookCheckmask, rookCheckers);
		const Bitboard attackedPiecesByRookKing = position.OccupiedBitmask & rookKingAttacks;
		const Bitboard occupiedBitmaskWithoutRookKingVictims = position.OccupiedBitmask ^ attackedPiecesByRookKing;
		const Bitboard piecesBehindRookKingAttackedPieces = GetSingleRookAttacks(king, occupiedBitmaskWithoutRookKingVictims) & occupiedBitmaskWithoutRookKingVictims;
		rookPinners |= piecesBehindRookKingAttackedPieces & enemyRookQueen;
		FillPinmask(kingIndex, rookPinmask, rookPinners);
	}

	moveList.MoveListMisc.Checkers = rookCheckers | bishopCheckers | knightCheckers | pawnCheckers;
	moveList.MoveListMisc.Pinners = rookPinners | bishopPinners;
	moveList.MoveListMisc.Checkmask = rookCheckmask | bishopCheckmask;
	moveList.MoveListMisc.Pinmask = rookPinmask | bishopPinmask;
	moveList.MoveListMisc.AttackedSquares = attackedSquares;

	const uint32_t numCheckers = Popcnt(rookCheckers | bishopCheckers | knightCheckers | pawnCheckers);

	// king can always move, unless she can't
	const Bitboard legalKingMoves = GetKingMoves(kingIndex, attackedSquares) & ~currPieces.Pieces;
	WriteMoves<KING, color>(moveList, legalKingMoves, kingIndex);
	if (numCheckers > 1)
	{
		// double check. The only legal moves are king moves to run away from check, can't block it 
		return moveList;
	}
	// single or no checks
	// pinned pieces can't move in directions they can't move in
	// in check by sliders they can only capture the checking piece or block it 
	// in check by knights or pawns they can only capture the piece 
	Bitboard pawnLeftCaptures = GetLegalPawnCapturesLeft<color>(currPieces.Pawns, bishopPinmask, rookPinmask, oppositePieces.Pieces);
	Bitboard pawnRightCaptures = GetLegalPawnCapturesRight<color>(currPieces.Pawns, bishopPinmask, rookPinmask, oppositePieces.Pieces);
	Bitboard legalPawnAdvances = GetLegalPawnAdvances<color>(currPieces.Pawns, bishopPinmask, rookPinmask, position.OccupiedBitmask);
	Bitboard legalPawnDoubleAdvances = legalPawnAdvances & GetDoubleAdvancesCandidates<color>();
	legalPawnDoubleAdvances = GetPawnAdvances<color>(legalPawnDoubleAdvances) & ~position.OccupiedBitmask;

	// bishops pinned by a rook can't move 
	const Bitboard movableBishops = currPieces.Bishops & ~rookPinmask;
	// likewise, rooks pinned by 
	const Bitboard movableRooks = currPieces.Rooks & ~bishopPinmask;
	const Bitboard movableKnights = currPieces.Knights & ~(bishopPinmask | rookPinmask);
	// pinned queens can move always
	
	// no checks
	if (!(knightCheckers | pawnCheckers | rookCheckers | bishopCheckers))
	{
		WriteSliderMoves<color, false, false>(moveList, movableBishops, movableRooks,
			currPieces.Queens, position.OccupiedBitmask, currPieces.Pieces, bishopPinmask,
			rookPinmask, 0, 0, 0);
		WriteKnightMoves<color, false, false>(moveList, movableKnights, currPieces.Pieces, 0,
			0, 0);
	}
	// blockable checks
	else if (!(knightCheckers | pawnCheckers) && (rookCheckers | bishopCheckers))
	{
		WriteSliderMoves<color, true, false>(moveList, movableBishops, movableRooks,
			currPieces.Queens, position.OccupiedBitmask, currPieces.Pieces, bishopPinmask,
			rookPinmask, rookCheckers | bishopCheckers, bishopCheckmask, rookCheckmask);
		pawnLeftCaptures &= (bishopCheckmask|rookCheckmask);
		pawnRightCaptures &= (bishopCheckmask | rookCheckmask);
		legalPawnAdvances &= (bishopCheckmask | rookCheckmask);
		legalPawnDoubleAdvances &= (bishopCheckmask | rookCheckmask);
		WriteKnightMoves<color, true, false>(moveList, movableKnights, currPieces.Pieces, rookCheckers | bishopCheckers,
			bishopCheckmask, rookCheckmask);
	}
	// unblockable checks
	else
	{
		WriteSliderMoves<color, true, true>(moveList, movableBishops, movableRooks,
			currPieces.Queens, position.OccupiedBitmask, currPieces.Pieces, bishopPinmask,
			rookPinmask, knightCheckers | pawnCheckers, 0, 0);
		pawnLeftCaptures &= knightCheckers | pawnCheckers;
		pawnRightCaptures &= knightCheckers | pawnCheckers;
		legalPawnAdvances = 0;
		legalPawnDoubleAdvances = 0;
		WriteKnightMoves<color, true, true>(moveList, movableKnights, currPieces.Pieces, knightCheckers | pawnCheckers,
			0, 0);
	}
	// all bad moves have been pruned
	WritePawnMoves<color>(moveList, pawnLeftCaptures, pawnRightCaptures, legalPawnAdvances, legalPawnDoubleAdvances, currPieces.Pawns);


	if constexpr (hasEP)
	{
		const uint32_t enPassantSquareIndex = BitIndex(position.EnPassantSquare);
		const Bitboard victim = EN_PASSANT_VICTIM_BITMASK_LOOKUP[enPassantSquareIndex];
		// move the EP pawn and remove the target, see if king is attacked by a slider
		const Bitboard occupiedBitmaskWithEnPassantVictimsRemoved = position.OccupiedBitmask ^ victim;
		// pawns that are pinned by rooks can't EP
		Bitboard enPassantCandidates = EN_PASSANT_CANDIDATES_LOOKUP[enPassantSquareIndex] & (currPieces.Pawns & ~rookPinmask);
		while (enPassantCandidates)
		{
			const Bitboard enPassantCandidate = PopBit(enPassantCandidates);
			const Bitboard occupiedBitmaskWithEnPassantMoveMade = occupiedBitmaskWithEnPassantVictimsRemoved ^ (enPassantCandidate | position.EnPassantSquare);
			const Bitboard kingBishopAttacks = GetSingleBishopAttacks(king, occupiedBitmaskWithEnPassantMoveMade);
			const Bitboard kingRookAttacks = GetSingleRookAttacks(king, occupiedBitmaskWithEnPassantMoveMade);
			if (
				!(kingBishopAttacks & enemyBishopQueen)
				&& 
				!(kingRookAttacks & enemyRookQueen)
				)
			{
				constexpr bool isCastling = false;
				constexpr bool isEnPassant = true;
				WriteMoves<PAWN, color, isCastling, isEnPassant>(moveList, position.EnPassantSquare, BitIndex(enPassantCandidate));
			}
		}
	}

	// kingside castling
	if constexpr (castlingPermissions & 0b1)
	{
		const bool canCastle = !((attackedSquares & KingsideCastlingKingPath<color>()) ||
			(position.OccupiedBitmask & (KingsideCastlingRookPath<color>() & ~king)));
		if (canCastle)
		{
			constexpr bool isCastling = true;
			WriteMoves<KING, color, isCastling>(moveList, KingsideCastlingRookBitmask<color>(), kingIndex);
		}
	}
	// queenside castling
	if constexpr (castlingPermissions & 0b10)
	{
		const bool canCastle = !((attackedSquares & QueensideCastlingKingPath<color>()) ||
			(position.OccupiedBitmask & (QueensideCastlingRookPath<color>() & ~king)));
		if (canCastle)
		{
			constexpr bool isCastling = true;
			WriteMoves<KING, color, isCastling>(moveList, QueensideCastlingRookBitmask<color>(), kingIndex);
		}
	}
	return moveList;
}

template<Color sideToMove>
forceinline MoveList& GenerateMoves(const Position& position, MoveList& moveList)
{
	const bool isEnPassantAllowed = static_cast<bool>(position.EnPassantSquare);
	const uint32_t castlingBitmask = position.GetCurrentCastling<sideToMove>().CastlingPermissionsBitmask;

		 if (castlingBitmask == 0b11 && !isEnPassantAllowed) GenerateMoves<sideToMove, 0b11, false>(moveList, position);
	else if (castlingBitmask == 0b00 && !isEnPassantAllowed) GenerateMoves<sideToMove, 0b00, false>(moveList, position);
	else if (castlingBitmask == 0b01 && !isEnPassantAllowed) GenerateMoves<sideToMove, 0b01, false>(moveList, position);
	else if (castlingBitmask == 0b10 && !isEnPassantAllowed) GenerateMoves<sideToMove, 0b10, false>(moveList, position);
	else if (castlingBitmask == 0b11 &&  isEnPassantAllowed) GenerateMoves<sideToMove, 0b11, true >(moveList, position);
	else if (castlingBitmask == 0b00 &&  isEnPassantAllowed) GenerateMoves<sideToMove, 0b00, true >(moveList, position);
	else if (castlingBitmask == 0b01 &&  isEnPassantAllowed) GenerateMoves<sideToMove, 0b01, true >(moveList, position);
	else if (castlingBitmask == 0b10 &&  isEnPassantAllowed) GenerateMoves<sideToMove, 0b10, true >(moveList, position);

	return moveList;
}

forceinline MoveList& GenerateMoves(const Position& position, MoveList& moveList)
{
	const bool isEnPassantAllowed = static_cast<bool>(position.EnPassantSquare);
	const Color color = position.SideToMove;
	const uint32_t castlingBitmask = position.GetCurrentCastling().CastlingPermissionsBitmask;

		 if (color == WHITE && castlingBitmask == 0b11 && !isEnPassantAllowed) GenerateMoves<WHITE, 0b11, false>(moveList, position);
	else if (color == BLACK && castlingBitmask == 0b11 && !isEnPassantAllowed) GenerateMoves<BLACK, 0b11, false>(moveList, position);
	else if (color == WHITE && castlingBitmask == 0b00 && !isEnPassantAllowed) GenerateMoves<WHITE, 0b00, false>(moveList, position);
	else if (color == BLACK && castlingBitmask == 0b00 && !isEnPassantAllowed) GenerateMoves<BLACK, 0b00, false>(moveList, position);
	else if (color == WHITE && castlingBitmask == 0b01 && !isEnPassantAllowed) GenerateMoves<WHITE, 0b01, false>(moveList, position);
	else if (color == BLACK && castlingBitmask == 0b01 && !isEnPassantAllowed) GenerateMoves<BLACK, 0b01, false>(moveList, position);
	else if (color == WHITE && castlingBitmask == 0b10 && !isEnPassantAllowed) GenerateMoves<WHITE, 0b10, false>(moveList, position);
	else if (color == BLACK && castlingBitmask == 0b10 && !isEnPassantAllowed) GenerateMoves<BLACK, 0b10, false>(moveList, position);
	else if (color == WHITE && castlingBitmask == 0b11 &&  isEnPassantAllowed) GenerateMoves<WHITE, 0b11, true >(moveList, position);
	else if (color == BLACK && castlingBitmask == 0b11 &&  isEnPassantAllowed) GenerateMoves<BLACK, 0b11, true >(moveList, position);
	else if (color == WHITE && castlingBitmask == 0b00 &&  isEnPassantAllowed) GenerateMoves<WHITE, 0b00, true >(moveList, position);
	else if (color == BLACK && castlingBitmask == 0b00 &&  isEnPassantAllowed) GenerateMoves<BLACK, 0b00, true >(moveList, position);
	else if (color == WHITE && castlingBitmask == 0b01 &&  isEnPassantAllowed) GenerateMoves<WHITE, 0b01, true >(moveList, position);
	else if (color == BLACK && castlingBitmask == 0b01 &&  isEnPassantAllowed) GenerateMoves<BLACK, 0b01, true >(moveList, position);
	else if (color == WHITE && castlingBitmask == 0b10 &&  isEnPassantAllowed) GenerateMoves<WHITE, 0b10, true >(moveList, position);
	else if (color == BLACK && castlingBitmask == 0b10 &&  isEnPassantAllowed) GenerateMoves<BLACK, 0b10, true >(moveList, position);

	return moveList;
}