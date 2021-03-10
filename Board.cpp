// include board header
#include "Board.h"

// board constructor
Board::Board() 
{

}

// load board position from Forsyth-Edwards-Notation
void Board::loadFromFen(std::string fen) {
	// split fen into a string array
	std::string splitFen[6];
	int i = 0;
	for (char c : fen)
	{
		if (c == ' ')
		{
			i++;
		}
		else
		{
			splitFen[i] += c;
		}
	}

	// reset pieces
	for (int i = 0; i < 64; i++)
	{
		piecesMB[i] = EMPTY;
	}
	for (int i = 0; i < 12; i++)
	{
		piecesBB[i] = U64(0);
		pieceLists[i] = PieceList();
	}
	takenBB = U64(0);
    colorBB[0] = U64(0);
	colorBB[1] = U64(0);
	zobrist.reset();
	
	// x and y vars for looping through the board
	int x = 0;
	int y = 0;

	// loop through each character in board position
	for (char c : splitFen[0])
	{
		// if char is '/' switch to next line
		if (c == '/')
		{
			y++;
			x = 0;
		}
		// if char is a digit skip n squares
		else if (std::isdigit(c))
		{
			x += c - '0';
		}
		// if char is a letter, convert it to id and add it to the bitboard under x and y
		else
		{
			addPiece(Piece::charToInt(c), Square::fromXY(x, y));

			x++;
		}
	}

	// initialize whose move it is by the second string
	turnColor = (splitFen[1] == "w" ? WHITE : BLACK);
	if (turnColor)
	{
		zobrist.changeTurn();
	}

	// reset castling rights
	for (int i = 0; i < 4; i++)
	{
		castlingRights[i] = false;
	}

	// loop through third string
	for (char c : splitFen[2])
	{
		// change castling rights based on character
		switch (c)
		{
			case 'K':
				castlingRights[0] = true;
				zobrist.changeCastling(0);
				break;
			case 'Q':
				castlingRights[1] = true;
				zobrist.changeCastling(1);
				break;
			case 'k':
				castlingRights[2] = true;
				zobrist.changeCastling(2);
				break;
			case 'q':
				castlingRights[3] = true;
				zobrist.changeCastling(3);
				break;
		}
	}

	// if fourth string is "-" add -1 as en passant square
	if (splitFen[3] == "-")
	{
		enPassant = -1;
	}
	// else set en passant to the specified square
	else
	{
		enPassant = Square::fromString(splitFen[3]);
		zobrist.changeEnPassant(Square::fileOf(enPassant));
	}

	// load half move clock and move count from fifth and sixth string
	halfMoveClock = std::stoi(splitFen[4]);
	moveCount = std::stoi(splitFen[5]);
}

void Board::movePiece(int piece, int from, int to)
{
	int pieceColor = Piece::colorOf(piece);

	// get a from and to bitboard based on move
	U64 fromBB = U64(1) << from;
	U64 toBB = U64(1) << to;
	U64 fromToBB = fromBB ^ toBB;

	// simulate move in the piece bitboard
	piecesBB[piece] ^= fromToBB;

	// update taken bitboard and color bitboard
	takenBB ^= fromBB;
	takenBB |= toBB;
	colorBB[pieceColor] ^= fromToBB;

	pieceLists[piece].move(from, to);
	zobrist.movePiece(piece, from, to);

	// update mailbox based on move
	piecesMB[from] = EMPTY;
	piecesMB[to] = piece;
}

void Board::addPiece(int piece, int square)
{
	int pieceColor = Piece::colorOf(piece);

	// get a from and to bitboard based on move
	U64 BB = U64(1) << square;

	piecesBB[piece] |= BB;
	takenBB |= BB;
	colorBB[pieceColor] |= BB;
	piecesMB[square] = piece;
	pieceLists[piece].add(square);
	zobrist.changePiece(piece, square);
}

void Board::removePiece(int piece, int square)
{
	int pieceColor = Piece::colorOf(piece);

	// get a from and to bitboard based on move
	U64 BB = U64(1) << square;

	piecesBB[piece] &= ~BB;
	takenBB &= ~BB;
	colorBB[pieceColor] &= ~BB;
	piecesMB[square] = EMPTY;
	pieceLists[piece].remove(square);
	zobrist.changePiece(piece, square);
}

// make a given move
void Board::makeMove(Move move)
{
	// save current information in the stack
	AdditionalInfo info(castlingRights, enPassant, halfMoveClock);
	previousInfo.push(info);

	previousPositions.push_back(zobrist.getHashKey());

	// get piece type and color
	int pieceType = Piece::typeOf(move.piece);
	bool pieceColor = Piece::colorOf(move.piece);

	// update captured piece if there is one (and it's not en passant)
	if ((move.cPiece != EMPTY) && (!move.enPassant))
	{
		removePiece(move.cPiece, move.to);
	}

	movePiece(move.piece, move.from, move.to);

	// if move is en passant
	if (move.enPassant)
	{
		// calculate the square of enemy pawn and create bitboard
		int capturedSquare = enPassant + ((pieceColor == WHITE) ? SOUTH : NORTH);

		removePiece(move.cPiece, capturedSquare);
	}

	// if move is castling
	if (move.castling)
	{
		// check if castling is queenside, calculate rank
		bool queenside = Square::fileOf(move.to) == 2;
		int rank = Square::rankOf(move.from) * 8;

		// calculate from and to squares of rook and create bitboard
		int rookFrom = rank + (queenside ? 0 : 7);
		int rookTo = rank + (queenside ? 3 : 5);

		movePiece(piecesMB[rookFrom], rookFrom, rookTo);
	}

	// if it's a promotion
	if (move.promotion != EMPTY)
	{
		// change pieces bitboard and mailbox, increase piece value
		removePiece(move.piece, move.to);
		addPiece(move.promotion, move.to);
	}

	// if moved piece is rook, remove castling rights based on square
	if (pieceType == ROOK)
	{
		int index = -1;

		switch (move.from)
		{
		case 0:
			index = 3;
			break;
		case 7:
			index = 2;
			break;
		case 56:
			index = 1;
			break;
		case 63:
			index = 0;
			break;
		}

		if (index != -1)
		{
			if (castlingRights[index])
			{
				zobrist.changeCastling(index);
			}
			castlingRights[index] = false;
		}
	}

	// if captured piece is rook, remove castling rights based on square
	if (Piece::typeOf(move.cPiece) == ROOK)
	{
		int index = -1;

		switch (move.to)
		{
		case 0:
			index = 3;
			break;
		case 7:
			index = 2;
			break;
		case 56:
			index = 1;
			break;
		case 63:
			index = 0;
			break;
		}

		if (index != -1)
		{
			if (castlingRights[index])
			{
				zobrist.changeCastling(index);
			}
			castlingRights[index] = false;
		}
	}

	// if captured piece is king, remove both castling rights
	if (pieceType == KING)
	{
		if (castlingRights[pieceColor * 2])
		{
			zobrist.changeCastling(pieceColor * 2);
		}
		if (castlingRights[pieceColor * 2 + 1])
		{
			zobrist.changeCastling(pieceColor * 2 + 1);
		}

		castlingRights[pieceColor * 2] = false;
		castlingRights[pieceColor * 2 + 1] = false;
	}

	// if pawn was moved twice, save en passant square
	if (std::abs(move.to - move.from) == 16 && pieceType == PAWN)
	{
		if (enPassant != -1)
		{
			zobrist.changeEnPassant(Square::fileOf(enPassant));
		}

		enPassant = (move.to + move.from) / 2;
		zobrist.changeEnPassant(Square::fileOf(enPassant));
	}
	else
	{
		if (enPassant != -1)
		{
			zobrist.changeEnPassant(Square::fileOf(enPassant));
		}

		enPassant = -1;
	}

	// increase half move clock if no pawn move or capture
	if (Piece::typeOf(move.piece) != PAWN && move.cPiece == EMPTY)
	{
		halfMoveClock++;
	}
	else
	{
		halfMoveClock = 0;
	}

	// change turn and increase move count
	turnColor = !turnColor;
	zobrist.changeTurn();

	if (turnColor == WHITE)
	{
		moveCount++;
	}
}

/*// unmake move
void Board::unmakeMove(Move move)
{
	// get type and color of moved piece
	int pieceType = Piece::typeOf(move.piece);
	bool pieceColor = Piece::colorOf(move.piece);

	// create bitboards based on from and to squares
	U64 fromBB = U64(1) << move.from;
	U64 toBB = U64(1) << move.to;
	U64 fromToBB = fromBB ^ toBB;

	// if it's a promotion
	if (move.promotion != EMPTY)
	{
		removePiece(move.promotion, move.to);
		addPiece(move.piece, move.to);
	}

	movePiece(move.piece, move.to, move.from);

	// update captured piece if there is one
	if ((move.cPiece != EMPTY) && (!move.enPassant))
	{
		addPiece(move.cPiece, move.to);
	}

	// if move is en passant
	if (move.enPassant)
	{
		// calculate square of captured pawn and create bitboard
		int capturedSquare = enPassant + ((pieceColor == WHITE) ? SOUTH : NORTH);
		addPiece(move.cPiece, capturedSquare);
	}

	// if move is castling
	if (move.castling)
	{
		// calculate if castle is queenside and calculate rank of rook
		bool queenside = Square::fileOf(move.to) == 2;
		int rank = Square::rankOf(move.from) * 8;

		// calculate the rook from and to squares and create bitboard
		int rookFrom = rank + (queenside ? 0 : 7);
		int rookTo = rank + (queenside ? 3 : 5);

		movePiece(piecesMB[rookTo], rookTo, rookFrom);
	}

	// change turn and reduce move count
	turnColor = !turnColor;
	if (turnColor == BLACK)
	{
		moveCount--;
	}

	// get information before this move
	AdditionalInfo lastInfo = previousInfo.top();
	previousInfo.pop();

	// load information
	for (int i = 0; i < 4; i++)
	{
		castlingRights[i] = lastInfo.castlingRights[i];
	}
	enPassant = lastInfo.enPassant;
	halfMoveClock = lastInfo.halfMoveClock;

	zobrist.set(previousPositions.back());
	previousPositions.pop_back();
}*/

// unmake move
void Board::unmakeMove(Move move)
{
	// get information before this move
	AdditionalInfo lastInfo = previousInfo.top();
	previousInfo.pop();

	// load information
	for (int i = 0; i < 4; i++)
	{
		castlingRights[i] = lastInfo.castlingRights[i];
	}
	enPassant = lastInfo.enPassant;
	halfMoveClock = lastInfo.halfMoveClock;

	// get type and color of moved piece
	int pieceType = Piece::typeOf(move.piece);
	bool pieceColor = Piece::colorOf(move.piece);

	// create bitboards based on from and to squares
	U64 fromBB = U64(1) << move.from;
	U64 toBB = U64(1) << move.to;
	U64 fromToBB = fromBB ^ toBB;

	// if it's a promotion
	if (move.promotion != EMPTY)
	{
		// change promoted piece to pawn in bitboard and mailbox before reversing the move, decrease piece value
		piecesBB[move.piece] ^= toBB;
		piecesBB[move.promotion] ^= toBB;
		piecesMB[move.to] = move.piece;
		pieceLists[move.piece].add(move.to);
		pieceLists[move.promotion].remove(move.to);
	}

	// update piece bitboard
	piecesBB[move.piece] ^= fromToBB;

	// update taken and color bitboards
	takenBB ^= fromToBB;
	colorBB[pieceColor] ^= fromToBB;

	// update mailbox based on move
	piecesMB[move.from] = move.piece;
	piecesMB[move.to] = EMPTY;

	pieceLists[move.piece].move(move.to, move.from);

	// update captured piece if there is one
	if ((move.cPiece != EMPTY) && (!move.enPassant))
	{
		// "restore" captured piece in all bitboards and mailbox
		piecesBB[move.cPiece] |= toBB;
		takenBB |= toBB;
		colorBB[!pieceColor] |= toBB;
		piecesMB[move.to] = move.cPiece;

		// add square to piece lists
		pieceLists[move.cPiece].add(move.to);
	}

	// if move is en passant
	if (move.enPassant)
	{
		// calculate square of captured pawn and create bitboard
		int capturedSquare = enPassant + ((pieceColor == WHITE) ? SOUTH : NORTH);
		U64 capturedBB = U64(1) << capturedSquare;

		// restore the captured pawn
		piecesBB[move.cPiece] ^= capturedBB;
		takenBB ^= capturedBB;
		colorBB[!pieceColor] ^= capturedBB;
		piecesMB[capturedSquare] = move.cPiece;

		// add square to piece lists
		pieceLists[move.cPiece].add(capturedSquare);
	}

	// if move is castling
	if (move.castling)
	{
		// calculate if castle is queenside and calculate rank of rook
		bool queenside = Square::fileOf(move.to) == 2;
		int rank = Square::rankOf(move.from) * 8;

		// calculate the rook from and to squares and create bitboard
		int rookFrom = rank + (queenside ? 0 : 7);
		int rookTo = rank + (queenside ? 3 : 5);
		U64 rookFromToBB = (U64(1) << rookFrom) ^ (U64(1) << rookTo);

		// get the rook piece
		int rookPiece = piecesMB[rookTo];

		// reverse the rook move
		piecesBB[rookPiece] ^= rookFromToBB;
		takenBB ^= rookFromToBB;
		colorBB[pieceColor] ^= rookFromToBB;
		piecesMB[rookFrom] = rookPiece;
		piecesMB[rookTo] = EMPTY;
		pieceLists[rookPiece].move(rookTo, rookFrom);
	}

	// change turn and reduce move count
	turnColor = !turnColor;
	if (turnColor == BLACK)
	{
		moveCount--;
	}

	zobrist.set(previousPositions.back());
	previousPositions.pop_back();
}

// generate all moves with DirGolem
void Board::generateMoves(bool onlyCaptures)
{
	// save color of turn
	int color = turnColor;

	// save bitboards of squares in between of attacker and king
	U64 inBetween[4] = { U64(0), U64(0), U64(0), U64(0) };

	// save bitboards of rays coming from king
	U64 superAttacks[2] = { U64(0), U64(0) };

	// save bitboard of all attacks of other color
	U64 anyAttacks = U64(0);

	// create bitboard of empty squares
	U64 empty = ~takenBB;

	// first - obtain information from the enemy color's moves for checks and pins

	// loop through 8 main directions
	for (int i = 0; i < 8; i++)
	{
		// initialize attacks and super attacks
		U64 attacks, kingSuperAttacks;

		// save additional attacking piece besides queen (rook if direction is orthogonal, bishop if diagonal)
		U64 additionalPieces = (i < 4) ? piecesBB[ROOK + !color] : piecesBB[BISHOP + !color];

		// save attacks of these "ray pieces", while excluding king from empty set
		attacks = BB::rayAttacks(additionalPieces | piecesBB[QUEEN + !color], empty ^ piecesBB[KING + color], dirs[i]);

		// add these attacks
		anyAttacks |= attacks;

		// calculate attacks from the king in the opposite direction
		kingSuperAttacks = BB::rayAttacks(piecesBB[KING + color], empty, -dirs[i]);

		// add these "super attacks" to the specific super attack bitboard
		superAttacks[i / 4] |= kingSuperAttacks;

		// add the intersection between sliding attacks and king attacks to the in between bitboard
		inBetween[i / 2] |= attacks & kingSuperAttacks;
	}
	
	// enemy knight attacks
	anyAttacks |= BB::knightAttacks(piecesBB[KNIGHT + !color]);

	// enemy pawn attacks
	anyAttacks |= BB::pawnAnyAttacks(piecesBB[PAWN + !color], !color);

	// enemy king attacks
	anyAttacks |= BB::kingAttacks(piecesBB[KING + !color]);

	// calculate all in between squares
	U64 allInbetween = inBetween[0] | inBetween[1] | inBetween[2] | inBetween[3];

	// calculate pieces which block the check
	U64 blocks = allInbetween & ~takenBB;

	// calculate pieces where the check is from by intersecting super attacks of the king with the enemy color's pieces
	U64 checkFrom = (superAttacks[0] & (piecesBB[ROOK + !color] | piecesBB[QUEEN + !color]))
		| (superAttacks[1] & (piecesBB[BISHOP + !color] | piecesBB[QUEEN + !color]))
		| (BB::knightAttacks(piecesBB[KING + color]) & piecesBB[KNIGHT + !color])
		| (BB::pawnAnyAttacks(piecesBB[KING + color], color) & piecesBB[PAWN + !color]);

	// save null if it's a check or if it's a double check by using signed shifts to avoid branches
	I64 nullIfCheck = ((I64)(anyAttacks & piecesBB[KING + color]) - 1) >> 63;
	I64 nullIfDblCheck = ((I64)(checkFrom & (checkFrom - 1)) - 1) >> 63;

	// save checking information for later
	isCheck = nullIfCheck == 0;

	// get pieces where the turn color's pieces can move to avoid checks
	U64 checkTo = checkFrom | blocks | nullIfCheck;

	// full bitboard if not only captures, create capture mask
	U64 nullIfCaptures = ~U64(0) * !onlyCaptures;
	U64 captureMask = nullIfCaptures | colorBB[!color];
	
	// save move targets for every direction and create a target mask for all moves
	U64 moveTargets[16] = { U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0), U64(0)};
	U64 targetMask = ~colorBB[color] & checkTo & nullIfDblCheck & captureMask;

	// loop through 4 directions (horizontal, vertical, diagonal, antidiagonal)
	for (int i = 0; i < 4; i++)
	{
		// get additional piece (bishop or rook) based on direction
		U64 additionalPieces = (i < 2) ? piecesBB[ROOK + color] : piecesBB[BISHOP + color];

		// calculate sliders wihich aren't pinned in this direction
		U64 sliders = (additionalPieces | piecesBB[color + QUEEN]) & ~(allInbetween ^ inBetween[i]);

		// add attacks of these sliders to the target array after applying target mask
		moveTargets[i * 2] = BB::rayAttacks(sliders, empty, dirs[i * 2]) & targetMask;
		moveTargets[i * 2 + 1] = BB::rayAttacks(sliders, empty, dirs[i * 2 + 1]) & targetMask;
	}
	
	// get knights which aren't pinned and calculate moves in the knights' 8 directions
	U64 knights = piecesBB[color + KNIGHT] & ~allInbetween;
	for (int i = 8; i < 16; i++)
	{
		moveTargets[i] = BB::shiftTwo(knights, dirs[i]) & targetMask;
	}
	
	// get pawn direction based on color and get array index of this direction
	int pawnDir = color == WHITE ? NORTH : SOUTH;
	int pawnDirIndex = dirToIndex[pawnDir];

	// target bitboard for pawns based on enemy pieces
	U64 targets = colorBB[!color] & targetMask;
	U64 pawns;

	for (int dir : {WEST, EAST})
	{
		// add pawn attacks in east and west of pawns which aren't pinned in that direction
		pawns = piecesBB[color + PAWN] & ~(allInbetween ^ inBetween[dirToIndex[pawnDir + dir] / 2]);
		moveTargets[dirToIndex[pawnDir + dir]] |= BB::shiftOne(pawns, pawnDir + dir) & targets;
	}
 
	// add pawn pushes and double pawn pushes for pawns that aren't pinned vertically
	pawns = piecesBB[color + PAWN] & ~(allInbetween ^ inBetween[1]);
	U64 pawnPushes = BB::shiftOne(pawns, pawnDir) & ~takenBB;
	moveTargets[pawnDirIndex] |= pawnPushes & targetMask;
	
	U64 rank4 = color == WHITE ? U64(0x000000FF00000000) : U64(0x00000000FF000000);
	moveTargets[pawnDirIndex] |= BB::shiftOne(pawnPushes, pawnDir) & ~takenBB & targetMask & rank4;

	// if en passant could be possible
	if (enPassant != -1)
	{
		// get empty square bitboard without pawn which is attackes by en passant
		U64 emptyWithoutPawn = ~(takenBB ^ (U64(1) << (enPassant - pawnDir)));
		U64 inBetweenHor = U64(0);

		for (int dir : {WEST, EAST})
		{
			// get horizontal enemy queen and rook attacks, get attacks in opposite direction of king, save intersection
			U64 attacks = BB::rayAttacks(piecesBB[ROOK + !color] | piecesBB[QUEEN + !color], emptyWithoutPawn, dir);
			U64 superAttacks = BB::rayAttacks(piecesBB[KING + color], emptyWithoutPawn, -dir);
			inBetweenHor |= attacks & superAttacks;
		}

		// always make the move possible if the en passant pawn is a target
		targetMask = ~(((I64)(U64(1) << (enPassant - pawnDir) & targetMask) - 1) >> 63);
		targets = (U64(1) << enPassant) & targetMask;

		for (int dir : {WEST, EAST})
		{
			// add pawn attacks in east and west of pawns which aren't pinned in that direction
			pawns = piecesBB[color + PAWN] & ~inBetweenHor & ~(allInbetween ^ inBetween[dirToIndex[pawnDir + dir] / 2]);
			moveTargets[dirToIndex[pawnDir + dir]] |= BB::shiftOne(pawns, pawnDir + dir) & targets;
		}
	}

	// move king in all possible directions which aren't attacked
	targetMask = ~(colorBB[color] | anyAttacks) & captureMask;
	U64 king = piecesBB[color + KING];
	for (int i = 0; i < 8; i++)
	{
		moveTargets[i] |= BB::shiftOne(king, dirs[i]) & targetMask;
	}

	// don't castle if in check
	king &= nullIfCheck;

	// check if spaces between king and rook are taken or in check and that castling rights aren't taken, if not add castles
	targetMask = ~(takenBB | anyAttacks);
	U64 eastCastle = BB::shiftOne(king, EAST) & targetMask & (~U64(0) * ((castlingRights[0] && (color == WHITE)) || (castlingRights[2] && (color == BLACK))));
	moveTargets[0] |= BB::shiftOne(eastCastle, EAST) & targetMask & captureMask;
	U64 westCastle = BB::shiftOne(king, WEST) & targetMask & (~U64(0) * ((castlingRights[1] && (color == WHITE)) || (castlingRights[3] && (color == BLACK))));
	moveTargets[1] |= BB::shiftOne(BB::shiftTwo(westCastle, WEST + WEST) & ~takenBB, EAST) & targetMask & captureMask;
	
	// clear the move list
	moveList.clear();

	// loop through 8 ray directions
	for (int i = 0; i < 8; i++)
	{
		// while there are targets to analyze
		while (moveTargets[i] != 0)
		{
			// find the next target and erase it from bitboard
			int target = BB::bitScanForward(moveTargets[i]);
			moveTargets[i] ^= U64(1) << target;

			// cast a ray in that direction until you find a piece
			int source = target - dirs[i];
			while (piecesMB[source] == EMPTY)
			{
				source -= dirs[i];
			}
			
			// get piece color and type
			int pieceColor = Piece::colorOf(piecesMB[source]);
			int pieceType = Piece::typeOf(piecesMB[source]);

			// move properties
			int takenPiece = piecesMB[target];
			bool isEnPassant = false;
			bool isCastling = false;
			
			// en passant move properties
			if (target == enPassant && pieceType == PAWN)
			{
				takenPiece = !pieceColor + PAWN;
				isEnPassant = true;
			}

			// castling move properties
			if (std::abs(source - target) == 2 && pieceType == KING)
			{
				isCastling = true;
			}

			// promotion move properties
			if ((target / 8 == pieceColor * 7) && (pieceType == PAWN))
			{
				// add all promotion pieces ad moves
				for (int p : {QUEEN, ROOK, BISHOP, KNIGHT})
				{
					Move move = { source, target, piecesMB[source], takenPiece, false, false, p + pieceColor, 0 };
					moveList.push_back(move);
				}
			}
			else
			{
				// add move to move list from properties
				Move move = { source, target, piecesMB[source], takenPiece, isEnPassant, isCastling, EMPTY, 0 };
				moveList.push_back(move);
			}
		}
	}
	
	// knight move directions
	for (int i = 8; i < 16; i++)
	{
		// while there are targets
		while (moveTargets[i] != 0)
		{
			// get next target and erase it from bitboard
			int target = BB::bitScanForward(moveTargets[i]);
			moveTargets[i] ^= U64(1) << target;

			// get the source of the move
			int source = target - dirs[i];
			
			// add move to move list
			Move move = { source, target, piecesMB[source], piecesMB[target], false, false, EMPTY, 0 };
			moveList.push_back(move);
		}
	}
}

int Board::getState()
{
	// if no moves are available
	if (moveList.size() == 0)
	{
		if (isCheck)
		{
			// if in check, end the game on a win
			if (turnColor == WHITE)
			{
				return BLACK_WIN;
			}
			else
			{
				return WHITE_WIN;
			}
		}
		// end the game on a draw if stalemate
		else
		{
			return DRAW;
		}
	}
	// end the game on a draw if 50-move-rule
	else if (halfMoveClock >= 100)
	{
		return DRAW;
	}
	else
	{
		// check for insufficient material
		if (pieceLists[WHITE + QUEEN].getCount() == 0 && pieceLists[BLACK + QUEEN].getCount() == 0 &&
		   (pieceLists[WHITE + ROOK].getCount() == 0 && pieceLists[BLACK + ROOK].getCount() == 0 &&
			pieceLists[WHITE + PAWN].getCount() == 0 && pieceLists[BLACK + PAWN].getCount() == 0))
		{
			if (pieceLists[WHITE + KNIGHT].getCount() == 0 && pieceLists[WHITE + BISHOP].getCount() == 0 &&
				(pieceLists[BLACK + KNIGHT].getCount() == 0 && pieceLists[BLACK + BISHOP].getCount() == 0))
			{
				return DRAW;
			}

			for (int col = 0; col < 2; col++)
			{
				PieceList allyKnights = pieceLists[col + KNIGHT];
				PieceList allyBishops = pieceLists[col + BISHOP];
				PieceList enemyKnights = pieceLists[!col + KNIGHT];
				PieceList enemyBishops = pieceLists[!col + BISHOP];

				if (allyKnights.getCount() == 1 && allyBishops.getCount() == 0 && enemyKnights.getCount() == 0 && enemyBishops.getCount() == 0)
				{
					return DRAW;
				}

				if (allyKnights.getCount() == 0 && allyBishops.getCount() == 1 && enemyKnights.getCount() == 0 && enemyBishops.getCount() == 0)
				{
					return DRAW;
				}

				if (allyKnights.getCount() == 0 && allyBishops.getCount() == 1 && enemyKnights.getCount() == 0 && enemyBishops.getCount() == 1)
				{
					if ((allyBishops[0] % 2) == (enemyBishops[0] % 2))
					{
						return DRAW;
					}
				}
			}
		}

		if (std::count(previousPositions.begin(), previousPositions.end(), zobrist.getHashKey()) == 2)
		{
			return DRAW;
		}
	}

	return PLAY;
}

// return if it's white's turn
int Board::getTurnColor()
{
	return turnColor;
}

// return if current color is in check
bool Board::getCheck()
{
	return isCheck;
}

// return half move clock
int Board::getHalfMoveClock()
{
	return halfMoveClock;
}

// return pieces of board
U64* Board::getPiecesBB()
{
	return piecesBB;
}

// return pieces mailbox
int* Board::getPiecesMB()
{
	return piecesMB;
}

PieceList* Board::getPieceLists()
{
	return pieceLists;
}

// return the move list
std::vector<Move> Board::getMoveList()
{
	return moveList;
}