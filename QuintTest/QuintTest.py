import chess, chess.engine, time
from multiprocessing import Pool, Manager, Value
from Results import Results

def nameFromPath(path):
    return path[max(path.rfind("\\"), path.rfind("/"))+1:path.rindex(".exe")]

def playGames(games: int, timeLimit: float, engineNames: list, enginePaths: list, player1Wins: Value, player2Wins: Value, draws: Value):
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
            if winner == i % 2:
                player1Wins.value += 1
            else:
                player2Wins.value += 1
        else:
            draws.value += 1

        print(f"{player1Wins.value + draws.value + player2Wins.value} games played, current score - {engineNames[0]} wins: {player1Wins.value}; draws: {draws.value}; {engineNames[1]} wins: {player2Wins.value}")
    
    for engine in engines:
        engine.close()

if __name__ == "__main__":
    engineNames = ["original", "original"]
    engineFolder = "Engines/"   
    enginePaths = [engineFolder + engineNames[i] + ".exe" for i in range(2)]
    games = 100
    chunkSize = 10
    processes = 10
    timeLimit = 0.1

    manager = Manager()
    player1Wins = manager.Value("i", 0)
    player2Wins = manager.Value("i", 0)
    draws = manager.Value("i", 0)

    start = time.time()
    
    inputs = [(chunkSize, timeLimit, engineNames, enginePaths, player1Wins, player2Wins, draws)] * int(games / chunkSize)
    with Pool(processes) as pool:
         pool.starmap(playGames, inputs)

    timePassed = round(time.time() - start, 2)
    results = Results(player1Wins.value, player2Wins.value, draws.value)

    print(f"Time spent: {timePassed}s")
    print(f"Time control: {timeLimit}s per move")
    print(f"Amount of games: {results.gameAmount()}")
    print(f"Final score - {engineNames[0]} wins: {results.player1Wins}; draws: {results.draws}; {engineNames[1]} wins: {results.player2Wins}")
    print(f"Elo difference: {results.eloDifferenceString()}")
    print(f"Likelihood of superiority: {round(results.los() * 100, 2)}%")