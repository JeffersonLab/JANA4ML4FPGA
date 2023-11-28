from flask import Blueprint, jsonify, g
from flask import request
import plotly.express as px
import plotly.graph_objects as go
import uproot
from uproot import KeyInFileError
from uproot.models.TH import Model_TH1F_v3
from werkzeug.utils import secure_filename

histo_api = Blueprint('histo_api', __name__)


@histo_api.route('/hist1d/<run>/<path:path>')
def hist1d(run, path):

    # Get the histogram
    run = int(run)

    try:

        hist_path = f"dqm/events/evt_{run}/{path}"
        print(f"serving as hist1d: {hist_path}")
        histogram = g.root_file[hist_path]
    except Exception as e:
        response = jsonify({"message": str(e)})
        response.status_code = 500
        print(f"Getting response error: {str(e)}")
        return response
        # Get the data from the histogram
    data = histogram.to_numpy()

    # # Create a Plotly figure
    fig = go.Figure(data=[go.Bar(x=data[1][:-1], y=data[0])])
    fig.update_layout(bargap=0)

    # fig.add_trace(go.Scatter(x=vol_df['Date'], y=vol_df[data_name], mode='lines', name=data_name))
    #
    response = jsonify(fig)
    return response


@histo_api.route('/geminfo/')
def geminfo(run, path):

    # Get the histogram
    histogram = g.root_file["dqm/events/evt_24/gem_plane/plane_x"]
    isinstance(histogram, Model_TH1F_v3)
    # Get the data from the histogram
    data = histogram.to_numpy()

    # fig.add_trace(go.Scatter(x=vol_df['Date'], y=vol_df[data_name], mode='lines', name=data_name))
    #
    response = jsonify(histogram.to_numpy())
    return response