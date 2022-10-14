import chess, chess.engine, math
from random import randint

def nameFromPath(path):
    return path[max(path.rfind("\\"), path.rfind("/"))+1:path.rindex(".exe")]

engineNames = ["QuintBot_original", "QuintBot_pawnstorm40"]
engineFolder = "Engines/"   
paths = [engineFolder + engineNames[i] + ".exe" for i in range(2)]
games = 300
timeLimit = 0.03

engines = [chess.engine.SimpleEngine.popen_uci(path) for path in paths]
scores = [0, 0]

board = chess.Board()

for i in range(games):
    board.reset()
    game = []
    
    while (board.outcome(claim_draw=True) == None):
        engineNr = board.turn if i % 2 == 0 else not board.turn
        try:
            result = engines[engineNr].play(board, chess.engine.Limit(time=timeLimit))
        except chess.engine.EngineError as error:
            print(f"Error \"{error}\" occured in engine {engineNames[engineNr]} after playing the moves: {game}")

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

    print(f"{i + 1} games played already.")
    for i in range(2):
        print(f"Engine {engineNames[i]} has won {scores[i]} games.")

for engine in engines:
    print("Closing engine")
    engine.close()

expectedScore = scores[0] / games
if expectedScore == 0:
    eloDifference = 400
elif expectedScore == 1:
    eloDifference = -400
else:
    eloDifference = 400 * math.log10(1 / (scores[0] / games) - 1)
eloString = ("+" if eloDifference > 0 else "") + str(round(eloDifference, 2))

print(f"Time control: {timeLimit}s per move")
print(f"Amount of games: {300}")
print(f"Engine names: {engineNames[0]} - {engineNames[1]}")
print(f"Final score: {scores[0]} - {scores[1]}")
print(f"Elo difference: {eloString}")