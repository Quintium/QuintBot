#pragma once

#include <iostream>
#include <fstream>
#include "Board.h"

// class for a node in the game
class Node
{
	std::string move;
	int games;
	std::vector<Node> nodes;

public:
	Node(std::string data)
	{
		// strip off "{" and "}"
		data = data.substr(1, data.size() - 1);

		// find the first comma and set the move to the text before it
		size_t comma = data.find(",");
		move = data.substr(0, comma);

		// find the second comma and set the games to the text before it
		size_t comma2 = data.find(",", comma + 1);
		games = std::stoi(data.substr(comma + 1, comma2));

		// loop through the list of objects
		size_t objectStart = comma2 + 2;
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
		if (nodes.size() == 0)
		{
			return std::optional<Node>();
		}

		if (moves.size() == 0)
		{
			return *this;
		}

		for (Node node : nodes)
		{
			if (node.move == moves[0].getNotation())
			{
				moves.erase(moves.begin());
				return node.getNode(moves);
			}
		}

		return std::optional<Node>();
	}

	std::string randomMove()
	{
		unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
		std::mt19937 generator(seed);
		std::uniform_real_distribution<double> distribution(0, 1);
		double random = distribution(generator);

		int sum = 0;
		for (Node node : nodes)
		{
			sum += node.games;
		}

		double subtotal = 0;
		for (Node node : nodes)
		{
			subtotal += ((double)node.games) / sum;
			if (subtotal >= random)
			{
				return node.move;
			}
		}
	}
};

class Openings : public Node
{
public:
	Openings(std::string data) : Node(data)
	{
	}

	// load openings from file
	static Openings loadOpenings(std::string assetsPath)
	{
		// load text from file
		std::ifstream file(assetsPath + "/Data/opening_database.od");
		std::string data;
		file >> data;
		file.close();

		if (data == "")
		{
			// load empty openings if can't load file
			return Openings("{,0,[]}");
		}
		else
		{
			// load openings from file
			return Openings(data);
		}
	}
};