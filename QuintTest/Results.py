import math

class Results:
    player1Wins: int
    player2Wins: int
    draws: int

    def __init__(self, player1Wins, player2Wins, draws):
        self.player1Wins = player1Wins
        self.player2Wins = player2Wins
        self.draws = draws

    def gameAmount(self) -> int:
        return self.player1Wins + self.player2Wins + self.draws

    def player1Score(self) -> int:
        return self.player1Wins + self.draws / 2

    def player2Score(self) -> int:
        return self.player2Wins + self.draws / 2

    def eloDifferenceString(self) -> str:
        expectedScore = self.player1Score() / self.gameAmount()
        if expectedScore == 0:
            eloDifference = 600
        elif expectedScore == 1:
            eloDifference = -600
        else:
            eloDifference = 400 * math.log10(1 / expectedScore - 1)
        eloString = ("+" if eloDifference > 0 else "") + str(round(eloDifference, 2))

        return eloString

    def los(self) -> float: # Likelihood of superiority
        if self.player1Wins == self.player2Wins:
            return 0.5
        return 0.5 * (1 + math.erf((self.player2Wins - self.player1Wins) / math.sqrt(2 * (self.player1Wins + self.player2Wins))))
