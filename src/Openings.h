#pragma once
#include <optional>
#include "Board.h"

// class for a node in the game
class Node
{
	std::string move; // move the node corresponds to
	int games; // amount of games the move was featured in
	std::vector<Node> childNodes; // child nodes

public:
	// constructor
	Node(std::string data);

	// find node after certain moves
	std::optional<Node> findNode(std::vector<Move> moves);

	// generate reandom move
	std::string randomMove();
};

// class for opening database (root node)
class Openings : public Node
{
public:
	// constructor
	Openings(std::string data);

	// load openings from string
	static Openings loadOpenings();
};