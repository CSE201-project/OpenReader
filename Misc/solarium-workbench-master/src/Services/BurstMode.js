import { blobToBase64 } from '../Services/Converter';
import { currentDate } from '../Tools/CurrentDate';
import { getParamsToSave } from '../Tools/SaveParams';

const fs = window.fs;

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function sequence(props, displayCb, shouldAbortCb) {
    let maxNumber = parseInt(props.imgNumber);
    let interval = parseInt(props.interval);

    for (let i = 0; i < maxNumber; i++) {
        if (shouldAbortCb && shouldAbortCb()) {
            break;
        }
        let date = currentDate();
        let paramsPath = props.pathToSave + `/image_` + date + '.txt';
        let imagePath = props.pathToSave + `/image_` + date + '.png';

        try {
            var pictureData = await props.device.takePicture(props.cameraParams);
            var image = await blobToBase64(pictureData);
        } catch (err) {
            console.log('Error in Sequence: ', err);
            break;
        }
        await fs.promises.writeFile(paramsPath, await getParamsToSave(props.device, props.cameraParams));
        await fs.promises.writeFile(imagePath, image.replace(/^data:image\/\w+;base64,/, ""), 'base64');
        if (displayCb)
            displayCb(image, i + 1);
        await sleep(interval);
    }
};

async function allIntensity(props, displayCb, shouldAbortCb) {
    let interval = parseInt(props.interval);

    for (let intensity = 0; intensity < 255; intensity++) {
        let success = await props.device.ledsSetIntensityCommand(
            props.type,
            intensity
        );
        if (success) {
            if (shouldAbortCb && shouldAbortCb()) {
                break;
            }
            let date = currentDate();
            let paramsPath = props.pathToSave + `/image_` + date + '.txt';
            let imagePath = props.pathToSave + `/image_` + date + '.png';

            try {
                var pictureData = await props.device.takePicture(props.cameraParams);
                var image = await blobToBase64(pictureData);
            } catch (err) {
                console.log('Error in All Intensity: ', err);
                break;
            }
            await fs.promises.writeFile(paramsPath, await getParamsToSave(props.device, props.cameraParams));
            await fs.promises.writeFile(imagePath, image.replace(/^data:image\/\w+;base64,/, ""), 'base64');
            if (displayCb)
                displayCb(image, intensity + 1);
        }
        await sleep(interval);
    }
};

export default {
    sequence,
    allIntensity
};