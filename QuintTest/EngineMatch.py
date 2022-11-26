import chess, chess.engine
import multiprocessing, multiprocessing.pool
from multiprocessing import Pool, Manager, Value
from Results import Results
from Engine import Engine

def playGames(gamesToPlay: int, timeLimit: float, engines: list, stop: Value) -> tuple:
    results = Results(engines[0], engines[1], 0, 0, 0, timeLimit)

    try:
        engineProcesses = [engine.createProcess() for engine in engines]
        board = chess.Board()

        for i in range(gamesToPlay):
            board.reset()
            game = []
            whitePlayer = i % 2
            
            while (board.outcome(claim_draw=True) == None):
                engineNr = whitePlayer if board.turn == chess.WHITE else not whitePlayer
                try:
                    result = engineProcesses[engineNr].play(board, chess.engine.Limit(time=timeLimit))
                except chess.engine.EngineError as error:
                    gameString = " ".join(game)
                    print(f"Error '{error}' occured in engine {engines[engineNr].fullName()} after playing the moves: '{gameString}'")
                    print(f"Aborting process...")

                    stop.value = 1
                    for engineProcess in engineProcesses:
                        engineProcess.close()
                    return results
                    
                board.push(result.move)
                game.append(result.move.uci())

            winnerColor = board.outcome(claim_draw=True).winner

            if winnerColor == None:
                results.draws += 1
            else:
                if winnerColor == chess.WHITE:
                    winnerPlayer = whitePlayer
                else:
                    winnerPlayer = not whitePlayer

                if winnerPlayer == 0:
                    results.player1Wins += 1
                else:
                    results.player2Wins += 1

            print(f"Game played: {engines[0].fullName()} vs {engines[1].fullName()}")

            if stop.value == 1:
                print("Aborting process...")
                break
    except Exception as err:
        print(Exception, err)
        print("Aborting process...")
        stop.value = 1

    for engineProcess in engineProcesses:
        engineProcess.close()

    return results

def calculateTaskSize(games: int, processes: int):
    for n in range(1, games):
        taskSize = games / n
        if games % n == 0 and taskSize % 2 == 0 and taskSize * processes <= games: # task size has to be divisor, even and program should take more than one cycle
            return round(taskSize)

    raise RuntimeError("No fitting task size found, decrease number of processes or change to a more divisible number of games.")

def pairEngines(engines: list, games: int, timeLimit: float, processes: int):
    global pool
    if not processes:
        processes = int(multiprocessing.cpu_count() / 2)
    taskSize = calculateTaskSize(games, processes)

    manager = Manager()
    stop = manager.Value("i", 0)

    with Pool(processes) as pool:
        inputs = [(taskSize, timeLimit, engines, stop)] * round(games / taskSize)
        try:
            results = Results.sum(pool.starmap(playGames, inputs))
        except Exception as err:
            print(Exception, err)
            stop.value = 1

    return results