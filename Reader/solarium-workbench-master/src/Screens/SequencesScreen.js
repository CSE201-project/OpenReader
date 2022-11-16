import React from 'react';
import { Link } from 'react-router-dom';
import {
    Container,
    Typography,
    AppBar,
    Box,
    Grid,
    Button,
    IconButton,
    Card,
    CardMedia,
    TextField,
    CircularProgress,
    Dialog,
    DialogActions,
    DialogTitle,
    DialogContent,
    Tabs,
    Tab,
    Switch,
    FormControlLabel
} from '@material-ui/core';
import { PhotoCamera, Add, Save, Done, ArrowBackOutlined, Close } from '@material-ui/icons';
import { red, green, blue } from '@material-ui/core/colors';
import { saveAs } from 'file-saver';

import '../react/Print.css';

import MeasureParamsWidget from '../Components/MeasureParamsWidget';

import ProfileRGB from '../Components/ProfileRGB';
import { blobToBase64 } from '../Services/Converter';
import { currentDate } from '../Tools/CurrentDate';
import { getCalibrationStudyParamsToSave } from '../Tools/SaveParams';
import DeviceInfosWidget from '../Components/DeviceInfosWidget';
import HistogramDialog from '../Components/HistogramDialog';
import PictureCanvasRGB from '../Components/PictureCanvasRGB';
import SequenceLauncherDialog from '../Components/SequenceLauncherDialog';
import SequenceToStudiesDialog from '../Components/SequenceToStudiesDialog';
import K7Chooser from '../Components/K7Chooser';

const fs = window.fs;
const dialog = window.dialog;
const configDir = window.configDir;

const CAMERA_PARAMS_FILEPATH = configDir + '/camera-params.json';
const BASE64_HEADER = /^data:image\/\w+;base64,/;
const CROPPING_PARAMS = require('../Assets/cropping-params.json');
const DEFAULT_CAMERA_PARAMS = require('../Assets/default-camera-params.json');
const DEFAULT_CALIBRATION_PARAMS = require('../Assets/default-calibration.json');

export default class SequencesScreen extends React.Component {
    constructor(props) {
        super(props);

        let cameraParams = this.props.device.isSeries3 ? DEFAULT_CAMERA_PARAMS.series3 : DEFAULT_CAMERA_PARAMS.series2;
        let defaultCalibration = DEFAULT_CALIBRATION_PARAMS[this.props.device.Name].LEDs_params;
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
        let measureParams = { ...cameraParams, LEDs_params:leds, isValid: true };


        this.state = {
            image: '',
            info: 'loading...',
            pathToSave: '',
            isLoading: false,
            params: 'loading...',
            showHisto: false,
            redProfile: '',
            greenProfile: '',
            blueProfile: '',
            showProfile: false,
            loadingInit: true,
            openTakePictureDialog: false,
            openConfirmationDialog: false,
            acquisitionParamsSelected: 0,
            currentMeasureParams: measureParams,
            fetchCropPicture: true,
            stripPackagingCassette: false,
            cassetteName: "K701BT_BL"
        };
    }

    handleOpenHisto = () => {
        this.setState({ showHisto: true });
    }

    handleCloseHisto = () => {
        this.setState({ showHisto: false });
    }

    openConfirmationDialog = () => {
        this.setState({ openConfirmationDialog: true });
    }

    closeConfirmationDialog = () => {
        this.setState({ openConfirmationDialog: false });
    }

    openTakePictureDialog = () => {
        console.log()
        this.setState({ openTakePictureDialog: true });
    }

    closeTakePictureDialog = () => {
        this.setState({ openTakePictureDialog: false });
    }


    handleCloseBurstMode = () => {
        this.setState({ showMulti: false });
    }

    handleIntensityInput = (event) => {
        this.setState({ intensityInput: event.target.value });
    }

    handleImgNumberInput = (event) => {
        this.setState({ imgNumberInput: event.target.value });
    }

    handleIntervalInput = (event) => {
        this.setState({ intervalInput: event.target.value });
    }

    handleLaunchSequence = (sequenceName, sequenceInterval, sequenceCount) => {
        let currentParams = this.state.currentMeasureParams;
        currentParams.strip_packaging = this.state.stripPackagingCassette ? "cassette" : "strip";
        currentParams.sequence_name = sequenceName;
        currentParams.sequence_interval = sequenceInterval;
        currentParams.sequence_count = sequenceCount;
        this.props.device.launchSequence(currentParams);
    }

    writeCameraParams(params) {
        if (this.state.paramsValidity) {
            fs.writeFileSync(CAMERA_PARAMS_FILEPATH, JSON.stringify(params));
        }
    }

    takeCalibratedPicture = async () => {
        this.pictureType = this.state.fetchCropPicture ? "cropped" : "full";
        this.setState({
            openTakePictureDialog: false,
            showProfile: false,
            isLoading: true,
            image: '',
            histoRed: '',
            histoGreen: '',
            histoBlue: ''
        });

        if (this.state.currentMeasureParams.isValid) {
            try {
                let currentParams = this.state.currentMeasureParams;
                currentParams.strip_packaging = this.state.stripPackagingCassette ? "cassette" : "strip";

                let croppingParams =  CROPPING_PARAMS[this.props.device.Name];
                let croppingParamsForK7 =  croppingParams[this.state.cassetteName.replace(/_.*/,'')];
                
                let calibrationPayload = this.props.device.buildCalibrationPayload(croppingParamsForK7, currentParams, this.state.cassetteName)
                let measureResult = await this.props.device.takeCalibrationPicture(calibrationPayload);                
                let pictureData = await this.props.device.getFile(measureResult.id, this.pictureType);

                if (pictureData === null) {
                    throw 'Picture is empty';
                }

                let image = await blobToBase64(pictureData);

                if (!image[0]) {
                    dialog.showErrorBox("Error while taking picture", "Solarium device storage might be full");
                }
                //await this.createHistogram(pictureData);

                if (this.pictureType === "cropped") {
                    this.setState({
                        imageBlob: pictureData,
                        image: image,
                        redProfile: measureResult.profile.red,
                        greenProfile: measureResult.profile.green,
                        blueProfile: measureResult.profile.blue,
                        profileRGB: measureResult.profile,
                        showProfile: true
                    });
                } else {
                    this.setState({
                        imageBlob: pictureData,
                        image: image
                    });
                }
            } catch (err) {
                console.log('takeCalibratedPicture at l.169:', err);
            }
        } else {
            console.log('this.state.currentMeasureParams:', this.state.currentMeasureParams);
            dialog.showErrorBox('Invalid camera params', '');
        }
        this.setState({ isLoading: false });
    }

    openChooseFolderDialog = () => {
        let pathArray = dialog.showOpenDialogSync({ properties: ['openDirectory'] });
        if (pathArray)
            this.setState({ pathToSave: pathArray[0].concat('/') });
    }

    async saveProfileRGB(date) {
        let canvas = document.getElementById('profileRGBCanvas');

        if (!date)
            date = currentDate();
        if (this.state.pathToSave[0]) {
            let profilePath = this.state.pathToSave + "/profileRGB_" + date + '.png';
            canvas.toBlob(async function (blob) {
                let profile = await blobToBase64(blob);
                fs.promises.writeFile(profilePath, profile.replace(BASE64_HEADER, ""), 'base64');
            });
        } else {
            canvas.toBlob(function (blob) {
                saveAs(blob, "profileRGB_" + date + ".png");
            });
        }
    }

    async savePicture(pictureType) {
        let date = currentDate();

        let basePath = this.state.pathToSave;
        if (!basePath[0]) {
            let pathArray = dialog.showOpenDialogSync({ properties: ['openDirectory'] });
            if (pathArray) {
                basePath = pathArray[0];
                this.setState({ pathToSave: basePath });
            } else {
                return;
            }
        }

        if (this.state.image[0]) {
            let imagePath = basePath + `/image_` + date + '.png';
            let paramsPath = basePath + `/image_` + date + '.txt';

            fs.promises.writeFile(imagePath, this.state.image.replace(BASE64_HEADER, ""), 'base64');
            fs.promises.writeFile(paramsPath, await getCalibrationStudyParamsToSave(this.props.device, this.state.currentMeasureParams));

            if (pictureType === "cropped") {
                await this.saveProfile(date, "red");
                await this.saveProfile(date, "green");
                await this.saveProfile(date, "blue");
            }

        } else {
            dialog.showErrorBox('Error', 'Take a picture before saving');
        }
    }

    renderAppBar() {
        return (
            <AppBar position='sticky' style={styles.appBar}>
                <Grid container alignItems='center'>
                    <Grid item xs={3}>
                        <IconButton
                            style={styles.appBarButton}
                            onClick={this.leaveScreen}
                            component={Link}
                            to="/"
                        >
                            <ArrowBackOutlined />
                        </IconButton>
                    </Grid>
                    <Grid item xs={6}>
                        <Typography style={styles.title} variant="h4" gutterBottom>
                            Sequences Study
                        </Typography>
                        <Typography variant='subtitle2' >
                            {`Connected to ${this.props.device.Name}`}
                        </Typography>
                    </Grid>
                    <Grid item xs={3}></Grid>
                </Grid>
            </AppBar>
        );
    }

    renderProfileRGB() {
        return (
            <Box>
                {this.state.showProfile &&
                    <Card>
                        {!this.state.isLoading && <ProfileRGB profiles={this.state.profileRGB} />}
                        <Button variant='contained' onClick={async () => await this.saveProfileRGB(null)} style={{ color: "white", backgroundColor: red[900], marginLeft: 50 }} >
                            Save
                        </Button>
                    </Card>
                }
            </Box>
        );
    }

    renderHistogramButton() {
        return (
            <div>
                <HistogramDialog photo={this.state.imageBlob} colorDepth={"8"} />
            </div>
        );
    }

    renderSequenceLauncher() {
        return (
            <div>
                <SequenceLauncherDialog onLaunchSequence={this.handleLaunchSequence} />
            </div>
        );
    }

    renderSequences2Studies() {
        return (<SequenceToStudiesDialog dialog={dialog} fs={fs}></SequenceToStudiesDialog>);
    }

    onMeasureParamsChanged = (measureParams) => {
        this.setState({ currentMeasureParams: measureParams });
    }

    handleSwitchCropChange = (event) => {
        this.setState({fetchCropPicture:event.target.checked});
    }

    handleSwitchCassetteChange = (event) => {
        this.setState({stripPackagingCassette:event.target.checked});
    }

    renderPanel() {
        return (
            <Grid container paddingTop={4} >
                <Grid item xs={6}>
                    <MeasureParamsWidget style={{ marginTop: 20 }} 
                        key={this.state.acquisitionParamsSelected} 
                        measureParams={this.state.currentMeasureParams}                        
                        isDeviceSeries3={this.props.device.isSeries3}
                        onParamsChanged={this.onMeasureParamsChanged} 
                    />
                    <DeviceInfosWidget device={this.props.device} style={{ marginTop: 20 }} />
                </Grid>
                <Grid item xs={6}>
                    <FormControlLabel label="Crop ?"
                        control={<Switch onChange={this.handleSwitchCropChange} checked={this.state.fetchCropPicture}></Switch>}
                    />
                    <K7Chooser defaultK7={this.state.cassetteName} onSelectionChanged={(k7) => this.setState({cassetteName:k7})} />
                        
                    <IconButton
                        onClick={this.takeCalibratedPicture}
                        style={{
                            backgroundColor: red[900],
                            color: this.state.isLoading ? red[900] : 'white'
                        }}
                    >
                        <PhotoCamera />
                    </IconButton>

                    {this.state.isLoading && <CircularProgress size={24} style={styles.loadingIcon} />}

                    {this.renderSequenceLauncher()}
                    {this.renderSequences2Studies()}
                    <Box margin={2}></Box>

                    <PictureCanvasRGB image={this.state.image} rotated={!this.state.fetchCropPicture} />
                    {this.renderProfileRGB()}
                    <Box>
                        {this.state.image[0] && this.renderHistogramButton()}
                    </Box>
                </Grid>
            </Grid>
        );
    }

    render() {
        return (
            <Container>
                {this.renderAppBar()}
                {this.renderPanel()}
            </Container >
        );
    }
}

const styles = {
    title: {
        fontFamily: 'revert',
        fontWeight: 'bold',
        color: 'white'
    },
    appBar: {
        backgroundColor: red[900]
    },
    appBarButton: {
        backgroundColor: 'white',
        color: red[900],
        fontFamily: 'revert'
    },
    iconButton: {
        backgroundColor: red[900],
        color: 'white',
        margin: 10
    },
    loadingIcon: {
        color: 'white',
        position: 'absolute',
        marginTop: 12,
        marginLeft: -36,
    },
    initLoadingIcon: {
        color: blue[900],
        position: "absolute",
        marginTop: 13,
        marginLeft: 10
    },
    burstIndicator: {
        color: red[900],
        position: 'absolute',
        marginTop: 5,
        marginLeft: 5,
    },
    button: {
        color: 'white',
        backgroundColor: red[900],
        fontFamily: 'revert',
        marginTop: 50
    },
    burstButton: {
        color: 'white',
        backgroundColor: red[900],
        fontFamily: 'revert',
        marginTop: 20
    },
    searchButton: {
        marginTop: 10,
        marginLeft: 2
    },
    params: {
        margin: 2,
        marginTop: 10
    },
    snapshot: {
        margin: 5,
        marginTop: 5,
        marginLeft: 61
    },
    info: {
        margin: 2,
        marginTop: 7
    },
    saveCount: {
        marginTop: 50
    },
    addButton: {
        marginTop: -10,
        color: red[900]
    },
    removeButton: {
        marginTop: -10,
        color: red[900]
    }
}
