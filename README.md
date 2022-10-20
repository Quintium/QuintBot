# QuintBot
QuintBot is a chess engine developed in C++ and inspired by the video [Coding Adventure: Chess AI](https://www.youtube.com/watch?v=U4ogK0MIzqk) by Sebastian Lague.
The engine can be played on lichess.org under the username [@QuintBot](https://lichess.org/@/QuintBot).
This project is licensed under the MIT license (see LICENSE.txt), which means that you are free to copy, modify and distribute code from this repository without any restrictions.
Thanks to the [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) for providing knowledge about engine development.

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
	- Lower TT entry space
    - Aspiration windows
	- PVS/NegaScout
	- Null move pruning
	- Profiling
	- Remove branching
	- Better move ordering
	- Killer heuristic
	- History heuristic
	- Multi-threading

- Evaluation:
	- **Tuning**
	- Mobility
	- Space

### Issues
- don't abort search if quicker mate is available
- manual move overhead to decrease flagging because of server issues, remove move overhead in lichess-bot
- fix rate limit bug in lichess-bot
- decrease PV overwrites

### Infrastructure
- QuintTest improvements: statistical outputs, multiprocessing pools with chunks, automated tuning (Google Cloud CPU?)
- long-term Raspberry Pi set-up
- UCI settings
- binary releases on GitHub
- List engine on CCRL

### Formatting
- Remove over-commenting
