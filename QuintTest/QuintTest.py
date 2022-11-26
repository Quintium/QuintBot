import argparse
import Test

if __name__ == "__main__":
    arger = argparse.ArgumentParser()

    subparsers = arger.add_subparsers(dest="command")
    
    testParser = subparsers.add_parser("test")
    testParser.add_argument("engineNames", nargs=2, type=str)
    testParser.add_argument("-g", "--games", type=int, required=True, dest="games")
    testParser.add_argument("-t", "--time", type=float, required=True, dest="timeLimit")
    testParser.add_argument("-p", "--proc", default=None, type=int,  dest="processes")
    testParser.add_argument("-o", "--out", default=None, type=str,  dest="outputPath")
    options = arger.parse_args()

    if options.command == "test":
        Test.test(options.engineNames, options.games, options.timeLimit, options.processes, options.outputPath)