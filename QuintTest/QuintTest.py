import chess, chess.engine, math
from multiprocessing import Process, Array, Queue

def nameFromPath(path):
    return path[max(path.rfind("\\"), path.rfind("/"))+1:path.rindex(".exe")]

def playGames(games, timeLimit, engineNames, enginePaths, results, stopQueue):
    engines = [chess.engine.SimpleEngine.popen_uci(path) for path in enginePaths]

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
            results[winner if i % 2 == 0 else not winner] += 1
        else:
            results[0] += 0.5
            results[1] += 0.5

        print(f"{int(results[0] + results[1])} games played already.")
        for i in range(2):
            print(f"Engine {engineNames[i]} has won {results[i]} games.")

        if not stopQueue.empty():
            break
    
    for engine in engines:
        print("Closing engine")
        engine.close()

if __name__ == "__main__":
    engineNames = ["QuintBot_original", "QuintBot_original"]
    engineFolder = "Engines/"   
    enginePaths = [engineFolder + engineNames[i] + ".exe" for i in range(2)]
    games = 400
    timeLimit = 0.2

    results = Array("f", [0, 0])
    stopQueue = Queue()

    processes = []
    processAmount = 8
    for i in range(processAmount):
        p = Process(target=playGames, args=(int(games / processAmount), timeLimit, engineNames, enginePaths, results, stopQueue))
        processes.append(p)
        p.start()

    try:
        for p in processes:
            p.join()
    except:
        stopQueue.put("stop")

    expectedScore = results[0] / (results[0] + results[1])
    if expectedScore == 0:
        eloDifference = 400
    elif expectedScore == 1:
        eloDifference = -400
    else:
        eloDifference = 400 * math.log10(1 / expectedScore - 1)
    eloString = ("+" if eloDifference > 0 else "") + str(round(eloDifference, 2))

    print(f"Time control: {timeLimit}s per move")
    print(f"Amount of games: {int(results[0] + results[1])}")
    print(f"Engine names: {engineNames[0]} - {engineNames[1]}")
    print(f"Final score: {results[0]} - {results[1]}")
    print(f"Elo difference: {eloString}")