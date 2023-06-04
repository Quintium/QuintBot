#pragma once
#include <optional>
#include "Board.h"

// class for a node in the game
class Node
{
	std::string move; // move the node corresponds to
	int games; // amount of games the move was featured in
	std::vector<Node> childNodes;

public:
	Node(std::string data);

	// find node after certain moves
	std::optional<Node> findNode(std::vector<Move> moves);

	// generate move that is played after this node
	std::string randomMove();
};

// class for opening database (root node)
class Openings : public Node
{
public:
	Openings(std::string data);
	static Openings loadOpenings();
};