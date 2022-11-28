# QuintBot
QuintBot is an open-source chess engine developed in C++ and inspired by the video [Coding Adventure: Chess AI](https://www.youtube.com/watch?v=U4ogK0MIzqk) by Sebastian Lague.
The engine can be played on lichess.org under the username [@QuintBot](https://lichess.org/@/QuintBot). It runs on 64-bit processors and is OS-independent.
This project is licensed under the MIT license (see LICENSE.txt), which means that you are free to copy, modify and distribute code from this repository without any restrictions, however when copying substantial portions of this program, you have to include LICENSE.txt in the source code.
Thanks to the [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) for providing knowledge about engine development.

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
- better KeyboardInterrupt handling
- total games printing
- stop testing if los is too low
- automated rough/fine tuning
- determine right amount of processes and time control
- more variation

### Infrastructure
- Create new release/test new changes
- Better error handling
- Update GUI branch
- fix rate limit problem (currently increased rate limiting delay)
- long-term Raspberry Pi set-up
- UCI settings
- List engine on CCRL

### Formatting
- Comment QuintTest/QuintOpenings
- pass board reference insead of piecesMB
- Use std::array/std::vector instead of C-style array
- Convert pointers to smart pointers or pass by reference
- Remove over-commenting

### Issues
- fix rate limit bug in lichess-bot
- bug in "6k1/3R4/4K3/8/2r5/8/8/8 b - - 93 124": sudden slow down in depths
- remove resetting of transposition table (or make it more efficient)
- repetitions in transposition table
- decrease PV overwrites