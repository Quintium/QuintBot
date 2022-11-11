# QuintBot
QuintBot is a chess engine developed in C++ and inspired by the video [Coding Adventure: Chess AI](https://www.youtube.com/watch?v=U4ogK0MIzqk) by Sebastian Lague.
The engine can be played on lichess.org under the username [@QuintBot](https://lichess.org/@/QuintBot).
This project is licensed under the MIT license (see LICENSE.txt), which means that you are free to copy, modify and distribute code from this repository without any restrictions, however when copying substantial portions of this program, you have to include LICENSE.txt in the source code.
Thanks to the [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) for providing knowledge about engine development.
This engine only runs on 64-bit processors and supports both Windows (master branch) and Linux (Linux branch)

## Features
- Bitboard-based direction-wise move generation based on [DirGolem](https://www.chessprogramming.org/DirGolem)
- Full implementation of chess rules including, repetitions, 50-move-rules and insufficient material
- Alpha-beta search
- Quiescence search
- Transposition table
- Self-made opening database from lichess games
- Implementation of UCI protocol
- Included simple GUI (outdated branch)

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
	- TT size
	- TT replacement strategy
    - Aspiration windows
	- PVS/NegaScout
	- Null move pruning
	- Profiling
	- Remove branching
	- Better move ordering
	- Killer heuristic
	- History heuristic
	- Mate distance pruning
	- Multi-threading (Lazy SMP)
	- Magic bitboard?

- Evaluation:
	- **Tuning**
	- Mobility
	- Space

### QuintTest
- stop testing if los is too low
- determine right amount of processes
- tweak minimum time control
- automated tuning
- realistic time control maybe?

### Issues
- remove resetting of transposition table (or make it more efficient)
- 50 move rule/repetitions in transposition table
- No redundant move information
- fix rate limit bug in lichess-bot
- decrease PV overwrites

### Infrastructure
- Update GUI branch
- fix rate limit problem (currently increased rate limiting delay)
- long-term Raspberry Pi set-up
- UCI settings
- List engine on CCRL

### Formatting
- Comment QuintTest/QuintOpenings
- Convert pointers to optionals, pass by reference
- Remove over-commenting