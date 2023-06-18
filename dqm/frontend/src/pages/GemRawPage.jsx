import { useState } from 'react';
import ApiFetchPlot from "../components/ApiFetchPlot";
import BackendInfo from 'backend';

function GemRawPage() {
    const [vol7, setVol7] = useState(1 );
    const [vol30, setVol30] = useState(0);
    const [vol60, setVol60] = useState(0);

    const [park7, setPark7] = useState(1 );
    const [park30, setPark30] = useState(0);
    const [park60, setPark60] = useState(0);

    const prefixes_vol = [];
    const prefixes_park = [];

    

    let link =  `${BackendInfo.baseUrl}/volatility`
    let link_park =  `${BackendInfo.baseUrl}/volatility`;

    if(vol7) {
        prefixes_vol.push('v7');
    }
    if(vol30) {
        prefixes_vol.push('v30');
    }
    if(vol60) {
        prefixes_vol.push('v60');
    }
    if(prefixes_vol.length > 0) {
        link += '/' + prefixes_vol.join('-');
    }

    if(park7) {
        prefixes_park.push('p7');
        prefixes_park.push('v7');
    }
    if(park30) {
        prefixes_park.push('p30');
        prefixes_park.push('v30');
    }
    if(park60) {
        prefixes_park.push('p60');
        prefixes_park.push('v60');
    }
    if(prefixes_park.length > 0) {
        link_park += '/' + prefixes_park.join('-');
    }

    return(
        <>
        <div className="card">
            <div className= "checkbox_vol" style={{marginTop: 40 + 'px', marginLeft: 80 + 'px'}}>
                <input type="checkbox" checked={vol7} onChange={ (event)=>{
                                            console.log(event);
                                            setVol7(event.target.checked);
                                            }}/> Vol7
                <input type="checkbox" style={{ marginLeft: 20 + 'px'}} checked={vol30} onChange={ (event)=>{
                                            console.log(event);
                                            setVol30(event.target.checked);
                                            }}/> Vol30
                <input type="checkbox" style={{ marginLeft: 20 + 'px'}} checked={vol60} onChange={ (event)=>{
                                            console.log(event);
                                            setVol60(event.target.checked);
                                            }}/> Vol60
            </div>
              <ApiFetchPlot link={link}></ApiFetchPlot>
        </div>
        <div className="card">
            <div className= "checkbox_vol" style={{marginTop: 40 + 'px', marginLeft: 80 + 'px'}}>
                <input type="checkbox" checked={park7} onChange={ (event)=>{
                                            console.log(event);
                                            setPark7(event.target.checked);
                                            }}/> Park7
                <input type="checkbox" style={{ marginLeft: 20 + 'px'}} checked={park30} onChange={ (event)=>{
                                            console.log(event);
                                            setPark30(event.target.checked);
                                            }}/> Park30
                <input type="checkbox" style={{ marginLeft: 20 + 'px'}} checked={park60} onChange={ (event)=>{
                                            console.log(event);
                                            setPark60(event.target.checked);
                                            }}/> Park60
            </div>
              <ApiFetchPlot link={link_park}></ApiFetchPlot>
        </div>
      </>
    );

}

export default GemRawPage;