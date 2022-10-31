import chess, chess.engine, time, itertools, multiprocessing, multiprocessing.pool
from multiprocessing import Pool, Manager, Value
from Results import Results
from Engine import Engine

def playGames(gamesToPlay: int, timeLimit: float, engines: list, totalGamesPlayed: Value, player1Wins: Value, player2Wins: Value, draws: Value, stop: Value) -> tuple:
    try:
        engineProcesses = [engine.createProcess() for engine in engines]
        board = chess.Board()

        for i in range(gamesToPlay):
            board.reset()
            game = []
            
            while (board.outcome(claim_draw=True) == None):
                engineNr = board.turn if i % 2 == 0 else not board.turn
                try:
                    result = engineProcesses[engineNr].play(board, chess.engine.Limit(time=timeLimit))
                except chess.engine.EngineError as error:
                    print(f"Error \"{error}\" occured in engine {engines[engineNr].fullName()} after playing the moves: {game}")

                    for engine in engines:
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
            totalGamesPlayed.value += 1

            print(f"{totalGamesPlayed.value} total games played; Now playing: {engines[0].fullName()} - {engines[1].fullName()}")
            print(f"{player1Wins.value + draws.value + player2Wins.value} games: {player1Wins.value} - {draws.value} - {player2Wins.value}")
            print()

            if stop.value == 1:
                print("Aborting process...")
                break
    except KeyboardInterrupt:
        print("Aborting process...")
        stop.value = 1

    for engineProcess in engineProcesses:
        engineProcess.close()

    return (timeLimit, engines, player1Wins, player2Wins, draws)

def matchFinished(results):
    output = results[0]
    resultStats = Results(output[2].value, output[3].value, output[4].value)

    matchMessage = ""
    matchMessage += f"Engine match: {output[1][0].fullName()} vs {output[1][1].fullName()}:\n"
    matchMessage += f"Time Limit: {output[0]}\n"
    matchMessage += f"Games played: {resultStats.gameAmount()}\n"
    matchMessage += f"Final score: {resultStats.player1Wins} - {resultStats.draws} - {resultStats.player2Wins}\n"
    matchMessage += f"Elo difference: {resultStats.eloDifferenceString()}\n"
    matchMessage += f"Likelihood of superiority: {round(resultStats.los() * 100, 2)}%\n"
    matchMessage += "\n"

    print(matchMessage, end="")
    with open("QuintTest.out", "a") as file:
        file.write(matchMessage)

if __name__ == "__main__":
    originalEngine = Engine("materialargs.exe", [0, 0, 0, 0, 0])
    opponentParameters = [[0, 250, 500], [0, 250, 500], [0, 250, 500], [0, 250, 500], [0, 250, 500]]
    opponentEngines = [Engine("materialargs.exe", comb) for comb in itertools.product(*opponentParameters)]
    processes = multiprocessing.cpu_count()
    timeLimit = 0.1

    # games per engine and task size have to be even
    gamesPerEngine = 30
    found = False
    for n in range(1, gamesPerEngine):
        if gamesPerEngine % n == 0 and (gamesPerEngine / n) % 2 == 0 and gamesPerEngine / n <= gamesPerEngine * len(opponentEngines) / processes:
            taskAmount = n
            taskSize = round(gamesPerEngine / n)
            found = True
            break

    if not found:
        raise RuntimeError("No fitting task size found")

    manager = Manager()
    totalGamesPlayed = manager.Value("i", 0)

    open("QuintTest.out", "w").close()

    opponentsString = ", ".join([engine.fullName() for engine in opponentEngines])
    message = ""
    message += f"Pairing {originalEngine.fullName()} against {opponentsString}\n"
    message += f"Time control: {timeLimit}s per move\n"
    message += f"Amount of games per engine: {gamesPerEngine}\n"
    message += f"Task size: {taskSize}\n"
    message += "\n"
    print(message, end="")
    with open("QuintTest.out", "a") as file:
        file.write(message)

    start = time.time()
    stop = manager.Value("i", 0)
    
    with Pool(processes) as pool:
        tasks = []
        results = []
        for opponentEngine in opponentEngines:
            player1Wins = manager.Value("i", 0)
            player2Wins = manager.Value("i", 0)
            draws = manager.Value("i", 0)
            results.append([player1Wins, player2Wins, draws]) # values have to be stored somewhere

            inputs = [(taskSize, timeLimit, [originalEngine, opponentEngine], totalGamesPlayed, player1Wins, player2Wins, draws, stop)] * taskAmount
            tasks.append(pool.starmap_async(playGames, inputs, None, matchFinished))

        try:
            for task in tasks:
                task.wait()
        except:
            stop.value = 1

    timePassed = round(time.time() - start, 2)

    message = ""
    message += "All engine matches finished\n"
    message += f"{originalEngine.fullName()} played against {opponentsString}\n"
    message += f"Time spent: {timePassed}s\n"
    message += f"Time control: {timeLimit}s per move\n"
    message += f"Task size: {taskSize}\n"
    message += f"Amount of games per engine: {gamesPerEngine}\n"
    message += f"Total amount of games: {gamesPerEngine * len(opponentEngines)}\n"
    
    print(message, end="")
    with open("QuintTest.out", "a") as file:
        file.write(message)