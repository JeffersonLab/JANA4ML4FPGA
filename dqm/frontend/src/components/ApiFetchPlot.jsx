import useSWR from 'swr'
import Plot from 'react-plotly.js';
import React from 'react';

function ApiFetchPlot(props) {

  const { data, error, isLoading } = useSWR(props.link);

  let notReadyReason = ""
  if(error) {
    console.log("Error loading plot")
    console.log(error);
    notReadyReason = "Error loading plot";
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