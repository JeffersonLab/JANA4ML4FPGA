import useSWR from 'swr'
import Plot from 'react-plotly.js';
import React from 'react';

const fetcher = async url => {
    const res = await fetch(url);

    // If the status code is not in the range 200-299,
    // we still try to parse and throw it.
    if (!res.ok) {
        const error = new Error('An error occurred while fetching the data.')
        if (res.status === 500) {
            // Attach extra info to the error object.
            error.info = await res.json()
        }
        error.status = res.status
        throw error
    }

    return res.json()
}

function ApiFetchPlot(props) {

    const { data, error, isLoading } = useSWR(props.link, fetcher);

    let notReadyReason = ""
    if(error) {
        console.log("Error loading plot")
        console.log(error);

        // Default error message
        let backendErrorDescr = error?.response?.data?.error || "Unspecified error on backend"

        // Backend provides error messages if error code is 500
        if(error.status === 500 && error.info && error.info.message) {
            notReadyReason = error.info.message;
        }

        notReadyReason = `Error loading plot: '${backendErrorDescr}'`;
    }

    if(isLoading) {
        notReadyReason = "Loading data";
    }

    if (data) {
        return (<Plot data={data.data} layout={data.layout} config={data.config}/>);
    }
    return (<div>{notReadyReason}</div>);
}

export default ApiFetchPlot;