import argparse
import os
import sys
import inspect
import uproot


if __name__ == "__main__":

    # Directory path
    this_folder = os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0]))
    print(this_folder)

    source_code_dir = os.path.join(this_folder, "")
    sys.path.insert(0, source_code_dir)

    from pydqm.app import app

    parser = argparse.ArgumentParser(description='Process some *.root files.')
    parser.add_argument('filepath', help='The path to a .root file')
    parser.add_argument('--debug', action='store_false', help='Include this flag to enable debug mode')

    args = parser.parse_args()

    print(f"File path is: {args.filepath}")
    root_file = uproot.open(args.filepath)
    print(f"File keys: {root_file.items()}")


    # run application
    app.config["ROOT_FILE_PATH"] = args.filepath
    app.run(debug=args.debug)
