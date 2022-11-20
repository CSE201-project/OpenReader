const fetch = require('node-fetch');
const AbortController = require('abort-controller');

module.exports = class SolariumDevice {
    IsConnected = false;
    Host = '';
    Name = '';
    LEDs = {};
    TemperatureSensors = [];
    solariumVersion = '';

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

    async _fetchWithTimeout(uri, options) {
        const { timeout = 8000 } = options;
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), timeout);
      
        const response = await fetch(uri, {
          ...options,
          signal: controller.signal  
        });

        clearTimeout(timeoutId);
      
        return response;
      }

    async info() {
        let uri = `${this._urlBase}/info`;

        try {
            let response = await this._fetchWithTimeout(uri,  {
                method: 'GET',
                mode: 'cors',
                headers: { 'Content-Type': 'application/json' },
                timeout: 3000
            });
            if (response.status === 200) {
                let infoResult = await response.json();
                return (infoResult);
            }
        } catch (error) {
            console.debug(`Error in getInfo : ${error}`);
        }
        return null;
    }

    async connect() {
        let info = await this.info();
        if (info) {
            this.Name = info.name;
            this.IsConnected = true;
            this.solariumVersion = info.version;
            return true;
        } else {
            return false;
        }
    }

    async takePicture(payload) {
        let uri = `${this._urlBase}/snapshot`;

        try {
            let response = await fetch(uri, {
                method: 'POST',
                body: JSON.stringify(payload),
            });
            if (response.status === 200) {
                return await response.blob();
            }
        } catch (error) {
            console.log(`Error in takePicture : ${error}`);
        }
        return null;
    }

    async ledsEnableCommand(type, val) {
        let params = {
            type: type,
            status: val ? 'enable' : 'disable',
            intensity: this.LEDs[type].intensity,
        };
        return await this._ledsCommand(params);
    }

    async ledsSetIntensityCommand(type, val) {
        let params = {
            type: type,
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

    async getPatientFile(itemID) {
        try {
            let uri = this._urlBase + `/browse/${itemID}`;
            console.debug('calling', uri);
            let response = await fetch(uri);
            let json = await response.json();
            return json;
        } catch (error) {
            console.log(error);
            return null;
        }
    }

    async getPatientList(search, listStart, listEnd) {
        try {
            let uri = !search[0] ? 
                `${this._urlBase}/browse?s=${listStart}&e=${listEnd}` 
                : `${this._urlBase}/browse?s=${listStart}&e=${listEnd}&q=${search}`;
            console.debug('calling ', uri);
            let response = await fetch(uri);
            let json = await response.json();
            return json;
        } catch (error) {
            console.log(error);
            return null;
        }
    }
    
    async getAnalysisResult(params) {
        try {
            let uri = this._urlBase + '/measure';
            let response = await fetch(uri, {
                method: 'POST',
                body: JSON.stringify(params),
            });
            let json = await response.json();
            return json;
        } catch (error) {
            console.log('Error in getPatientResults: ', error);
        }
    }
}
