import { currentDate } from "../Tools/CurrentDate";

const _fetchWithTimeout = async (uri, options) => {
    const {timeout = 30 * 1000} = options ?? {};
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), timeout);

    console.log(`uri : ${uri}`);
    const response = await fetch(uri, {
        ...options,
        mode: 'cors',
        //cache: 'no-cache',
        signal: controller.signal,
    });

    clearTimeout(timeoutId);

    return response;
};

export default class SolariumDevice {
    IsConnected = false;
    isSeries3 = false;
    Host = '';
    Name = '';
    LEDs = {};
    TemperatureSensors = [];
    solariumVersion = '';
    defaultHTTPTimeoutMS = 5000;

    constructor(host) {
        this.setHost(host);
        this.LEDs.visible = {
            intensity: 127,
            enabled: false
        };
        this.LEDs.uv = {
            intensity: 127,
            enabled: false
        };
    }

    setHost(IP) {
        this.Host = IP.replace(/^\s+|\s+$/g, '');
        this._urlBase = 'http://' + this.Host + ':8080';
    }

    async _fetchAsJson(endpoint, options) {
        let uri = this._urlBase + endpoint;
        let response = await _fetchWithTimeout(uri, options);
        if (response.status === 200) {
            return await response.json();
        } else {
            throw `Error calling ${endpoint} : HTTP Status ${response.status}`;
        }
    }

    async _fetchAsText(endpoint, options) {
        let uri = this._urlBase + endpoint;
        let response = await _fetchWithTimeout(uri, options);
        if (response.status === 200) {
            return await response.text();
        } else {
            throw `Error calling ${endpoint} : HTTP Status ${response.status}`;
        }
    }

    async _fetch(endpoint, options) {
        let uri = this._urlBase + endpoint;
        let response = await _fetchWithTimeout(uri, options);
        if (response.status === 200) {
            return response;
        } else {
            throw `Error calling ${endpoint} : HTTP Status ${response.status}`;
        }
    }

    async connect() {
        let info = await this.info();
        if (info) {
            this.Name = info.name;
            this.IsConnected = true;
            this.solariumVersion = info.version;
            this.isSeries3 = info.name.match(/^lmx-3/) !== null;
            return true;
        } else {
            return false;
        }
    }

    async info() {
        try {
            let res = await this._fetchAsJson('/info');
            this.TemperatureSensors = res.temperature_sensors;
            return res;
        } catch (error) {
            console.log(error);
            return null;
        }
    }

    async getPatientsList(search, listStart, listEnd) {
        try {
            return this._fetchAsJson(`/browsePatient?s=${listStart}&e=${listEnd}&q=${search}`);
        } catch (error) {
            console.log(error);
            return null;
        }
    }

    async getPatientAnalysis(itemID) {
        try {
            return this._fetchAsJson(`/browseAnalysis?patientId=${itemID}`);
        } catch (error) {
            console.log('Error in browseAnalysis: ', error);
            return null;
        }
    }

    async deletePatient(id) {
        try {
            let res = await this._fetch(`/browsePatient?id=${id}`, {
                method: 'DELETE',
            });
            return await res.text();
        } catch (err) {
            console.log('Error deleting patient : ' + err);
            return null;
        }
    }

    async deleteAnalysis(id) {
        try {
            let res = await this._fetchAsText(`/browseAnalysis?q=${id}`, {
                method: 'DELETE',
            });
            return await res.text();
        } catch (err) {
            console.log('Error deleting patient : ' + err);
            return null;
        }
    }

    async getAnalysisResult(params) {
        try {
            return await this._fetchAsJson('/measure', {
                method: 'POST',
                body: JSON.stringify(params),
            });
        } catch (error) {
            console.log('Error in getAnalysisResult: ', error);
            return null;
        }
    }

    async getFile(id, pictureType) {
        let pictureName =
            pictureType === 'cropped' ? 'cropped_strip.png' : 'raw_picture.png';
        let uri = `/files/${id}/${pictureName}`;

        try {
            let res = await this._fetch(uri);
            return await res.blob();
        } catch (err) {
            console.log('Error getting file : ' + err);
            return null;
        }
    }

    async ledsEnableCommand(type, val, intensity) {
        let params = {
            type: type,
            status: val ? 'enable' : 'disable',
            intensity: intensity,
        };
        return await this._ledsCommand(params);
    }

    async ledsSetIntensityCommand(type, val) {
        let params = {
            type: type,
            status: 'enable',
            intensity: val,
        };
        return await this._ledsCommand(params);
    }

    async _ledsCommand(params) {
        let url = `${this._urlBase}/leds/${params.type}`;

        try {
            let result = await fetch(url, {
                method: 'POST',
                body: JSON.stringify(params),
            });
            if (result.status === 200) {
                this.LEDs[params.type].intensity = params.intensity;
                if (params.hasOwnProperty('status')) {
                    this.LEDs[params.type].enabled = params.status === 'enable';
                }
                return true;
            }
        } catch (error) {
            console.log(`Error in LedsCommand : ${error}`);
            return false;
        }
    }

    async takePicture(payload) {
        let uri = `${this._urlBase}/snapshot`;
        try {
            let response = await _fetchWithTimeout(uri, {
                method: 'POST',
                mode: 'cors',
                timeout: 45000,
                body: JSON.stringify(payload)
            });
            if (response.status === 200) {
                return await response.blob();
            }
        } catch (error) {
            console.log(`Error in takePicture : ${error}`);
        }
        return null;
    }

    buildCalibrationPayload(croppingParams, currentParams, cassetteName) {
        let payload = {
            croppingParam: croppingParams,
            acquisitionParam: currentParams,
            cassetteName: cassetteName
        };
        return payload;
    }

    async takeCalibrationPicture(payload) {
        try {
            let uri = this._urlBase + '/calibration-measure';
            let response = await _fetchWithTimeout(uri, {
                method: 'POST',
                mode: 'cors',
                timeout: 15000,
                body: JSON.stringify(payload)
            });
            let json = await response.json();
            return json;
        } catch (error) {
            console.log('Error in takeCalibrationPicture: ', error);
        }
    }

    async getCalibrationParams() {
        try {
            let uri = this._urlBase + '/calibration-measure';
            let response = await fetch(uri);
            let json = await response.json();
            return json;
        } catch (error) {
            console.log('Error in getCalibrationParams: ', error);
        }
    }

    async takeStudySnapshot(payload) {
        try {
            let uri = this._urlBase + '/studies';
            let response = await fetch(uri, {
                method: 'POST',
                mode: 'cors',
                timeout: this.defaultHTTPTimeoutMS,
                body: JSON.stringify(payload)
            });
            if (response.status === 200) {
                let concentrationResults = response.headers.get("x-concentration-results");                
                return [await response.blob(), concentrationResults ];
            }
        } catch (error) {
            console.log('Error in takeStudySnapshot: ', error);
        }
        return null;
    }

    async deleteStudySnapshot(study, studyDate, snapshot) {
        try {
            let uri = encodeURI(`${this._urlBase}/studies?study=${study}&date=${studyDate}&snapshot=${snapshot}`);
            let response = await fetch(uri, {
                method: 'DELETE',
                mode: 'cors',
                cache: 'no-cache'
            });
            if (response.status === 200) {
                return true;
            }
        } catch (error) {
            console.log('Error in takeStudySnapshot: ', error);
        }
        return false;
    }

    async launchSequence(payload) {
        try {
            let uri = this._urlBase + '/sequence';
            let response = await fetch(uri, {
                method: 'POST',
                mode: 'cors',
                body: JSON.stringify(payload)
            });
            if (response.status === 200) {
                
            }

        } catch (error) {
            console.log('Error in launchSequence: ', error);
        }
    }

    async launchParamFinder(name, finderParams, cassetteName) {
        const payload = {
            name: `${currentDate()}_${name}`,
            cassette_name: cassetteName,
            params : finderParams
        };
        try {
            let uri = this._urlBase + '/finder';
            let response = await fetch(uri, {
                method: 'POST',
                mode: 'cors',
                body: JSON.stringify(payload)
            });
            if (response.status === 200) {
                return '';
            } else {
                return await response.text();
            }

        } catch (error) {
            console.log('Error in launchParamFinder: ', error);
        }
    }
    
    // ###########################################

    
}
