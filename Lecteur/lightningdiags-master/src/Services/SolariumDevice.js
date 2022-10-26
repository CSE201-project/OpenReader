import { ClipPath } from "react-native-svg";

const spaceUsedByAnalysis = 8 * 1024 * 1024;

const blobToBase64 = (blob) => {
    try {
        let reader = new FileReader();
        reader.readAsDataURL(blob);
        return new Promise((resolve) => {
            reader.onloadend = () => {
                resolve(reader.result);
            };
        });
    } catch (err) {
        console.log('error: ', err);
        return new Promise((resolve) => null);
    }
};

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
    Connected = false;
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
            return await this._fetchAsText(`/browsePatient?id=${id}`, {
                method: 'DELETE',
            });
        } catch (err) {
            console.log('Error deleting patient : ' + err);
            return null;
        }
    }

    async deleteAnalysis(id) {
        try {
            return await this._fetchAsText(`/browseAnalysis?id=${id}`, {
                method: 'DELETE',
            });
        } catch (err) {
            console.log('Error deleting patient : ' + err);
            return null;
        }
    }

    async takeMeasure(params) {
        try {
            return await this._fetchAsJson('/measure', {
                method: 'POST',
                timeout: 60 * 1000,
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

    getPdfUrl(id) {
        let url = `${this._urlBase}/files/${id}/analysis_report.pdf`;
        return url;
    }

    async getAnalysisFile(id) {
        let filename = 'analysis_results.json';
        let uri = `/files/${id}/${filename}`;

        try {
            return await this._fetchAsJson(uri);
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

    async takePictureAsBase64() {
        let rawPicture = await this.takePicture();
        return rawPicture ? await blobToBase64(rawPicture) : null;
    }

    async takeCalibrationPicture(payload) {
        try {
            let uri = '/calibration-measure';
            return await this._fetchAsJson(uri, {
                method: 'POST',
                timeout: 5000,
                body: JSON.stringify(payload),
            });
        } catch (error) {
            console.log('Error in takeCalibrationPicture: ', error);
        }
    }

    async getSpaceInfo() {
        try {
            let uri = '/info?space';
            let fullResponse = await this._fetchAsJson(uri, {
                method: 'GET',
                timeout: 5000
            });
            return fullResponse.data;
        } catch (error) {
            console.error("Error on SpaceInfo : ", error);
            return {available:0};
        }
    }

    async getNbOfAnalysisRemaining() {        
        let spaceInfo = await this.getSpaceInfo();
        return Math.ceil(spaceInfo.available / spaceUsedByAnalysis);
    }
}
