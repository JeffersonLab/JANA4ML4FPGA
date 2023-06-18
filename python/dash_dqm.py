import uproot
from dash import dcc
import dash
# import dash_html_components as html
import plotly.graph_objects as go
from dash import html

# from dash.dependencies import Input, Output

# Open the ROOT file
file = uproot.open("D:\\data\\gemtrd\\003200_hists.root")

# Get the histogram
histogram = file["dqm/events/evt_24/gem_plane/plane_x"]

# Get the data from the histogram
data = histogram.to_numpy()
#
# # Create a Plotly figure
fig = go.Figure(data=[go.Bar(x=data[1][:-1], y=data[0])])
#
# fig.show()
# Create the Dash app
app = dash.Dash(__name__)

# Define the layout of the app
app.layout = html.Div([
    dcc.Graph(figure=fig)
])
#
# Run the app
if __name__ == '__main__':
    app.run_server(debug=True)