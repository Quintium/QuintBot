#pragma once

#include <iostream>
#include <fstream>
#include "Board.h"

// class for a node in the game
class Node
{
	std::string move;
	std::vector<Node> nodes;

public:
	Node(std::string data)
	{
		// strip off "{" and "}"
		data = data.substr(1, data.size() - 1);

		// find the first comma and set the move to the text before it
		size_t comma = data.find(",");
		move = data.substr(0, comma);

		// loop through the list of objects
		size_t objectStart = comma + 2;
		while (data[objectStart] != ']')
		{
			// find the end of the object
			size_t objectEnd = objectStart + 1;
			int openBraces = 1;
			while (openBraces > 0)
			{
				if (data[objectEnd] == '{')
				{
					openBraces++;
				}
				else if (data[objectEnd] == '}')
				{
					openBraces--;
				}
				objectEnd++;
			}

			// create a node with that object string and add it to the nodes
			
			nodes.push_back(Node(data.substr(objectStart, objectEnd)));

			objectStart = objectEnd + 1;
		}
	}

	// get node after moves
	std::optional<Node> getNode(std::vector<Move> moves)
	{
		if (moves.size() == 0)
		{
			return *this;
		}

		for (Node node : nodes)
		{
			if (node.move == moves[0].getNotation() && node.nodes.size() > 0)
			{
				moves.erase(moves.begin());
				return node.getNode(moves);
			}
		}

		return std::optional<Node>();
	}

	std::string randomMove()
	{
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::mt19937 generator(seed);
		std::uniform_int_distribution distribution(0, (int)nodes.size() - 1);
		return nodes[distribution(generator)].move;
	}
};

class Openings : public Node
{
public:
	Openings(std::string data) : Node(data)
	{
	}

	// load openings from file
	static Openings loadOpenings()
	{
		std::ifstream file("data/opening_database.od");
		std::string data;
		file >> data;
		file.close();

		return Openings(data);
	}
};