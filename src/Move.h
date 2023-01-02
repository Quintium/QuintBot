#pragma once

// class for squares in the board
class Square
{
public:
	// return file or rank number from character
	static int fromChar(char c)
	{
		if (std::isdigit(c))
		{
			return 8 - (c - '0');
		}
		else
		{
			return c - 'a';
		}
	}

	// return character from file or rank number
	static char toChar(int n, bool isRank)
	{
		if (isRank)
		{
			return '0' + 8 - n;
		}
		else
		{
			return 'a' + n;
		}
	}

	// create new square from a string, e. g. "a3"
	static int fromString(std::string s)
	{
		// convert letters to x and y
		int x = fromChar(s[0]);
		int y = fromChar(s[1]);

		return x + y * 8;
	}

	// convert x and y values to string, e. g. "a3"
	static std::string toString(int n)
	{
		return std::string(1, toChar(fileOf(n), false)) + toChar(rankOf(n), true);
	}

	// create square from two x and y
	static int fromXY(int x, int y)
	{
		return x + y * 8;
	}

	// return file of square
	static int fileOf(int n)
	{
		return n % 8;
	}

	// return rank of square
	static int rankOf(int n)
	{
		return n / 8;
	}

	// adjust square if perspective is not white
	static int perspective(int n, int col)
	{
		return (col == WHITE) ? n : (63 - n);
	}

	// reverse coordinate if perspective is not white
	static int coordPerspective(int n, int col)
	{
		return (col == WHITE) ? n : (7 - n);
	}

	// return if square on the board should be light
	static bool isLight(int n)
	{
		return (fileOf(n) + rankOf(n)) % 2 == 0;
	}
};

// struct for a move
struct Move
{
	// all move properties
	int from;
	int to;
	int piece;
	int cPiece;
	bool enPassant;
	bool castling;
	int promotion;
	int score;

	// checking if two moves are the same by checking from and to squares and promotion piece
	bool operator==(Move move)
	{
		return (move.from == from) && (move.to == to) && (move.promotion == promotion);
	}

	// return notation of move e.g. "a1f3" or "c2c1q" 
	std::string getNotation()
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
	static Move loadFromSquares(int from, int to, int* piecesMB)
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
		Move move = { from, to, movedPiece, takenPiece, isEnPassant, isCastling, promotionPiece, 0 };
		return move;
	}

	// load move from long algebraic notation
	static Move loadFromNotation(std::string notation, int* piecesMB)
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
	static Move nullmove()
	{
		Move newMove = { EMPTY, EMPTY, EMPTY, EMPTY, false, false, EMPTY, 0 };
		return newMove;
	}

	// check if move is valid
	static bool isNull(Move move)
	{
		return move == Move::nullmove();
	}
};