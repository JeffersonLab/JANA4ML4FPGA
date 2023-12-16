#%%

import uproot
import matplotlib.pyplot as plt
import numpy as np

#%%
# Set rangdom seed (while we are not using it, probably)
np.random.seed(1)

# The first two events are not with data
first_good_event=2

num_channels = 128

# Open file
events = uproot.open("003200_v9_hists.root")["events"]

# Uncomment to see file contents
# events.keys()