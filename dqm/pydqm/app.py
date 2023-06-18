import inspect

import plotly
from flask import Flask, g
from flask.json.provider import DefaultJSONProvider

import os
import json
import typing as t

from .histo_api import histo_api
from flask_cors import CORS
from flask import current_app
import uproot

# This address beginning will be added to API bueprints
API_PREFIX = '/api/v1'


class PlotlyCompatibleJsonProvider(DefaultJSONProvider):
    def dumps(self, obj: t.Any, **kwargs: t.Any) -> str:
        """Serialize data as JSON to a string.

        Keyword arguments are passed to :func:`json.dumps`. Sets some
        parameter defaults from the :attr:`default`,
        :attr:`ensure_ascii`, and :attr:`sort_keys` attributes.

        :param obj: The data to serialize.
        :param kwargs: Passed to :func:`json.dumps`.
        """
        cls = plotly.utils.PlotlyJSONEncoder

        if "default" not in cls.__dict__:
            kwargs.setdefault("default", self.default)

        ensure_ascii = self._app.config["JSON_AS_ASCII"]
        sort_keys = self._app.config["JSON_SORT_KEYS"]

        if ensure_ascii is not None:
            import warnings

            warnings.warn(
                "The 'JSON_AS_ASCII' config key is deprecated and will"
                " be removed in Flask 2.3. Set 'app.json.ensure_ascii'"
                " instead.",
                DeprecationWarning,
            )
        else:
            ensure_ascii = self.ensure_ascii

        if sort_keys is not None:
            import warnings

            warnings.warn(
                "The 'JSON_SORT_KEYS' config key is deprecated and will"
                " be removed in Flask 2.3. Set 'app.json.sort_keys'"
                " instead.",
                DeprecationWarning,
            )
        else:
            sort_keys = self.sort_keys

        kwargs.setdefault("ensure_ascii", ensure_ascii)
        kwargs.setdefault("sort_keys", sort_keys)
        kwargs.setdefault("cls", cls)
        return json.dumps(obj, **kwargs)


app = Flask(__name__)
CORS(app)
app.json = PlotlyCompatibleJsonProvider(app)


app.register_blueprint(histo_api, url_prefix=API_PREFIX)


# api = Api(app)
# api.add_resource(VolatilityPage, '/volatility')

# Directory path
# add trigger db library if needed
this_folder = os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0]))
# python_folder = os.path.join(this_folder, "python")
# if python_folder not in sys.path:
#     sys.path.insert(0, python_folder)

# Debug print
config = app.config
config["BACKEND_ROOT_PATH"] = this_folder

@app.before_request
def before_request():
    g.root_file = uproot.open(current_app.config["ROOT_FILE_PATH"])


@app.route('/')
def index():
    print(current_app.config)  # Prints the configuration dictionary for your application

    return "REST API backend. This is a placeholder for index. You probably need some other link " \
        + repr(current_app.config["ROOT_FILE_PATH"])


if __name__ == "__main__":
    app.run(debug=True)


