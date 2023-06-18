#!/usr/bin/env python3
import datetime
import shlex
import subprocess
import os
import shutil
import argparse
from datetime import datetime


class ConsoleRunSink:
    """The ConsoleRunSink class serves as a handler for the output of the subprocess that will be started."""

    def __init__(self):
        self.to_show = []

    def add_line(self, line):
        print(line)

    def done(self):
        print("Sink done")

    @property
    def is_displayed(self):
        return True


default_sink = ConsoleRunSink()


def _run_command(command, sink=default_sink, cwd=None, shell=False, retval_raise=False):
    """Wrapper around subprocess.Popen that returns:

    : return retval, start_time, end_time, lines
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
    # we don't need it, and we don't care as C++ warnings generate too much stderr
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


class JanaConfigurator:
    def __init__(self):
        self.jana_path = shutil.which("jana4ml4fpga")
        self.root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
        self.jana_plugin_path_env = os.environ.get("JANA_PLUGIN_PATH", "")
        self.plugin_dirs = [
            os.path.join(self.root_dir, "lib")
        ]
        self.histsfile = "output.root"
        self.plugins = [
            "log",
            "root_output",
            "cdaq",
            "test_cdaq"
        ]
        self.flags = []

    def configure_dev(self):
        self.plugin_dirs = [
            os.path.join(self.root_dir, "cmake-build-debug-docker-ml4fpga/src/plugins/cdaq"),
            os.path.join(self.root_dir, "cmake-build-debug-docker-ml4fpga/src/plugins/test_cdaq"),
            os.path.join(self.root_dir, "cmake-build-debug-docker-ml4fpga/src/services/log"),
            os.path.join(self.root_dir, "cmake-build-debug-docker-ml4fpga/src/services/root_output"),
        ]

    def print(self):
        print(f"Using JANA2 executable:\n {self.jana_path}")
        print(f"root_dir: {self.root_dir}")
        print(f"jana_plugin_path_env BEFORE : {self.jana_plugin_path_env}")

    def run(self):

        jana_plugin_path_env = self.jana_plugin_path_env
        for plugin_dir in self.plugin_dirs:
            jana_plugin_path_env += plugin_dir + ":" + jana_plugin_path_env

        print(f"jana_plugin_path_env AFTER : {jana_plugin_path_env}")

        os.environ["JANA_PLUGIN_PATH"] = jana_plugin_path_env

        _run_command([
            self.jana_path,
            "-Pplugins="+",".join(self.plugins),
            "-Pjana:debug_plugin_loading=1",
            "-Pjana:timeout=0",
            "-Pnthreads=1",
        ] + self.flags)




def cdaq(args):
    print("Executing cdaq command with:")
    print(f"  - Channel: {args.index}")
    print(f"  - Filename: {args.filename}")


def convert(args):
    print("Executing convert command with:")
    print(f"  - Input: {args.input}")
    print(f"  - Output: {args.output}")

    jana = JanaConfigurator()
    jana.flags += [
        "-Pdaq:srs_window_raw:ntsamples=${SRSBIN}",
        "-Pgemrecon:LogLevel=info",
        "-Pgemrecon:ClusterF:LogLevel=info",
        "-Pgemrecon:mapping=${SRS_MAPPING}",
        "-Phistsfile=ROOT/Run_${RUNNUM}.root  $FILELIST"
    ]



    # jana_path = shutil.which("jana4ml4fpga")
    # root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    # jana_plugin_path_env = os.environ.get("JANA_PLUGIN_PATH", "")
    #
    # print(f"Using JANA2 executable:\n {jana_path}")
    # print(f"root_dir: {root_dir}")
    # print(f"jana_plugin_path_env BEFORE : {jana_plugin_path_env}")
    #
    # plugin_dirs = [
    #     os.path.join(root_dir, "cmake-build-debug/src/plugins/cdaq"),
    #     os.path.join(root_dir, "cmake-build-debug/src/plugins/test_cdaq"),
    #     os.path.join(root_dir, "cmake-build-debug/src/services/log"),
    #     os.path.join(root_dir, "cmake-build-debug/src/services/root_output"),
    # ]
    #
    # plugins = [
    #     "CDAQfile",
    #     "flat_tree",
    #     "root_output",
    #     "gemrecon"
    # ]
    #
    # for plugin_dir in plugin_dirs:
    #     jana_plugin_path_env = plugin_dir + ":" + jana_plugin_path_env
    #
    # print(f"jana_plugin_path_env AFTER : {jana_plugin_path_env}")
    #
    # os.environ["JANA_PLUGIN_PATH"] = jana_plugin_path_env
    #
    # _run_command([
    #     jana_path,
    #     "-Pplugins="+",".join(plugins),
    #     "-Pjana:debug_plugin_loading=1",
    #     "-Pcdaq:LogLevel=trace",
    #     "-Pjana:timeout=0",
    #     "-Pnthreads=1",
    #     "tcp-cdaq-evio"
    # ])




def run(args):
    print("Executing run command with:")
    print(f"  - Script: {args.script}")


def online(args):
    print("Executing online command with:")
    print(f"  - URL: {args.url}")


def main():
    parser = argparse.ArgumentParser(description="Console application with cdaq, convert, run, and online commands.")

    subparsers = parser.add_subparsers()

    cdaq_parser = subparsers.add_parser("cdaq", help="cdaq command help")
    cdaq_parser.add_argument("index", type=int, help="Channel number")
    cdaq_parser.add_argument("filename", type=str, help="Filename for output")
    cdaq_parser.set_defaults(func=cdaq)

    convert_parser = subparsers.add_parser("convert", help="convert command help")
    convert_parser.add_argument("input", type=str, help="Input file")
    convert_parser.add_argument("output", type=str, help="Output file")
    convert_parser.set_defaults(func=convert)

    run_parser = subparsers.add_parser("run", help="run command help")
    run_parser.add_argument("script", type=str, help="Script to execute")
    run_parser.set_defaults(func=run)

    online_parser = subparsers.add_parser("online", help="online command help")
    online_parser.add_argument("url", type=str, help="URL for the online service")
    online_parser.set_defaults(func=online)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
