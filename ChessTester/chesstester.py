import chess, chess.engine
from random import randint

def nameFromPath(path):
    return path[max(path.rfind("\\"), path.rfind("/"))+1:path.rindex(".exe")]

paths = [r"D:\Coding\Python\ChessTester\engines\ChessAI1.exe",r"D:\Coding\Python\ChessTester\engines\ChessAI_materialonly.exe"]
games = 50
timeLimit = 0.1

engines = [chess.engine.SimpleEngine.popen_uci(path) for path in paths]
scores = [0, 0]

board = chess.Board()

for i in range(games):
    board.reset()
    game = []
    
    while (board.outcome(claim_draw=True) == None):
        engine_nr = board.turn if i % 2 == 0 else not board.turn
        try:
            result = engines[engine_nr].play(board, chess.engine.Limit(time=timeLimit))
        except chess.engine.EngineError as error:
            print("Error \"" + str(error) + "\" occured in engine " + nameFromPath(paths[engine_nr]) + " after playing the moves: " + str(game))

            for engine in engines:
                print("Closing engine")
                engine.close()
            quit()
            
        board.push(result.move)
        game.append(result.move.uci())

    winner = board.outcome(claim_draw=True).winner

    if winner != None:
        scores[winner if i % 2 == 0 else not winner] += 1
    else:
        scores[0] += 0.5
        scores[1] += 0.5

    print(str(i + 1) + " games played already.")
    for i in range(2):
        print("Engine " + nameFromPath(paths[i]) + " has won " + str(scores[i]) + " games.")

for engine in engines:
    print("Closing engine")
    engine.close()