#include "Move.h"

// checking if two moves are the same by checking from and to squares and promotion piece
bool Move::operator==(Move move)
{
	return (move.from == from) && (move.to == to) && (move.promotion == promotion);
}

// return notation of move e.g. "a1f3" or "c2c1q" 
std::string Move::getNotation()
{
	// handle nullmoves
	if (Move::isNull(*this))
	{
		return "0000";
	}

	// concatenate from and to squares as base of move
	std::string base = Square::toString(from) + Square::toString(to);

	// if there's a promotion, specify the promotion piece
	if (promotion != -1)
	{
		switch (Piece::typeOf(promotion))
		{
		case QUEEN:
			base += "q";
			break;
		case ROOK:
			base += "r";
			break;
		case BISHOP:
			base += "b";
			break;
		case KNIGHT:
			base += "n";
			break;
		}
	}

	return base;
}

// load move from source and target squares
Move Move::loadFromSquares(int from, int to, const std::array<int, 64>& piecesMB)
{
	// move properties
	int movedPiece = piecesMB[from];
	int takenPiece = piecesMB[to];
	int promotionPiece = EMPTY;
	bool isEnPassant = false;
	bool isCastling = false;

	// get piece color and type
	int pieceColor = Piece::colorOf(movedPiece);
	int pieceType = Piece::typeOf(movedPiece);

	// en passant move properties
	if ((from - to) % 8 != 0 && piecesMB[to] == EMPTY && pieceType == PAWN)
	{
		takenPiece = !pieceColor + PAWN;
		isEnPassant = true;
	}

	// castling move properties
	if (std::abs(from - to) == 2 && pieceType == KING)
	{
		isCastling = true;
	}

	// promotion move properties
	if ((Square::rankOf(to) == pieceColor * 7) && (pieceType == PAWN))
	{
		promotionPiece = QUEEN + pieceColor;
	}

	// return move based on properties
	Move move{ from, to, movedPiece, takenPiece, isEnPassant, isCastling, promotionPiece, 0 };
	return move;
}

// load move from long algebraic notation
Move Move::loadFromNotation(std::string notation, const std::array<int, 64>& piecesMB)
{
	// handle nullmoves
	if (notation == "0000")
	{
		return Move::nullmove();
	}

	int from = Square::fromString(notation.substr(0, 2));
	int to = Square::fromString(notation.substr(2, 4));
	Move move = Move::loadFromSquares(from, to, piecesMB);

	int pieceColor = Piece::colorOf(piecesMB[from]);
	if (notation.size() == 5)
	{
		switch (notation[4])
		{
		case 'q':
			move.promotion = QUEEN + pieceColor;
			break;
		case 'r':
			move.promotion = ROOK + pieceColor;
			break;
		case 'b':
			move.promotion = BISHOP + pieceColor;
			break;
		case 'n':
			move.promotion = KNIGHT + pieceColor;
			break;
		}
	}

	return move;
}

// return a null move
Move Move::nullmove()
{
	Move newMove = { EMPTY, EMPTY, EMPTY, EMPTY, false, false, EMPTY, 0 };
	return newMove;
}

// check if move is valid
bool Move::isNull(Move move)
{
	return move == Move::nullmove();
}
