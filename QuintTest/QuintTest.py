import chess, chess.engine, time, itertools, multiprocessing, multiprocessing.pool
from multiprocessing import Pool, Manager, Value
from Results import Results
from Engine import Engine

def playGames(gamesToPlay: int, timeLimit: float, engines: list, stop: Value, gamesPlayed: int) -> tuple:
    results = Results(engines[0], engines[1], 0, 0, 0)

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
                    print(f"Error \"{error}\" occured in engine {engines[engineNr].fullName()} after playing the moves: {game}")

                    for engineProcess in engineProcesses:
                        engineProcess.close()
                    quit()
                    
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
            gamesPlayed.value += 1

            print(f"Game played: {engines[0].fullName()} vs. {engines[1].fullName()}; {gamesPlayed.value} games played so far")

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

def calculateTaskSize(gamesPerMatch: int, matchAmount: int, processes: int):
    for n in range(1, gamesPerMatch):
        taskSize = gamesPerMatch / n
        if gamesPerMatch % n == 0 and taskSize % 2 == 0 and taskSize * processes <= gamesPerMatch * matchAmount: # task size has to be divisor, even and program should take more than one cycle
            return round(taskSize)

    raise RuntimeError("No fitting task size found")

def log(message: str):
    print("\n" + message, end="")
    with open("QuintTest.out", "a") as file:
        file.write(message)

def matchFinished(resultList: list):
    global totalGamesPlayed

    results = Results.sum(resultList)
    totalGamesPlayed += results.gameAmount()
    log(results.statString(timeLimit, totalGamesPlayed))

if __name__ == "__main__":
    processes = int(multiprocessing.cpu_count() / 2)
    timeLimit = 0.1
    gamesPerMatch = 3000

    originalEngine = Engine("pool_infrastructure.exe", [])
    opponentParameters = []
    opponentEngines = [Engine("new.exe", comb) for comb in itertools.product(*opponentParameters)]
    matchAmount = len(opponentEngines)
    taskSize = calculateTaskSize(gamesPerMatch, matchAmount, processes)

    open("QuintTest.out", "w").close()

    message = ""
    message += f"Pairing {originalEngine.fullName()} against {matchAmount} engines\n"
    message += f"Opponents: {opponentEngines[0].name} with parameters {opponentParameters}\n"
    message += f"Time control: {timeLimit}s per move\n"
    message += f"Amount of games per match: {gamesPerMatch}\n"
    message += f"Task size: {taskSize}\n"
    message += f"Total amount of games: {gamesPerMatch * matchAmount}\n"
    message += "\n"
    log(message)

    start = time.time()
    manager = Manager()
    stop = manager.Value("i", 0)
    individualGamesPlayed = manager.Value("i", 0)
    
    totalGamesPlayed = 0
    with Pool(processes) as pool:
        tasks = []
        for opponentEngine in opponentEngines:
            inputs = [(taskSize, timeLimit, [originalEngine, opponentEngine], stop, individualGamesPlayed)] * round(gamesPerMatch / taskSize)
            tasks.append(pool.starmap_async(playGames, inputs, None, matchFinished))

        try:
            for task in tasks:
                task.wait()
        except Exception as err:
            print(Exception, err)
            stop.value = 1

    timePassed = round(time.time() - start, 2)

    message = ""
    message += "All engine matches finished\n"
    message += f"{originalEngine.fullName()} played against {matchAmount} engines\n"
    message += f"Opponents: {opponentEngines[0].name} with parameters {opponentParameters}\n"
    message += f"Time spent: {timePassed}s\n"
    message += f"Time control: {timeLimit}s per move\n"
    message += f"Amount of games per match: {gamesPerMatch}\n"
    message += f"Total amount of games: {gamesPerMatch * matchAmount}\n"
    log(message)