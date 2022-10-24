import { currentDate } from '../Tools/CurrentDate';

export async function getParamsToSave(device, cameraParams) {
    let paramsToSave = await device.info();
    let LEDs_params = {
        visible: {
            enabled: device.LEDs.visible.enabled,
            intensity: device.LEDs.visible.intensity
        },
        uv: {
            enabled: device.LEDs.uv.enabled,
            intensity: device.LEDs.uv.intensity
        }
    }
    paramsToSave.camera_params = JSON.parse(cameraParams);
    paramsToSave.date = currentDate();
    paramsToSave.LEDs_params = LEDs_params;
    return JSON.stringify(paramsToSave, undefined, 4);
}

export async function getCalibrationStudyParamsToSave(device, cameraParams) {
    let paramsToSave = await device.info();
    paramsToSave.acquisition = cameraParams;
    paramsToSave.date = currentDate();
    return JSON.stringify(paramsToSave, undefined, 4);
}