from ROOT import *
fileName = "/home/romanov/eic/JANA4ML4FPGA/cmake-build-debug/output.root"
file = TFile(fileName)
tree = file.Get("events")
for event in tree:
    print(event.srs_best_sample)
    print(event.srs_best_sample)
