import numpy as np

# The first two events are not with data, so we start from event 3
first_good_event = 2


# Number of channels per APV
num_channels = 128


def get_num_apvs(events, num_time_bins):
    """Returns number of APVs based on data in events tree"""

    # Number of samples read from one event is:
    # samples = num_time_bins * num_channels * num_apv
    # The number of APVs is calculated
    samples = events['srs_raw_samples'].array(
        library="np",
        entry_start=first_good_event,
        entry_stop=first_good_event+1)

    # Get number of APVs
    num_apvs = int(len(samples[0])/num_time_bins/num_channels)
    return num_apvs


def get_num_time_bins(events):
    """Gets the number of time bins based on data in events tree"""

    # GET NUMBER OF TIME BINS
    # Now we need to extract number of time-bins for each channel.
    # The number is actually the same for the whole run, so we need to get only one element
    # So we're taking 1st element from 1st event we read (1st and the only event) => [0][0]
    num_time_bins = events['srs_raw_samples_count'].array(
        library="np",
        entry_start=first_good_event,
        entry_stop=first_good_event+1)[0][0]
    return num_time_bins


def get_channel_order(events, num_apvs):
    """Gets the channel order of each APVs based on data in events tree"""

    # GET CHANNEL MAPPING
    # srs_raw_channel_apv contains correct order of channels for each apv. I.e. MAPPING
    # Since the mapping is the same for each apv, we need just one mapping
    # So we take just a single event, first APV and we get a map for 128 channels
    ch_mapping = events['srs_raw_channel_apv'].array(
        entry_start=first_good_event,
        entry_stop=first_good_event + 1)

    # Convert it to (events:apv:channel) shape:
    ch_mapping = np.reshape(ch_mapping, (len(ch_mapping), num_apvs, num_channels))

    # Now only take the last dimension and convert it to numpy
    ch_mapping = np.array(ch_mapping[0][0])
    return ch_mapping