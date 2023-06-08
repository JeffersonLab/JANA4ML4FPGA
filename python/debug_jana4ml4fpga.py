import datetime
import shlex
from datetime import datetime
import subprocess
import os
import shutil
from pprint import pprint


def _print_path_env(env_str: str, title: str):
    tokens = env_str.split(":")
    lines = [f"   {token}" for token in tokens if token]
    print(title)
    pprint(lines)


def run(command, cwd=None, shell=False, retval_raise=False):
    """Wrapper around subprocess.Popen that returns:

    :return retval, start_time, end_time, lines
    """
    if isinstance(command, str):
        command = shlex.split(command)

    # Pretty header for the command
    print('=' * 20)
    print("CWD: " + cwd if cwd else os.getcwd())
    print("RUN: " + " ".join(command))
    print('=' * 20)

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

                print(line)
                lines.append(line)

        # Get return value and finishing time
        retval = process.poll()

    end_time = datetime.now()

    print("------------------------------------------")
    print(f"RUN DONE. RETVAL: {retval} \n\n")
    if retval != 0:
        print(f"ERROR. Retval is not 0. Plese, look at the logs\n")

        if retval_raise:
            raise RuntimeError("ERROR. Retval is not 0. Plese, look at the logs")
    return retval, start_time, end_time, lines


def main():

    # Identify repository root
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    print(f"Repo root_dir: {root_dir}")

    # Identify executable to run
    executable = os.path.join(root_dir, "install/bin/jana4ml4fpga")
    print(f"Using executable:\n {executable}")
    print(f"File exists: {os.path.exists(executable)}")

    # Add compiled plugins to JANA_PLUGIN_PATH
    jana_plugin_path_env = os.environ.get("JANA_PLUGIN_PATH", "")
    _print_path_env(jana_plugin_path_env, "Initial JANA_PLUGIN_PATH")

    plugin_dirs = [
        os.path.join(root_dir, "cmake-build-debug/src/plugins/cdaq"),
        os.path.join(root_dir, "cmake-build-debug/src/plugins/test_cdaq"),
        os.path.join(root_dir, "cmake-build-debug/src/services/log"),
        os.path.join(root_dir, "cmake-build-debug/src/services/root_output"),
    ]
    for plugin_dir in plugin_dirs:
        jana_plugin_path_env = plugin_dir + ":" + jana_plugin_path_env
    os.environ["JANA_PLUGIN_PATH"] = jana_plugin_path_env

    _print_path_env(jana_plugin_path_env, "Final JANA_PLUGIN_PATH")

    # Add path for libJANA to where it belongs
    ld_lib_path_env = os.environ.get("LD_LIBRARY_PATH", "")
    jana_home_env = os.environ.get("JANA_HOME", "")
    print(f"JANA_HOME = {jana_home_env}")
    ld_lib_path_env = f"{jana_home_env}/lib:{ld_lib_path_env}"
    os.environ["LD_LIBRARY_PATH"] = ld_lib_path_env


    # Add plugins
    plugins = [
        "log",
        "root_output",
        #"cdaq",
        "example_evio_analysis"
    ]


    command = [
        executable,
        "-Pplugins="+",".join(plugins),
        "-Pjana:debug_plugin_loading=1",
        "-Pcdaq:LogLevel=trace",
        "-Pjana:timeout=0",
        "-Pnthreads=1",
        "/mnt/work/data/2023-03-03-trd-data/hd_rawdata_002543_000.evio"
        #"/mnt/work/data/2023-03-03-trd-data/hd_rawdata_002548_000.evio"
        #"/mnt/work/data/2023-03-03-trd-data/hd_rawdata_002531_000.evio"
    ]

    print(command)
    # Lets fly
    run(command, shell=False)


if __name__ == "__main__":
    main()
