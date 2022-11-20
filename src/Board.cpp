// include board header
#include "Board.h"

// load starting position
void Board::loadStartPosition()
{
	loadFromFen(startPosition);
}

// load board position from Forsyth-Edwards-Notation
void Board::loadFromFen(std::string fen) 
{
	// check if the game started normally (normal start position)
	normalStart = (fen == startPosition);

	// reset move history
	previousInfo = std::stack<PositionalInfo>();
	previousPositions.clear();
	moveHistory.clear();

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

// return FEN of board
std::string Board::getFen()
{
	// empty fen
	std::string fen = "";

	// loop through ranks
	for (int y = 0; y < 8; y++)
	{
		// count number of spaces
		int spaces = 0;

		// loop through files
		for (int x = 0; x < 8; x++)
		{
			// if it's a space increase space counter
			if (piecesMB[Square::fromXY(x, y)] == EMPTY)
			{
				spaces++;
			}
			else
			{
				// if there have been spaces before this piece, add them to fen and reset space counter
				if (spaces > 0)
				{
					fen += std::to_string(spaces);
					spaces = 0;
				}

				// add piece char to fen
				fen += Piece::intToChar(piecesMB[Square::fromXY(x, y)]);
			}
		}

		// if there are spaces left, add them to fen
		if (spaces > 0)
		{
			fen += std::to_string(spaces);
		}

		// end rank with "/" if it's not the last rank
		if (y != 7)
		{
			fen += "/";
		}
	}

	fen += " ";

	// add turn color to string
	fen += (turnColor == WHITE) ? "w" : "b";

	fen += " ";

	// add the castling rights to the string as chars
	std::string castlingString = "KQkq";
	bool anyCastling = false;
	for (int i = 0; i < 4; i++)
	{
		if (castlingRights[i])
		{
			fen += castlingString[i];
			anyCastling = true;
		}
	}

	// if there are no castling rights add "-" to fen
	if (!anyCastling)
	{
		fen += "-";
	}

	fen += " ";

	// if there's no en passant square, add "-" to fen
	if (enPassant == -1)
	{
		fen += "-";
	}
	// else add the square to fen
	else
	{
		fen += Square::toString(enPassant);
	}

	fen += " ";

	// add half move clock to fen
	fen += std::to_string(halfMoveClock);

	fen += " ";

	// add move count to fen
	fen += std::to_string(moveCount);

	// return the fen
	return fen;
}

// move a piece on the board
void Board::movePiece(int piece, int from, int to)
{
	// save color of piece
	int pieceColor = Piece::colorOf(piece);

	// get a from and to bitboard based on move
	U64 fromBB = U64(1) << from;
	U64 toBB = U64(1) << to;
	U64 fromToBB = fromBB ^ toBB;

	// simulate move in the pieces, taken and color bitboard
	piecesBB[piece] ^= fromToBB;
	takenBB ^= fromBB;
	takenBB |= toBB;
	colorBB[pieceColor] ^= fromToBB;

	// update mailbox based on move
	piecesMB[from] = EMPTY;
	piecesMB[to] = piece;

	// update piece list and zobrist key
	pieceLists[piece].move(from, to);
	zobrist.movePiece(piece, from, to);
}

// add piece to the board
void Board::addPiece(int piece, int square)
{
	// save color of piece
	int pieceColor = Piece::colorOf(piece);

	// get a bitboard of the square
	U64 BB = U64(1) << square;

	// update pieces, taken and color bitboard
	piecesBB[piece] |= BB;
	takenBB |= BB;
	colorBB[pieceColor] |= BB;

	// update mailbox based on piece
	piecesMB[square] = piece;

	// update piece list and zobrist key
	pieceLists[piece].add(square);
	zobrist.changePiece(piece, square);
}

// remove piece from the board
void Board::removePiece(int piece, int square)
{
	// save color of piece
	int pieceColor = Piece::colorOf(piece);

	// get a bitboard of the square
	U64 BB = U64(1) << square;

	// update pieces, taken and color bitboard
	piecesBB[piece] &= ~BB;
	takenBB &= ~BB;
	colorBB[pieceColor] &= ~BB;

	// update mailbox based on piece
	piecesMB[square] = EMPTY;

	// update piece list and zobrist key
	pieceLists[piece].remove(square);
	zobrist.changePiece(piece, square);
}

// make a given move
void Board::makeMove(Move move)
{
	// save current information in the stack
	PositionalInfo info(castlingRights, enPassant, halfMoveClock);
	previousInfo.push(info);

	// save current zobrist key
	previousPositions.push_back(zobrist.getHashKey());

	// get piece type and color
	int pieceType = Piece::typeOf(move.piece);
	int pieceColor = Piece::colorOf(move.piece);

	// update captured piece if there is one (and it's not en passant)
	if ((move.cPiece != EMPTY) && (!move.enPassant))
	{
		removePiece(move.cPiece, move.to);
	}

	// move the piece
	movePiece(move.piece, move.from, move.to);

	// if move is en passant
	if (move.enPassant)
	{
		// calculate the square of enemy pawn
		int capturedSquare = enPassant + ((pieceColor == WHITE) ? SOUTH : NORTH);

		// remove that pawn
		removePiece(move.cPiece, capturedSquare);
	}

	// if move is castling
	if (move.castling)
	{
		// check if castling is queenside, calculate rank
		bool queenside = Square::fileOf(move.to) == 2;
		int rank = Square::rankOf(move.from) * 8;

		// calculate from and to squares of rook
		int rookFrom = rank + (queenside ? 0 : 7);
		int rookTo = rank + (queenside ? 3 : 5);

		// move that rook
		movePiece(piecesMB[rookFrom], rookFrom, rookTo);
	}

	// if it's a promotion
	if (move.promotion != EMPTY)
	{
		// change remove pawn and add promotion piece
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
			// change zobrist castling right if necessary
			if (castlingRights[index])
			{
				zobrist.changeCastling(index);
			}

			// remove castling right
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
			// change zobrist castling right if necessary
			if (castlingRights[index])
			{
				zobrist.changeCastling(index);
			}

			// remove castling right
			castlingRights[index] = false;
		}
	}

	// if captured piece is king, remove both castling rights
	if (pieceType == KING)
	{
		// change zobrist castling rights if necessary
		if (castlingRights[pieceColor * 2])
		{
			zobrist.changeCastling(pieceColor * 2);
		}
		if (castlingRights[pieceColor * 2 + 1])
		{
			zobrist.changeCastling(pieceColor * 2 + 1);
		}

		// remove castling rights
		castlingRights[pieceColor * 2] = false;
		castlingRights[pieceColor * 2 + 1] = false;
	}

	// if pawn was moved two squares, save en passant square
	if (std::abs(move.to - move.from) == 16 && pieceType == PAWN)
	{
		// change previous en passant square in zobrist key
		if (enPassant != -1)
		{
			zobrist.changeEnPassant(Square::fileOf(enPassant));
		}

		// save en passant squares
		enPassant = (move.to + move.from) / 2;

		// change current en passant square in zobrist key
		zobrist.changeEnPassant(Square::fileOf(enPassant));
	}
	else
	{
		if (enPassant != -1)
		{
			// remove en passant square in zobrist key
			zobrist.changeEnPassant(Square::fileOf(enPassant));
		}

		// reset en passant square
		enPassant = -1;
	}

	// increase half move clock if no pawn move or capture, else reset it
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

	// add move to move history
	moveHistory.push_back(move);
}

// unmake move
void Board::unmakeMove(Move move)
{
	// get information before this move
	PositionalInfo lastInfo = previousInfo.top();
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

	// if it's a promotion remove promotion piece, add pawn
	if (move.promotion != EMPTY)
	{
		removePiece(move.promotion, move.to);
		addPiece(move.piece, move.to);
	}

	// move piece back
	movePiece(move.piece, move.to, move.from);

	// add captured piece if there is one
	if ((move.cPiece != EMPTY) && (!move.enPassant))
	{
		addPiece(move.cPiece, move.to);
	}

	// if move is en passant
	if (move.enPassant)
	{
		// calculate square of captured pawn and add it
		int capturedSquare = enPassant + ((pieceColor == WHITE) ? SOUTH : NORTH);
		addPiece(move.cPiece, capturedSquare);
	}

	// if move is castling
	if (move.castling)
	{
		// calculate if castle is queenside and calculate rank of rook
		bool queenside = Square::fileOf(move.to) == 2;
		int rank = Square::rankOf(move.from) * 8;

		// calculate the rook from and to squares
		int rookFrom = rank + (queenside ? 0 : 7);
		int rookTo = rank + (queenside ? 3 : 5);

		// move rook back
		movePiece(piecesMB[rookTo], rookTo, rookFrom);
	}

	// change turn and reduce move count
	turnColor = !turnColor;
	if (turnColor == BLACK)
	{
		moveCount--;
	}

	// set zobrist key to previous position
	zobrist.set(previousPositions.back());
	previousPositions.pop_back();

	// remove last move history
	moveHistory.pop_back();
}

// generate all moves with DirGolem
void Board::generateMoves(bool onlyCaptures)
{
	// save color of turn and enemy
	int color = turnColor;
	int eColor = 1 - color;

	// save bitboards of squares in between of attacker and king; 0 - horizontal, 1 - vertical, 2 - diagonal, 3 - antidiagonal; to calculate pins
	U64 inBetween[4] = { U64(0), U64(0), U64(0), U64(0) };

	// save bitboards of rays coming from king; 0 - orthogonal, 1 - diagonal; to calculate pieces giving check
	U64 superAttacks[2] = { U64(0), U64(0) };

	// save bitboard of all attacks of other color; to calculate squares in check
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
		U64 additionalPieces = (i < 4) ? piecesBB[ROOK + eColor] : piecesBB[BISHOP + eColor];

		// save attacks of these "ray pieces", while excluding king from empty set
		attacks = BB::rayAttacks(additionalPieces | piecesBB[QUEEN + eColor], empty ^ piecesBB[KING + color], dirs[i]);

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
	anyAttacks |= BB::knightAttacks(piecesBB[KNIGHT + eColor]);

	// enemy pawn attacks
	anyAttacks |= BB::pawnAnyAttacks(piecesBB[PAWN + eColor], eColor);

	// enemy king attacks
	anyAttacks |= BB::kingAttacks(piecesBB[KING + eColor]);

	// calculate all in between squares
	U64 allInbetween = inBetween[0] | inBetween[1] | inBetween[2] | inBetween[3];

	// calculate pieces which block the check
	U64 blocks = allInbetween & ~takenBB;

	// calculate pieces where the check is from by intersecting super attacks of the king with the enemy color's pieces
	U64 checkFrom = (superAttacks[0] & (piecesBB[ROOK + eColor] | piecesBB[QUEEN + eColor]))
		| (superAttacks[1] & (piecesBB[BISHOP + eColor] | piecesBB[QUEEN + eColor]))
		| (BB::knightAttacks(piecesBB[KING + color]) & piecesBB[KNIGHT + eColor])
		| (BB::pawnAnyAttacks(piecesBB[KING + color], color) & piecesBB[PAWN + eColor]);

	// save null if it's a check or if it's a double check by using signed shifts to avoid branches
	I64 nullIfCheck = ((I64)(anyAttacks & piecesBB[KING + color]) - 1) >> 63;
	I64 nullIfDblCheck = ((I64)(checkFrom & (checkFrom - 1)) - 1) >> 63;

	// save checking information for later
	isCheck = nullIfCheck == 0;

	// get pieces where the turn color's pieces can move to avoid checks
	U64 checkTo = checkFrom | blocks | nullIfCheck;

	// full bitboard if not only captures, create capture mask
	U64 nullIfCaptures = ~U64(0) * !onlyCaptures;
	U64 captureMask = nullIfCaptures | colorBB[eColor];
	
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
	U64 targets = colorBB[eColor] & targetMask;
	U64 pawns;

	for (int dir : {WEST, EAST})
	{
		// add pawn attacks in east and west of pawns which aren't pinned in that direction
		pawns = piecesBB[color + PAWN] & ~(allInbetween ^ inBetween[dirToIndex[pawnDir + dir] / 2]);
		moveTargets[dirToIndex[pawnDir + dir]] |= BB::shiftTwo(pawns, pawnDir + dir) & targets;
	}
 
	// add pawn pushes and double pawn pushes for pawns that aren't pinned not vertically
	pawns = piecesBB[color + PAWN] & ~(allInbetween ^ inBetween[1]);
	U64 pawnPushes = BB::shiftTwo(pawns, pawnDir) & ~takenBB;
	moveTargets[pawnDirIndex] |= pawnPushes & targetMask;
	
	U64 rank4 = color == WHITE ? U64(0x000000FF00000000) : U64(0x00000000FF000000);
	moveTargets[pawnDirIndex] |= BB::shiftTwo(pawnPushes, pawnDir) & ~takenBB & targetMask & rank4;

	// if en passant could be possible
	if (enPassant != -1)
	{
		// get empty square bitboard without pawn which is attackes by en passant
		U64 emptyWithoutPawn = ~(takenBB ^ (U64(1) << (enPassant - pawnDir)));
		U64 inBetweenHor = U64(0);

		for (int dir : {WEST, EAST})
		{
			// get horizontal enemy queen and rook attacks, get attacks in opposite direction of king, save intersection
			U64 attacks = BB::rayAttacks(piecesBB[ROOK + eColor] | piecesBB[QUEEN + eColor], emptyWithoutPawn, dir);
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
			moveTargets[dirToIndex[pawnDir + dir]] |= BB::shiftTwo(pawns, pawnDir + dir) & targets;
		}
	}

	// move king in all possible directions which aren't attacked
	targetMask = ~(colorBB[color] | anyAttacks) & captureMask;
	U64 king = piecesBB[color + KING];
	for (int i = 0; i < 8; i++)
	{
		moveTargets[i] |= BB::shiftTwo(king, dirs[i]) & targetMask;
	}

	// don't castle if in check
	king &= nullIfCheck;

	// check if spaces between king and rook are taken or in check and that castling rights aren't taken, if not add castles
	targetMask = ~(takenBB | anyAttacks);
	U64 eastCastle = BB::shiftTwo(king, EAST) & targetMask & (~U64(0) * ((castlingRights[0] && (color == WHITE)) || (castlingRights[2] && (color == BLACK))));
	moveTargets[0] |= BB::shiftTwo(eastCastle, EAST) & targetMask & captureMask;
	U64 westCastle = BB::shiftTwo(king, WEST) & targetMask & (~U64(0) * ((castlingRights[1] && (color == WHITE)) || (castlingRights[3] && (color == BLACK))));
	moveTargets[1] |= BB::shiftTwo(BB::shiftTwo(westCastle, WEST + WEST) & ~takenBB, EAST) & targetMask & captureMask;
	
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
			
			Move move = Move::loadFromSquares(source, target, piecesMB);
			int pieceColor = Piece::colorOf(piecesMB[source]);

			if (move.promotion != EMPTY)
			{
				// add all promotion pieces as moves
				for (int piece : {QUEEN, ROOK, BISHOP, KNIGHT})
				{
					move.promotion = piece + pieceColor;
					moveList.push_back(move);
				}
			}
			else
			{
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
			moveList.push_back(Move::loadFromSquares(source, target, piecesMB));
		}
	}
}

// check if there's a draw (except stalemate)
bool Board::checkDraw()
{
	// end the game on a draw if 50-move-rule
	if (halfMoveClock >= 100)
	{
		return true;
	}
	// end the game on a draw if repetition
	else if (std::count(previousPositions.begin(), previousPositions.end(), zobrist.getHashKey()) == 2)
	{
		return true;
	}
	// check for insufficient material
	else
	{
		// only check if there aren't any queens, rooks or pawns
		if (pieceLists[WHITE + QUEEN].getCount() == 0 && pieceLists[BLACK + QUEEN].getCount() == 0 &&
			(pieceLists[WHITE + ROOK].getCount() == 0 && pieceLists[BLACK + ROOK].getCount() == 0 &&
				pieceLists[WHITE + PAWN].getCount() == 0 && pieceLists[BLACK + PAWN].getCount() == 0))
		{
			// if there are only two kings, end game on draw
			if (pieceLists[WHITE + KNIGHT].getCount() == 0 && pieceLists[WHITE + BISHOP].getCount() == 0 &&
				(pieceLists[BLACK + KNIGHT].getCount() == 0 && pieceLists[BLACK + BISHOP].getCount() == 0))
			{
				return true;
			}

			// get counts for knights and bishops
			int knights[2] = { pieceLists[WHITE + KNIGHT].getCount(), pieceLists[BLACK + KNIGHT].getCount() };
			int bishops[2] = { pieceLists[WHITE + BISHOP].getCount(), pieceLists[BLACK + BISHOP].getCount() };

			// if both colors have one bishop each and there are no other minor pieces, end game on a draw if both bishops are on the same color
			if (knights[WHITE] == 0 && bishops[WHITE] == 1 && knights[BLACK] == 0 && bishops[BLACK] == 1)
			{
				if (Square::isLight(pieceLists[WHITE + BISHOP][0]) == Square::isLight(pieceLists[BLACK + BISHOP][0]))
				{
					return true;
				}
			}

			// check for both colors
			for (int col = 0; col < 2; col++)
			{
				// if this color has only one knight and there are no other minor pieces, end game on a draw
				if (knights[col] == 1 && bishops[col] == 0 && knights[!col] == 0 && bishops[!col] == 0)
				{
					return true;
				}

				// if this color has only one bishop and there are no other minor pieces, end game on a draw
				if (knights[col] == 0 && bishops[col] == 1 && knights[!col] == 0 && bishops[!col] == 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

// get game state
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

	// check for draws other than stalemate
	if (checkDraw())
	{
		return DRAW;
	}

	// if there are no losses or draws, return normal game state
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

// return move count
int Board::getMoveCount()
{
	return moveCount;
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

// return zobrist key
U64 Board::getZobristKey()
{
	return zobrist.getHashKey();
}

// return piece lists
PieceList* Board::getPieceLists()
{
	return pieceLists;
}

// return the possible move list
std::vector<Move> Board::getMoveList()
{
	return moveList;
}

// return move history
std::vector<Move> Board::getMoveHistory()
{
	return moveHistory;
}

// return if game started normally
bool Board::getNormalStart()
{
	return normalStart;
}