const DEFAULT_CAMERA_PARAMS = require('../Assets/default-camera-params.json');
const DEFAULT_CALIBRATION_PARAMS = require('../Assets/default-calibration.json');
const CROPPING_PARAMS = require('../Assets/cropping-params.json');

export default class RemoteStudy {
    constructor(studyName, concentrationListString, concentrationUnit, deviceName) {
        this.name = studyName;
        this.date = new Date().toISOString().split('T')[0].replaceAll('-','');
        this.unit = concentrationUnit;
        this.concentrationList = this._createConcentrationList(concentrationListString, concentrationUnit);
        this.acquisitionParams = [];
        this.deviceName = deviceName;
        this.cassetteName = "K701BT_SE";
        this.addAcquisitionWithDefaults();        
    }

    _createConcentrationList(concentrationListString, concentrationUnit) {
        let concentrationList = new Map();
        let inputString = concentrationListString.trim();
        if (inputString.length) {
            let list = inputString.split('\n');
            for (let i = 0; i < list.length; i++) {
                let concentrationName = list[i];// + ' ' + concentrationUnit;
                concentrationList.set(concentrationName, []);
            }
        }
        return concentrationList;
    }

    setConcentrationList(concentrationListString) {
        var newList = this._createConcentrationList(concentrationListString, this.unit);
        if (this.concentrationList && this.concentrationList.size) {

            let newListKeys = [...newList.keys()];
            let oldListKeys = [...this.concentrationList.keys()];

            for (let i = 0; i < oldListKeys.length; i++) {
                newList.set(newListKeys[i], this.concentrationList.get(oldListKeys[i]));
            }
        }
        this.concentrationList = newList;
    }

    addSnapshot(concentrationName) {
        let snapshot = {};

        snapshot.concentration = concentrationName;
        snapshot.photoNumber = this.getSnapshotNumber(concentrationName) + 1;
        snapshot.fileName = `${this.name}_${concentrationName}${this.unit}_N${String(snapshot.photoNumber).padStart(2, '0')}`;

        this.concentrationList.get(concentrationName).push(snapshot);
        return snapshot;
    }

    deleteSnapshot(snapshot) {
        let snapshotList = this.concentrationList.get(snapshot.concentration);
        let index = snapshotList.indexOf(snapshot);
        snapshotList.splice(index, 1);
    }

    getSnapshotNumber(concentrationName) {
        return this.concentrationList.get(concentrationName)?.length || 0;
    }

    getSnapshotPayload(snapshot) {
        let croppingParams =  CROPPING_PARAMS[this.deviceName];
        let croppingParamsForK7 =  croppingParams[this.cassetteName.replace(/_.*/,'')];

        let params = [croppingParamsForK7].concat(this.acquisitionParams);

        let payload = {
            study: this.name,
            date: this.date,
            compute_concentration: false,
            snapshot: snapshot.fileName,
            params: params,
            cassette_name: this.cassetteName
        };
        return payload;
    }

    addAcquisitionWithDefaults() {
        let isSeries3 = this.deviceName.match(/^lmx-3/);
        let defaultCameraParams = isSeries3 ? DEFAULT_CAMERA_PARAMS.series3 : DEFAULT_CAMERA_PARAMS.series2;
        let defaultCalibration = DEFAULT_CALIBRATION_PARAMS[this.deviceName].LEDs_params;

        let defaultAcquisition = { ...defaultCameraParams };
        let leds = {
            uv: {
                intensity: defaultCalibration.uv.intensity || 255,
                status: "enabled"
            },
            visible: {
                intensity: defaultCalibration.visible.intensity || 0,
                status: "disabled"
            }
        };
        defaultAcquisition.LEDs_params = leds;
        defaultAcquisition.isValid = true;

        this.acquisitionParams[this.acquisitionParams.length] = defaultAcquisition;
    }

    isLastConcentration(concentrationName) {
        let list = [...this.concentrationList.keys()];
        return concentrationName === list[list.length - 1];
    }
}
