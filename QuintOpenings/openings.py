import chess
import chess.pgn

# class for an opening node/position
class Node:
    move: str # move that leads to this node
    nodes: list # child nodes
    games: int # how many games reached this node
    ply: int # how many ply the node is from root

    # create empty node
    def __init__(self, move: str, ply: int):
        self.move = move
        self.nodes = []
        self.games = 0
        self.ply = ply
        
    # check if move is available in this position
    def hasMove(self, move: str):
        return any([node.move == move for node in self.nodes])

    # add new child node
    def addMove(self, move: str):
        newNode = Node(move, self.ply + 1)
        self.nodes.append(newNode)
        return newNode

    # get child node with certain move
    def getNode(self, move: str):
        return [node for node in self.nodes if node.move == move][0]

    # check if opening node is significant enough to be included:
    #   - only up to maxDepth
    #   - with a minimum amount of games
    #   - with a large amount probability of being played from the parent node
    def isRelevant(self, upperNode):
        return self.ply <= maxDepth and self.games / openings.games >= gameThreshold and self.games / upperNode.games >= probThreshold

    # keep all nodes that are significant enough
    def keepRelevant(self):
        self.nodes = [node for node in self.nodes if node.isRelevant(self)]
        
        for node in self.nodes:
            node.keepRelevant()

    # writing node to a string with the layout "{move, games, [{node1}, {node2}, ...]}"
    def toString(self):
        string = "{" + self.move + "," + str(self.games) + "," + "["
        
        for i in range(len(self.nodes)):
            string += self.nodes[i].toString() + ","
        
        string += "]}"
        
        return string
        
# class for the opening tree (the root node)
class Openings(Node):
    def __init__(self):
        super().__init__("", 0)

    def saveAs(self, fileName: str):
        database = open(fileName, "w")
        database.write(self.toString())
        database.close()
    
eloThreshold = 1600 # minimum elo of players of the games
timeThreshold = 0 # minimum time control of the games
probThreshold = 0.1 # how often a move has to be played in a position to be considered
gamesCount = 300000 # how many games the program will analyze
maxDepth = 10 # how deep the program will analyze

openings = Openings()
filePath = "D:/Coding/Lichess games/lichess_games_may_2017.pgn"

# loop through given amount of games in given file
games = open(filePath)
totalGames = 0
while (game := chess.pgn.read_game(games)) != None and openings.games < gamesCount:
    # get elo and time control from pgn
    whiteElo = game.headers["WhiteElo"]
    blackElo = game.headers["BlackElo"]
    time = game.headers["TimeControl"]
    if time != "-":
        time = int(time[:time.index("+")])
    else:
        time = 1000000

    # skip pgn if any thresholds fail
    if whiteElo == "?" or blackElo == "?" or int(whiteElo) < eloThreshold or int(blackElo) < eloThreshold or time < timeThreshold:   
        continue

    openings.games += 1
    node = openings
    ply = 1

    # traverse game until the given depth
    while not game.is_end() and ply <= maxDepth:
        game = game.next()

        # get/add next node
        move = game.move.uci()
        if node.hasMove(move):
            node = node.getNode(move)
        else:
            node = node.addMove(move)

        node.games += 1
        ply += 1  
                
    # print current progress every 100 games
    if openings.games % 100 == 0:
        print("Games added: " + str(openings.games))
        print("Total games: " + str(totalGames))
            
    totalGames += 1

games.close()

# adjust game threshold to obtain different opening trees
for i in range(1, 101):
    gameThreshold = i * 0.0001
    openings.keepRelevant()
    openings.saveAs("opening_database_threshold_" + str(i) + ".od")

