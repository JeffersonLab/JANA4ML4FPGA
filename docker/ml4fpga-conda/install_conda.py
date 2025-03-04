import argparse
import subprocess
import shlex
import os
import sys
from collections import OrderedDict
from os import path
from datetime import datetime
import re
from tempfile import mkstemp
from shutil import move
from os import remove


mamba_ENV_NAME = 'ml4fpga'
ENV_NAME_TOP_DIR = 'ML4FPGA_TOP_DIR'

SCRIPT_NAME_SETUP_mamba = "setup_mamba.sh"
SCRIPT_NAME_BUILD_SOFT = "build_software.sh"
SCRIPT_NAME_ENV_BASH = "setup_env.sh"
SCRIPT_NAME_ENV_CSH = "setup_env.csh"
SCRIPT_NAME_mamba_ENV = "environment.yaml"
INSTALL_SCRIPTS_DIR_NAME = "install_scripts"

# --------------------------------------------------------
#   mamba ENVIRONMENT
#   This file defines what packages mamba installs
# --------------------------------------------------------

mamba_env_content = """
name: ${mamba_env_name}
channels:
  - mamba-forge
  - defaults
dependencies:
  - python=3.10
  - gcc
  - cmake
  - xerces-c
  - xorg-libxmu
  - clhep
  - git
  - git-lfs
  - nodejs
  - boost
  - ipywidgets
  - zstd
  - hepmc3
  - pip
  - pip:
    - click
    - appdirs
    - edpm
    - uproot
    - awkward-numba
    - numpy
    - pandas
    - matplotlib
    - seaborn
    - plotly
    - pyjet
    - pyjano
    - wget
    - edpm
variables:
  PYTHONHTTPSVERIFY: "0"
  EDPM_DATA_PATH: ${EDPM_DATA_PATH}
"""



class InstallInfo:
    this_script_dir: str
    top_dir: str
    mamba_dir: str
    mamba_env_name: str
    mamba_env_dir: str
    scripts_dir: str
    script_setup_mamba: str
    script_build_soft: str
    script_openssl_cnf: str
    script_mamba_env: str
    env_name_top_dir = ENV_NAME_TOP_DIR


    @staticmethod
    def create_from_env() -> 'InstallInfo':
        """This method setups all main variables"""
        # The directory in which this script is located
        this_script_dir = path.dirname(path.abspath(__file__))

        # In case 'JANA4ML4FPGA_TOP_DIR' was not in environ
        top_dir = os.environ.get(ENV_NAME_TOP_DIR, this_script_dir)
        os.environ[ENV_NAME_TOP_DIR] = top_dir

        # mamba
        mamba_dir = path.join(top_dir, 'micromamba')
        mamba_env_name = mamba_ENV_NAME
        mamba_env_dir = path.join(mamba_dir, 'envs', mamba_env_name)        # Directory of the mamba environment

        # Create result
        result = InstallInfo()
        result.this_script_dir=this_script_dir
        result.top_dir=top_dir
        result.mamba_dir=mamba_dir
        result.mamba_env_name=mamba_env_name
        result.mamba_env_dir=mamba_env_dir
        result.scripts_dir=path.join(top_dir, INSTALL_SCRIPTS_DIR_NAME)
        result.script_setup_mamba=path.join(top_dir, INSTALL_SCRIPTS_DIR_NAME, SCRIPT_NAME_SETUP_mamba)
        result.script_build_soft=path.join(top_dir, INSTALL_SCRIPTS_DIR_NAME, SCRIPT_NAME_BUILD_SOFT)
        result.script_mamba_env=path.join(top_dir, INSTALL_SCRIPTS_DIR_NAME, SCRIPT_NAME_mamba_ENV)
        result.script_openssl_cnf=path.join(top_dir, INSTALL_SCRIPTS_DIR_NAME, 'openssl.cnf')


        result.print_self()

        return result

    def asdict(self):
        return {key:value for key, value in self.__dict__.items() if not key.startswith('__') and not callable(key)}

    def print_self(self):
        # Print some path
        print("Created install info:")
        print("  -this_script_dir:", self.this_script_dir)
        print("  -top_dir:", self.top_dir)
        print("  -mamba_dir:  ", self.mamba_dir)
        print("  -mamba_env_name: ", self.mamba_env_name)
        print("  -mamba_env_dir: ", self.mamba_env_dir)
        print("  -scripts_dir: ", self.scripts_dir)
        print("  -script_mamba_env: ", self.script_mamba_env)
        print("  -script_setup_mamba: ", self.script_setup_mamba)
        print("  -script_build_soft: ", self.script_build_soft)
        print("  -script_openssl_cnf: ", self.script_openssl_cnf)



# Create install info from environment and current directories
install_info = InstallInfo.create_from_env()
assert isinstance(install_info, InstallInfo)        # Mainly for AI and IDEs autocompletion


# Set strict channel priority
# look here
# https://mamba-forge.org/docs/user/tipsandtricks.html
# https://mamba-forge.org/docs/minutes/2020-01-22.html
# https://docs.mamba.io/projects/mamba/en/latest/user-guide/tasks/manage-channels.html
# test
# mamba config --describe channel_priority
#
mambarc_content = """
channel_priority: strict
channels:
  - mamba-forge
  - defaults
"""

openssl_cnf_content = """
openssl_conf = openssl_init

[openssl_init]
ssl_conf = ssl_sect

[ssl_sect]
system_default = system_default_sect

[system_default_sect]
Options = UnsafeLegacyRenegotiation
"""

print(install_info.asdict())

# noinspection PyArgumentList
template_user_sh = """
export ML4FPGA_TOP_DIR={top_dir}

# Start mamba environment
source {top_dir}/micromamba/etc/profile.d/mamba.sh
mamba activate {mamba_env_name}

# The path where edpm stores its JSon database and creates env files
export EDPM_DATA_PATH={top_dir}

# This tells EDPM not to generate source thisroot.sh
export ROOT_INSTALLED_BY_mamba=1

# This is unfortunate requirement for JLab certificates
export PYTHONHTTPSVERIFY=0
export OPENSSL_CONF={script_openssl_cnf}

# source environment generated by EDPM
# means ROOT and others
source $EDPM_DATA_PATH/env.sh
""".format(**install_info.asdict())

# noinspection PyArgumentList
template_user_csh = """
setenv ML4FPGA_TOP_DIR {top_dir}

# Start mamba environment
source {top_dir}/micromamba/etc/profile.d/mamba.csh
mamba activate {mamba_env_name}

# The path where edpm stores its JSon database and creates env files
setenv EDPM_DATA_PATH {top_dir}

# This tells EDPM not to generate source thisroot.sh
setenv ROOT_INSTALLED_BY_mamba 1

# This is unfortunate requirement for JLab certificates
setenv PYTHONHTTPSVERIFY 0
setenv OPENSSL_CONF {script_openssl_cnf}

# source environment generated by EDPM
# means ROOT and others
source $EDPM_DATA_PATH/env.csh
""".format(**install_info.asdict())


# noinspection PyArgumentList
template_setup_mamba = """
set -e
source {mamba_dir}/etc/profile.d/mamba.sh
    
export PYTHONHTTPSVERIFY=0
export OPENSSL_CONF={script_openssl_cnf}
mamba config --set ssl_verify false
mamba update -n base -y mamba
mamba create -y --name {mamba_env_name} python=3.10
mamba activate {mamba_env_name}

edpm --top-dir={top_dir}

edpm config global cxx_standard=17

# so edpm generated the right environment
export ROOT_INSTALLED_BY_mamba=1

# Set root which we installed before
edpm set root {mamba_env_dir}
edpm set clhep  {mamba_env_dir}
edpm set hepmc3 {mamba_env_dir}

""".format(**install_info.asdict())


# noinspection PyArgumentList
template_build_soft = """
set -e
export EDPM_DATA_PATH={top_dir}
source {mamba_dir}/etc/profile.d/mamba.sh
mamba activate {mamba_env_name}
echo ""
echo "================================"
echo "  B U I L D    P A C K A G E S  "
echo "================================"
echo ""
edpm config jana2 branch=v2.1.0
edpm config jana2 cmake_flags="-DUSE_ROOT=On -DUSE_PYTHON=Off -DUSE_PODIO=Off"
edpm install jana2
edpm install jana4ml4fpga
""".format(**install_info.asdict())


def run(command, cwd=None, shell=False, exit_on_error=True, silent=False):
    """Wrapper around subprocess.Popen that returns:

    :return retval, start_time, end_time, lines
    """
    if isinstance(command, str):
        command = shlex.split(command)

    # Pretty header for the command
    if not silent:
        print('=' * 20)
        print("RUN: " + " ".join(command))
        print('=' * 20)

    # Record the start time
    start_time = datetime.now()
    lines = []

    # stderr is redirected to STDOUT because otherwise it needs special handling
    # we don't need it and we don't care as C++ warnings generate too much stderr
    # which makes it pretty much like stdout
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd, shell=shell)
    while True:
        line = process.stdout.readline().decode('latin-1').replace('\r', '\n')

        if process.poll() is not None and line == '':
            break
        if line:
            if line.endswith('\n'):
                line = line[:-1]
            if not silent:
                try:
                    # This try block is to fix
                    # TypeError: endswith first arg must be bytes or a tuple of bytes, not str
                    # which may happen when building root under docker
                    print(line)
                except:
                    print(str(line.encode('utf-8')))
                lines.append(line)

    # Get return value and finishing time
    retval = process.poll()
    end_time = datetime.now()
    if not silent:
        print("------------------------------------------")
        print("RUN DONE. RETVAL: {} \n\n".format(retval))

    if retval != 0:
        if not silent:
            print("ERROR. Retval is not 0. Plese, look at the logs\n")
        if exit_on_error:
            exit(1)

    return retval, start_time, end_time, lines


def make_file(file_path, content):
    with open(file_path, 'w') as f:
        f.write(content)


def is_mamba_env_exist():
    global install_info

    mamba_env_exe = path.join(install_info.mamba_dir, 'bin', 'mamba-env')
    _, _, _, lines = run('{mamba_env_exe} list'.format(mamba_env_exe=mamba_env_exe), silent=True)

    # mamba env list give something like:
    # # mamba environments:
    #  base   <path>
    #  esc  * <path>
    pattern = re.compile("^{env_name}\\s".format(env_name=install_info.mamba_env_name))
    for line in lines:
        if pattern.match(line):
            return line


def step0_generate_scripts():
    """ Generate all bash scripts """
    global install_info

    print("Creating scripts directory")
    os.makedirs(install_info.scripts_dir, exist_ok=True)

    print("Generating scripts")
    make_file(path.join(install_info.top_dir, SCRIPT_NAME_ENV_BASH), template_user_sh)
    make_file(path.join(install_info.top_dir, SCRIPT_NAME_ENV_CSH), template_user_csh)
    make_file(install_info.script_openssl_cnf, openssl_cnf_content)
    make_file(install_info.script_setup_mamba, template_setup_mamba)
    make_file(install_info.script_build_soft, template_build_soft)


def step1_install_micromamba():
    """ Install micromamba """
    # install micromamba
    global install_info
    if os.path.isdir(install_info.mamba_dir):
        print("Path already exists. Skipping installation step.")
        return

    import platform
    if 'Darwin' in platform.system():
        if platform.machine() == 'arm64':  # For Apple Silicon (M1/M2)
            mamba_install_sh_link = 'https://micro.mamba.pm/api/micromamba/osx-arm64/latest'
        else:  # For Intel Macs
            mamba_install_sh_link = 'https://micro.mamba.pm/api/micromamba/osx-64/latest'
    elif platform.system() == 'Linux':
        if platform.machine() == 'aarch64':  # For ARM64 Linux
            mamba_install_sh_link = 'https://micro.mamba.pm/api/micromamba/linux-aarch64/latest'
        else:  # For x86_64 Linux
            mamba_install_sh_link = 'https://micro.mamba.pm/api/micromamba/linux-64/latest'
    elif platform.system() == 'Windows':
        mamba_install_sh_link = 'https://micro.mamba.pm/api/micromamba/win-64/latest'
    else:
        raise SystemError(f"Unsupported system: {platform.system()}")

    print("Downloading micromamba from {} ...".format(mamba_install_sh_link))
    run('curl {mamba_install_script} -o micromamba.sh'.format(mamba_install_script=mamba_install_sh_link))
    run("bash micromamba.sh -b -p " + install_info.mamba_dir)
    run("rm micromamba.sh")

    # global mamba config
    make_file(path.join(install_info.mamba_dir, '.mambarc'), mambarc_content)


def step2_setup_mamba():
    """ Setups mamba, install packages"""
    global install_info

    if is_mamba_env_exist():
        print(f"Environment {install_info.mamba_env_name} exists. Skipping enironment creation ")
        return

    # create epic environment with root
    return run('bash ' + install_info.script_setup_mamba, shell=False, silent=False)


def step3_build_software():
    # create epic environment with root
    return run('bash ' + install_info.script_build_soft, shell=False, silent=False)


if __name__ == "__main__":
    steps = OrderedDict()

    steps['gen_scripts'] = step0_generate_scripts
    steps['install_mamba'] = step1_install_micromamba
    steps['setup_mamba'] = step2_setup_mamba
    steps['build_soft'] = step3_build_software

    # This is to print argparse help
    steps_help = "Install steps (in default order):\n" + "\n".join(["   "+s for s in steps.keys()])

    parser = argparse.ArgumentParser(epilog=steps_help, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('-s', "--step",
                        help="Name of installation step. 'all' (default) - full installation",
                        default='all')

    parser.add_argument("--build-root", help="Build root from sources instead of installing from mamba",
                        action="store_true", default=False)
    args = parser.parse_args()

    if args.step == 'all':
        for step_func in steps.values():
            step_func()
    elif args.step in steps.keys():
        steps[args.step]()
    else:
        parser.print_help()

