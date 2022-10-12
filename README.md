# QuintBot
QuintBot is a chess engine developed in C++ and inspired by the video [Coding Adventure: Chess AI](https://www.youtube.com/watch?v=U4ogK0MIzqk) by Sebastian Lague.
The engine can be played on lichess.org under the username [@QuintBot](https://lichess.org/@/QuintBot).

## Features
- Bitboard-based direction-wise move generation based on [DirGolem](https://www.chessprogramming.org/DirGolem)
- Full implementation of chess rules including, repetitions, 50-move-rules and insufficient material
- Alpha-beta search
- Quiescence search
- Transposition table
- Self-made opening database from lichess games
- Implementation of UCI protocol
- Included simple GUI

## Opening database
The opening database was created by filtering openings played in online chess.
- Website: lichess.org
- Time period: May 2017
- Amount of games: 300000
- Elo limit: >1600
- Time control: any
- Frequency in position: >10%
- Frequency overall: >0.03%
- Ply limit: <10


## To-do
### Engine strength
- Improve time management
- Pondering
- Optimization:
	- Null move pruning
	- Killer heuristic
	- Multi-threading

- Evaluation:
	- Positional eval (pawn structure, king safety, good/bad bishop and knight)
	- Mobility
	- Space

### Infrastructure
- Host lichess bot on Raspberry Pi
- UCI settings
- Improve ChessTester to meausure improvements: time controls
- List engine on CCRL
