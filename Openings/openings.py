import chess
import chess.pgn

class Node:
    move: str
    nodes: list
    games: int
    ply: int

    def __init__(self, move: str, ply: int):
        self.move = move
        self.nodes = []
        self.games = 0
        self.ply = ply
        
    def hasMove(self, move: str):
        return any([node.move == move for node in self.nodes])

    def addMove(self, move: str):
        newNode = Node(move, self.ply + 1)
        self.nodes.append(newNode)
        return newNode

    def getNode(self, move: str):
        return [node for node in self.nodes if node.move == move][0]

    def isRelevant(self, upperNode):
        return self.ply <= maxDepth and self.games / openings.games >= gameThreshold and self.games / upperNode.games >= probThreshold

    def keepRelevant(self):
        self.nodes = [node for node in self.nodes if node.isRelevant(self)]
        
        for node in self.nodes:
            node.keepRelevant()

    def toString(self):
        string = "{" + self.move + "," + str(self.games) + "," + "["
        
        for i in range(len(self.nodes)):
            string += self.nodes[i].toString() + ","
        
        string += "]}"
        
        return string
        

class Openings(Node):
    def __init__(self):
        super().__init__("", 0)

    def saveAs(self, fileName: str):
        database = open(fileName, "w")
        database.write(self.toString())
        database.close()
    
eloThreshold = 1600 # how much elo the players of a game have to be for it to be considered
timeThreshold = 0 # what time control the games has to be for it to be considered
probThreshold = 0.1 # how often a move has to be played in a position to be considered
gamesCount = 300000 # how many games the program will analyze
maxDepth = 10 # how deep the program will analyze

openings = Openings()

games = open("../../Lichess games/lichess_games_may_2017.pgn")

totalGames = 0
while (game := chess.pgn.read_game(games)) != None and openings.games < gamesCount:
    whiteElo = game.headers["WhiteElo"]
    blackElo = game.headers["BlackElo"]
    time = game.headers["TimeControl"]
    if time != "-":
        time = int(time[:time.index("+")])
    else:
        time = 1000000;

    if whiteElo != "?" and blackElo != "?" and int(whiteElo) > eloThreshold and int(blackElo) > eloThreshold and time > timeThreshold:        
        openings.games += 1
        node = openings
        ply = 1
        while not game.is_end() and ply <= 10:
            game = game.next()

            move = game.move.uci()
            if node.hasMove(move):
                node = node.getNode(move)
            else:
                node = node.addMove(move)

            node.games += 1
            ply += 1  
                    
        if openings.games % 100 == 0:
            print("Games added: " + str(openings.games))
            print("Total games: " + str(totalGames))
            
    totalGames += 1

games.close()

for i in range(1, 101):
    gameThreshold = i * 0.0001 # how much the ratio of games the opening has been played in must be for it to be considered
    openings.keepRelevant()
    openings.saveAs("opening_database_threshold_" + str(i) + ".od")

