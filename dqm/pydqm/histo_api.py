from flask import Blueprint, jsonify, g
from flask import request
import plotly.express as px
import plotly.graph_objects as go


histo_api = Blueprint('histo_api', __name__)


@histo_api.route('/volatility/<crypto_name>/<vol>')
@histo_api.route('/volatility/<plot_request_str>')
@histo_api.route('/volatility')
def volatility(plot_request_str=' ', crypto_name='BTC-USD'):
    '''
    1. each plot divided by -
    2. each plot starts with:
       - 's' - standard deviation
       - 'v' - volatility (common)
       - 'p' - parkinson volatility
    3. Then goes number of days. Now discreet [7, 30, 60, 90, 180]

    v7-v30-p7-p30

    '''

    'vol_7_day'
    
    if not plot_request_str:
        message = 'plot_request_str is empty or None'
        print(message)
        return jsonify({'message':message}), 400
    
    plot_requests = plot_request_str.split('-')

    # get data
    ochl_df = g.dp.get_daily_vochl_values('BTC-USD')

    # calculate volatility
    vol_df = calculate_historical(ochl_df)

    # creating picture
    fig = go.Figure()

    request_to_data_prefix_map = {
        's': 'sd', 
        'v': 'vol', 
        'p': 'park_vol'
    }

    for plot_request in plot_requests:
        
        if plot_request[0] not in request_to_data_prefix_map.keys():
            message = f"Plot request format is bad. It should start with {request_to_data_prefix_map.keys()}"
            print(message)
            return jsonify({'error': message}), 400
        
        prefix = request_to_data_prefix_map[plot_request[0]]

        try:
            days = int(plot_request[1:])
        except Exception as err:
            message = f"Plot request format is bad. Error parsing number of days {err}"
            print(message)
            return jsonify({'error': message}), 400
        
        data_name = f"{prefix}_{days}_day"
        
        fig.add_trace(go.Scatter(x=vol_df['Date'], y=vol_df[data_name], mode='lines', name=data_name))

    response = jsonify(fig.to_dict())
    return response

