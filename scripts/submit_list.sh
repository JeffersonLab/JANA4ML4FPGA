#!/bin/bash

exit 1

ssh gluon113 "cd JANA3; ./run_list.sh LIST/file_fermi_3bin.aa 3 "

# ssh gluon114 "cd JANA3; ./run_list.sh LIST/file_fermi_3bin.ab 3 "
ssh gluonraid4 "cd JANA3; ./run_list.sh LIST/file_fermi_3bin.ab 3 "

ssh gluon115 "cd JANA3; ./run_list.sh LIST/file_fermi_3bin.ac 3 "

ssh gluon116 "cd JANA3; ./run_list.sh LIST/file_fermi_6bin.aa 6 "

# ssh gluon117 "cd JANA3; ./run_list.sh LIST/file_fermi_6bin.ab 6 "
ssh gluonraid5 "cd JANA3; ./run_list.sh LIST/file_fermi_6bin.ab 6 "

# ssh gluon200 "cd JANA3; ./run_list.sh LIST/file_fermi_6bin.ac 6 "
ssh gluonraid3 "cd JANA3; ./run_list.sh LIST/file_fermi_6bin.ac 6 "

xterm -e ssh gluon113 &
#xterm -e ssh gluon114 &
xterm -e ssh gluonraid4 &
xterm -e ssh gluon115 &
xterm -e ssh gluon116 &
#xterm -e ssh gluon117 &
xterm -e ssh gluonraid5 &
#xterm -e ssh gluon200 &
xterm -e ssh gluonraid3 &
