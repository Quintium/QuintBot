# QuintBot
QuintBot is an open-source chess engine developed in C++.
The engine can be played on lichess.org under the username [@QuintBot](https://lichess.org/@/QuintBot).
This project is licensed under the MIT license (see LICENSE.txt), which means that you are free to copy, modify and distribute code from this repository without any restrictions, however when copying substantial portions of this program, you have to include LICENSE.txt in the source code.
The creation of QuintBot was inspired by the video [Coding Adventure: Chess AI](https://www.youtube.com/watch?v=U4ogK0MIzqk) by Sebastian Lague.
Thanks to the [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) for providing knowledge about engine development.

## Features
- Bitboard-based direction-wise move generation based on [DirGolem](https://www.chessprogramming.org/DirGolem)
- Full implementation of chess rules including, repetitions, 50-move-rules and insufficient material
- Alpha-beta search
- Quiescence search
- Transposition table
- Implementation of UCI protocol
- Included simple GUI (outdated branch)

## Opening database
The opening database was created using QuintOpenings.
- Website: lichess.org
- Time period: May 2017
- Amount of games: 300000
- Elo limit: >1600
- Time control: any
- Frequency in position: >10%
- Frequency overall: >0.03%
- Ply limit: <10