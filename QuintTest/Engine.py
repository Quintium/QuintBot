import chess, chess.engine

class Engine:
    name: str
    args: list

    def __init__(self, name, args):
        self.name = name
        self.args = args

    def fullName(self) -> str:
        return (self.name[:-4] if self.name.endswith(".exe") else self.name) + "_" + "_".join([str(arg) for arg in self.args])

    def createProcess(self) -> chess.engine.SimpleEngine:
        return chess.engine.SimpleEngine.popen_uci(["engines/" + self.name] + [str(arg) for arg in self.args])