import math

from Engine import Engine

class Results:
    player1: Engine
    player2: Engine
    player1Wins: int
    player2Wins: int
    draws: int
    timeLimit: float

    def __init__(self, player1: Engine, player2: Engine, player1Wins: int, player2Wins: int, draws: int, timeLimit: float):
        self.player1 = player1
        self.player2 = player2
        self.player1Wins = player1Wins
        self.player2Wins = player2Wins
        self.draws = draws
        self.timeLimit = timeLimit

    def gameAmount(self) -> int:
        return self.player1Wins + self.player2Wins + self.draws

    def player1Score(self) -> int:
        return self.player1Wins + self.draws / 2

    def player2Score(self) -> int:
        return self.player2Wins + self.draws / 2

    def eloDifference(self) -> float:
        expectedScore = self.player1Score() / self.gameAmount()
        if expectedScore == 0:
            return 10000
        elif expectedScore == 1:
            return -10000
        else:
            return round(400 * math.log10(1 / expectedScore - 1), 2)

    def eloDifferenceString(self) -> str:   
        eloDiff = self.eloDifference()
        return ("+" if eloDiff > 0 else "") + str(eloDiff)

    def los(self) -> float: # Likelihood of superiority
        if self.player1Wins == self.player2Wins:
            return 50
        unroundedLos = 0.5 * (1 + math.erf((self.player2Wins - self.player1Wins) / math.sqrt(2 * (self.player1Wins + self.player2Wins))))
        return round(unroundedLos * 100, 2)

    def statString(self):
        message = ""
        message += f"Engine match: {self.player1.fullName()} vs {self.player2.fullName()}:\n"
        message += f"Time Limit: {self.timeLimit}\n"
        message += f"Games played: {self.gameAmount()}\n"
        message += f"Final score: {self.player1Wins} - {self.draws} - {self.player2Wins}\n"
        message += f"Elo difference: {self.eloDifferenceString()}\n"
        message += f"Likelihood of superiority: {self.los()}%\n"
        message += "\n"

        return message

    @staticmethod
    def sum(resultList: list):
        results = Results(resultList[0].player1, resultList[0].player2, 0, 0, 0, resultList[0].timeLimit)

        for result in resultList:
            results.player1Wins += result.player1Wins
            results.player2Wins += result.player2Wins
            results.draws += result.draws

        return results

