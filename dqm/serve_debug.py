import os
import sys
import inspect


if __name__ == "__main__":

    # Directory path
    this_folder = os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0]))
    print(this_folder)

    source_code_dir = os.path.join(this_folder, "..")
    sys.path.insert(0, source_code_dir)
    
    from app import app

    # run application
    app.run(debug=True)
