import { useState, useContext } from 'react';
import ApiFetchPlot from "../components/ApiFetchPlot";
import BackendInfo from 'backend';
import DqmContext from "../DqmContext";


function GemRawPage() {
    const { runNumber } = useContext(DqmContext);

    let link_x =  `${BackendInfo.baseUrl}/api/v1/hist1d/${runNumber}/gem_plane/plane_URWELLX`
    let link_y =  `${BackendInfo.baseUrl}/api/v1/hist1d/${runNumber}/gem_plane/plane_URWELLY`

    return(
        <div>
            <div className="card">
                  <ApiFetchPlot link={link_x}></ApiFetchPlot>
            </div>
            <div className="card">
                <ApiFetchPlot link={link_y}></ApiFetchPlot>
            </div>
        </div>
    );

}

export default GemRawPage;