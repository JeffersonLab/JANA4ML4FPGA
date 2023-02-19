import datetime
import shlex
from datetime import datetime
import subprocess
import os
import shutil

class ConsoleRunSink:

    def __init__(self):
        self.to_show = []

    def add_line(self, line):
        print(line)

    @property
    def is_displayed(self):
        return True


default_sink = ConsoleRunSink()


def run(command, sink=default_sink, cwd=None, shell=False, retval_raise=False):
    """Wrapper around subprocess.Popen that returns:

    :return retval, start_time, end_time, lines
    """
    if isinstance(command, str):
        command = shlex.split(command)

    # Pretty header for the command
    sink.add_line('=' * 20)
    sink.add_line("CWD: " + cwd if cwd else os.getcwd())
    sink.add_line("RUN: " + " ".join(command))
    sink.add_line('=' * 20)

    # Record the start time
    start_time = datetime.now()
    lines = []

    # stderr is redirected to STDOUT (and it then redirected to PIPE) because otherwise it needs special handling
    # we don't need it and we don't care as C++ warnings generate too much stderr
    # which makes it pretty much like stdout
    with subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd, shell=shell) as process:
        while True:
            line = process.stdout.readline().decode('latin-1').replace('\r', '\n')

            if process.poll() is not None and line == '':
                break
            if line:
                if line.endswith('\n'):
                    line = line[:-1]

                sink.add_line(line)
                lines.append(line)

        # Get return value and finishing time
        retval = process.poll()

    end_time = datetime.now()

    sink.add_line("------------------------------------------")
    sink.add_line(f"RUN DONE. RETVAL: {retval} \n\n")
    if retval != 0:
        sink.add_line(f"ERROR. Retval is not 0. Plese, look at the logs\n")

        if retval_raise:
            raise RuntimeError("ERROR. Retval is not 0. Plese, look at the logs")

    sink.done()
    return retval, start_time, end_time, lines


def main():
    jana_path = shutil.which("jana")
    print(f"Using JANA2 executable:\n {jana_path}")
    run([
        jana_path,

    ])



if __name__ == "__main__":
    main()
