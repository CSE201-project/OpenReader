'use strict';
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
    DialogContent,
    DialogTitle,
    FormControlLabel,
    Switch
} from '@material-ui/core';
import { PhotoCamera, Save, Done, RotateLeft, ArrowBackOutlined, Close } from '@material-ui/icons';
import { red, green, blue } from '@material-ui/core/colors';

import LEDGroup from '../Components/LEDGroup';
import DeviceInfosWidget from '../Components/DeviceInfosWidget';
import PictureCanvasRGB from '../Components/PictureCanvasRGB';

import BurstMode from '../Services/BurstMode';
import { blobToBase64 } from '../Services/Converter';
import { currentDate } from '../Tools/CurrentDate';
import { getParamsToSave } from '../Tools/SaveParams';

const fs = window.fs;
const dialog = window.dialog;
const Jimp = window.Jimp;
const configDir = window.configDir;

const DEFAULT_CAMERA_PARAMS = require('../Assets/default-camera-params.json');
const CAMERA_PARAMS_FILEPATH = configDir + '/camera-params.json';
const BASE64_HEADER = /^data:image\/\w+;base64,/;

class CalibrationScreen extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            image: '',
            pathToSave: '',
            isLoading: false,
            saved: false,
            histRedObject: {
                svgstring: '',
                min: '',
                max: '',
                moy: '',
                med: '',
                mod: ''
            },
            histGreenObject: {
                svgstring: '',
                min: '',
                max: '',
                moy: '',
                med: '',
                mod: ''
            },
            histBlueObject: {
                svgstring: '',
                min: '',
                max: '',
                moy: '',
                med: '',
                mod: ''
            },
            params: 'loading...',
            showHisto: false,
            showMulti: false,
            showBurstUVInput: false,
            showBurstVisibleInput: false,
            showBurstCameraInput: false,
            intensityInput: '0',
            imgNumberInput: '1',
            intervalInput: '1000',
            inLoop: false,
            leave: false,
            savedPicture: 0,
            paramsValidity: true,
            helperTextParams: "",
            openResetCameraParamsConfirmationDialog: false,
            fetchPrecropROI: false
        };
        this.abortBurstMode = false;
    }

    async componentDidMount() {
        let cameraParams = JSON.stringify(await this.getCameraParams(), undefined, 4);
        this.setState({ params: cameraParams });        
    }

    openResetCameraParamsConfirmationDialog = () => {
        this.setState({ openResetCameraParamsConfirmationDialog: true });
    }

    closeResetCameraParamsConfirmationDialog = () => {
        this.setState({ openResetCameraParamsConfirmationDialog: false });
    }

    roundToTwo(num) {
        return +(Math.round(num + "e+2") + "e-2");
    }

    handleOpenHisto = () => {
        this.setState({ showHisto: true });
    }

    handleCloseHisto = () => {
        this.setState({ showHisto: false });
    }

    handleOpenBurstMode = () => {
        if (this.state.paramsValidity) {
            this.setState({ showMulti: true });
        } else {
            dialog.showErrorBox('Invalid camera params', '');
        }
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

    getDefaultCameraParams(isSeries3) {
        let currentCameraParams;
        try {
            let defaultCameraParams = DEFAULT_CAMERA_PARAMS;
            currentCameraParams = isSeries3 ? defaultCameraParams.series3 : defaultCameraParams.series2;
            return currentCameraParams;
        }
        catch {
            console.error('Could not get default camera params');
            return {};
        }
    }

    async resetCameraParams() {
        let cameraParams = this.getDefaultCameraParams(this.props.device.isSeries3);
        this.writeCameraParams(cameraParams);
        this.setState({
            params: JSON.stringify(cameraParams, undefined, 4),
            paramsValidity: true,
            helperTextParams: "",
            openResetCameraParamsConfirmationDialog: false
        });
    }

    writeCameraParams(params) {
        if (this.state.paramsValidity) {
            fs.writeFileSync(CAMERA_PARAMS_FILEPATH, JSON.stringify(params));
        }
    }

    async getCameraParams() {
        let cameraParams = {};

        if (fs.existsSync(CAMERA_PARAMS_FILEPATH)) {
            let paramsFileContent = fs.readFileSync(CAMERA_PARAMS_FILEPATH);
            cameraParams = JSON.parse(paramsFileContent);
        } else {
            cameraParams = this.getDefaultCameraParams(this.props.device.isSeries3);
        }
        return cameraParams;
    }

    displayParamsError(error) {
        this.setState({
            paramsValidity: false,
            helperTextParams: error
        });
    }

    async checkParamsValidity(paramsObj) {
        if (paramsObj.color_depth) {
            if (paramsObj.color_depth !== "8bits" && paramsObj.color_depth !== "10bits" && paramsObj.color_depth !== "12bits") {
                this.displayParamsError("Invalid color depth");
                this.setState({ paramsValidity: false });
            }
        }
        if (paramsObj.awb) {
            if (paramsObj.awb !== "one" && paramsObj.awb !== "off" && paramsObj.awb !== "once") {
                this.displayParamsError("Invalid awb");
                this.setState({ paramsValidity: false });
            }
        }
        if (paramsObj.color_channel) {
            if (paramsObj.color_channel !== "red" && paramsObj.color_channel !== "green" && paramsObj.color_channel !== "blue") {
                this.displayParamsError("Invalid color channel");
                this.setState({ paramsValidity: false });
            }
        }
        for (const key in paramsObj) {
            if (key.startsWith('roi_')) {
                try {
                    let roinumber = parseInt(paramsObj[key]);
                } catch (error) {
                    this.displayParamsError(`${key} is not a number`);
                    this.setState({ paramsValidity: false });    
                }
            } else if ((key === 'exposure' || key.startsWith('gain_')) && paramsObj[key] < 0) {
                this.displayParamsError("Min. value is 0");
                this.setState({ paramsValidity: false });
            } else if (key.startsWith('gain_') && paramsObj[key] > 100) {
                this.displayParamsError("Gain max. is 100");
                this.setState({ paramsValidity: false });
            } else if (paramsObj[key] > 132 && key === 'exposure') {
                this.displayParamsError("Exposure max. is 132");
                this.setState({ paramsValidity: false });
            }
        }
        return paramsObj;
    }

    handleParamsInput = async (event) => {
        let params = event.target.value;
        this.setState({
            params: params,
            helperTextParams: ""
        });

        try {
            this.setState({ paramsValidity: true });
            let paramsObj = await this.checkParamsValidity(JSON.parse(params));
            this.writeCameraParams(paramsObj);
        }
        catch (err) {
            console.log('error', err);
            this.setState({ paramsValidity: false });
        }
    }

    async createHistogram(pictureData) {
        const imghist = require('../Services/ImageHistograms.js');

        if (pictureData !== null) {
            let photo = await Jimp.read(await pictureData.arrayBuffer());
            let colorDepth = JSON.parse(this.state.params).color_depth;

            let histRedObject = imghist.histogramRGB(imghist.colorChannels.Red, photo, parseInt(colorDepth));
            this.setState({ histRedObject: histRedObject });

            let histGreenObject = imghist.histogramRGB(imghist.colorChannels.Green, photo, parseInt(colorDepth));
            this.setState({ histGreenObject: histGreenObject });

            let histBlueObject = imghist.histogramRGB(imghist.colorChannels.Blue, photo, parseInt(colorDepth));
            this.setState({ histBlueObject: histBlueObject });
        }
    }

    async takePicture() {

        let roiSize = this.state.fetchPrecropROI ? 'cropped' : 'full';

        this.setState({
            showProfile: false,
            isLoading: true,
            image: '',
            histoRed: '',
            histoGreen: '',
            histoBlue: ''
        });

        if (this.state.paramsValidity) {
            let pictureParams = { roi_size: roiSize, ...JSON.parse(this.state.params)};
            let pictureData = await this.props.device.takePicture(pictureParams);
            if (pictureData === null || pictureData.size === 0) {
                dialog.showErrorBox("Error while taking picture", "Something happened with the camera");
                this.setState({ isLoading: false });
                return;
            }
            let image = await blobToBase64(pictureData);
            await this.createHistogram(pictureData);
            this.setState({ image: image });
        } else {
            dialog.showErrorBox('Invalid camera params', '');
        }
        this.setState({ isLoading: false });
    }

    openDialog = () => {
        let pathArray = dialog.showOpenDialogSync({ properties: ['openDirectory'] });
        if (pathArray)
            this.setState({ pathToSave: pathArray[0].concat('/') });
    }

    displaySaveIcon() {
        this.setState({ saved: true });
        setTimeout(() => {
            this.setState({ saved: false });
        }, 1500);
    }

    async saveFullPicture() {
        let date = currentDate();

        if (this.state.pathToSave[0] && this.state.image[0]) {
            let imagePath = this.state.pathToSave + `/image_` + date + '.png';
            let paramsPath = this.state.pathToSave + `/image_` + date + '.txt';

            fs.promises.writeFile(imagePath, this.state.image.replace(BASE64_HEADER, ""), 'base64');
            fs.promises.writeFile(paramsPath, await getParamsToSave(this.props.device, this.state.params));
            this.displaySaveIcon();
        } else if (!this.state.pathToSave[0] && this.state.image[0]) {
            let pathArray = dialog.showOpenDialogSync({ properties: ['openDirectory'] });
            if (pathArray) {
                let imagePath = pathArray[0].concat('/') + '/image_' + date + '.png';
                this.setState({ pathToSave: pathArray[0] });
                let paramsPath = imagePath.replace(".png", ".txt");
                fs.promises.writeFile(imagePath, this.state.image.replace(BASE64_HEADER, ""), 'base64');
                fs.promises.writeFile(paramsPath, await getParamsToSave(this.props.device, this.state.params));
                this.displaySaveIcon();
            }
        } else
            dialog.showErrorBox('Error', 'Take a picture before saving')
    }

    handleBurstModeAbort() {
        this.abortBurstMode = true;
        this.setState({ inLoop: false });
    }

    leaveScreen = async () => {
        this.handleBurstModeAbort();
        if (this.props.location.caller === '/') {
            await this.props.device.ledsEnableCommand("uv");
            await this.props.device.ledsEnableCommand("visible");
        }
    }

    resetCameraParamsConfirmationDialog() {
        return (
            <Dialog open={this.state.openResetCameraParamsConfirmationDialog} onClose={this.closeResetCameraParamsConfirmationDialog}>
                <DialogTitle>Reset camera params to default value ?</DialogTitle>
                <DialogActions>
                    <Button onClick={this.closeResetCameraParamsConfirmationDialog}>
                        Cancel
                    </Button>
                    <Button onClick={() => this.resetCameraParams()}>
                        Reset
                    </Button>
                </DialogActions>
            </Dialog>
        );
    }

    renderAppBar() {
        return (
            <AppBar position='sticky' style={styles.appBar}>
                <Grid container alignItems='center'>
                    <Grid item xs={4}>
                        <IconButton
                            style={styles.appBarButton}
                            onClick={this.leaveScreen}
                            component={Link}
                            to={{
                                pathname: this.props.location.caller,
                                studyObject: this.props.location.studyObject,
                                concentrationName: this.props.location.concentrationName
                            }}
                        >
                            <ArrowBackOutlined />
                        </IconButton>
                    </Grid>
                    <Grid item xs={4}>
                        <Typography style={styles.title} variant="h4" gutterBottom>
                            Device control
                        </Typography>
                    </Grid>
                    <Grid item xs={4}>
                        <Typography variant='subtitle2' >
                            {`Connected to ${this.props.device.Name}`}
                        </Typography>
                    </Grid>
                </Grid>
            </AppBar>
        );
    }

    renderCameraParams() {
        return (
            <Box style={styles.params}>
                <Grid container>
                    <Grid item xs={9}>
                        <TextField
                            value={this.state.params}
                            label='Camera Params'
                            variant='outlined'
                            onChange={this.handleParamsInput}
                            multiline
                            rows={6}
                            fullWidth
                            error={!this.state.paramsValidity}
                            helperText={this.state.helperTextParams}
                        />
                    </Grid>
                    <Grid item xs={3}>
                        <IconButton onClick={this.openResetCameraParamsConfirmationDialog} style={{
                            ...styles.iconButton,
                            marginTop: 45
                        }}>
                            <RotateLeft />
                        </IconButton>
                        {this.resetCameraParamsConfirmationDialog()}
                    </Grid>
                </Grid>
            </Box>
        );
    }

    renderHistogramButton() {
        return (
            <div>
                <Button style={styles.button} size='large' variant='contained' onClick={this.handleOpenHisto}>
                    Histogram
                </Button>
                <Dialog open={this.state.showHisto} onClose={this.handleCloseHisto}>
                    <DialogContent>
                        <Card variant="outlined">
                            <span dangerouslySetInnerHTML={{ __html: this.state.histRedObject.svgstring }} />
                            <br />
                            <span dangerouslySetInnerHTML={{ __html: this.state.histGreenObject.svgstring }} />
                            <br />
                            <span dangerouslySetInnerHTML={{ __html: this.state.histBlueObject.svgstring }} />
                        </Card>
                    </DialogContent>
                    <DialogActions>
                        <Card style={{ backgroundColor: red[700], marginRight: 25 }}>
                            <Typography>min: {this.state.histRedObject.min}</Typography>
                            <Typography>max: {this.roundToTwo(this.state.histRedObject.max)}</Typography>
                            <Typography>moy: {this.roundToTwo(this.state.histRedObject.moy)}</Typography>
                            <Typography>med: {this.roundToTwo(this.state.histRedObject.med)}</Typography>
                            <Typography>mod: {this.state.histRedObject.mod}</Typography>
                        </Card>
                        <Card style={{ backgroundColor: green[700], marginRight: 25 }}>
                            <Typography>min: {this.state.histGreenObject.min}</Typography>
                            <Typography>max: {this.roundToTwo(this.state.histGreenObject.max)}</Typography>
                            <Typography>moy: {this.roundToTwo(this.state.histGreenObject.moy)}</Typography>
                            <Typography>med: {this.roundToTwo(this.state.histGreenObject.med)}</Typography>
                            <Typography>mod: {this.state.histGreenObject.mod}</Typography>
                        </Card>
                        <Card style={{ backgroundColor: blue[700], marginRight: 50 }}>
                            <Typography>min: {this.state.histBlueObject.min}</Typography>
                            <Typography>max: {this.roundToTwo(this.state.histBlueObject.max)}</Typography>
                            <Typography>moy: {this.roundToTwo(this.state.histBlueObject.moy)}</Typography>
                            <Typography>med: {this.roundToTwo(this.state.histBlueObject.med)}</Typography>
                            <Typography>mod: {this.state.histBlueObject.mod}</Typography>
                        </Card>

                        <IconButton onClick={this.handleCloseHisto} style={styles.button}>
                            <Close />
                        </IconButton>
                    </DialogActions>
                </Dialog>
            </div>
        );
    }

    handleOpenBurstUVInput = () => {
        this.setState({ showBurstUVInput: true });
    }

    handleCloseBurstUVInput = () => {
        this.setState({ showBurstUVInput: false });
    }

    handleOpenBurstVisibleInput = () => {
        this.setState({ showBurstVisibleInput: true });
    }

    handleCloseBurstVisibleInput = () => {
        this.setState({ showBurstVisibleInput: false });
    }

    handleOpenCameraInput = () => {
        this.setState({ showBurstCameraInput: true });
    }

    handleCloseCameraInput = () => {
        this.setState({ showBurstCameraInput: false });
    }

    renderCameraParamsDialog() {
        return (
            <div>
                <Button disabled={this.state.inLoop} variant='contained' style={styles.burstButton} onClick={() => this.handleOpenCameraInput()} >
                    Camera Params
                </Button>
                <Dialog open={this.state.showBurstCameraInput} onClose={this.handleCloseCameraInput} >
                    <DialogTitle>{"Sequence Params"}</DialogTitle>
                    <DialogContent>
                        <Grid container>
                            <Grid item xs={6}>
                                <TextField
                                    value={this.state.intensityInput}
                                    onChange={this.handleIntensityInput}
                                    variant='outlined'
                                    label='Intensity'
                                    type='number'
                                />
                            </Grid>
                            <Grid item xs={6}>
                                <TextField
                                    value={this.state.imgNumberInput}
                                    onChange={this.handleImgNumberInput}
                                    variant='outlined'
                                    label='Picture number'
                                    type='number'
                                />
                            </Grid>
                        </Grid>
                    </DialogContent>
                    <DialogActions>
                        <Button variant='contained' style={styles.button} onClick={() => this.burstModeCameraParams()}>
                            Start Sequence
                        </Button>
                        <IconButton onClick={this.handleCloseCameraInput} style={styles.button}>
                            <Close />
                        </IconButton>
                    </DialogActions>
                </Dialog>
            </div>
        );
    }

    renderSequenceUV() {
        return (
            <div>
                <Button disabled={this.state.inLoop} variant='contained' style={styles.burstButton} onClick={() => this.handleOpenBurstUVInput()} >
                    Sequence UV
                </Button>
                <Dialog open={this.state.showBurstUVInput} onClose={this.handleCloseBurstUVInput} >
                    <DialogTitle>{"Sequence Params"}</DialogTitle>
                    <DialogContent>
                        <Grid container>
                            <Grid item xs={6}>
                                <TextField
                                    value={this.state.intensityInput}
                                    onChange={this.handleIntensityInput}
                                    variant='outlined'
                                    label='Intensity'
                                    type='number'
                                />
                            </Grid>
                            <Grid item xs={6}>
                                <TextField
                                    value={this.state.imgNumberInput}
                                    onChange={this.handleImgNumberInput}
                                    variant='outlined'
                                    label='Picture number'
                                    type='number'
                                />
                            </Grid>
                        </Grid>
                    </DialogContent>
                    <DialogActions>
                        <Button variant='contained' style={styles.button} onClick={() => this.burstModeSequenceUV()}>
                            Start Sequence
                        </Button>
                        <IconButton onClick={this.handleCloseBurstUVInput} style={styles.button}>
                            <Close />
                        </IconButton>
                    </DialogActions>
                </Dialog>
            </div>
        );
    }

    renderSequenceVisible() {
        return (
            <div>
                <Button disabled={this.state.inLoop} variant='contained' style={styles.burstButton} onClick={() => this.handleOpenBurstVisibleInput()} >
                    Sequence Visible
                </Button>
                <Dialog open={this.state.showBurstVisibleInput} onClose={this.handleCloseBurstVisibleInput} >
                    <DialogTitle>{"Sequence Params"}</DialogTitle>
                    <DialogContent>
                        <Grid container>
                            <Grid item xs={6}>
                                <TextField
                                    value={this.state.intensityInput}
                                    onChange={this.handleIntensityInput}
                                    variant='outlined'
                                    label='Intensity'
                                    type='number'
                                />
                            </Grid>
                            <Grid item xs={6}>
                                <TextField
                                    value={this.state.imgNumberInput}
                                    onChange={this.handleImgNumberInput}
                                    variant='outlined'
                                    label='Picture number'
                                    type='number'
                                />
                            </Grid>
                        </Grid>
                    </DialogContent>
                    <DialogActions>
                        <Button variant='contained' style={styles.button} onClick={() => this.burstModeSequenceVisible()}>
                            Start Sequence
                        </Button>
                        <IconButton onClick={this.handleCloseBurstVisibleInput} style={styles.button}>
                            <Close />
                        </IconButton>
                    </DialogActions>
                </Dialog>
            </div>
        );
    }

    renderBurstButton() {
        return (
            <div>
                <Button disabled={!this.state.pathToSave[0]} variant='outlined' onClick={this.handleOpenBurstMode}>
                    BurstMode
                </Button>
                {this.state.inLoop && <CircularProgress size={24} style={styles.burstIndicator} />}
                <Dialog open={this.state.showMulti} onClose={this.handleCloseBurstMode}>
                    <DialogTitle>{"Burst Mode"}</DialogTitle>
                    <DialogContent>
                        {!this.state.inLoop &&
                            <TextField
                                value={this.state.intervalInput}
                                onChange={this.handleIntervalInput}
                                variant='outlined'
                                label='Interval (ms)'
                                type='number'
                            />
                        }
                        {!this.state.inLoop &&
                            <Grid container direction='column' alignItems='center'>
                                <Grid item>
                                    {this.renderSequenceUV()}
                                </Grid>
                                <Grid item>
                                    {this.renderSequenceVisible()}
                                </Grid>
                                <Grid item>
                                    <Button disabled={this.state.inLoop} variant='contained' style={styles.burstButton} onClick={() => this.burstModeAllIntensityUV()}>
                                        All Intensity UV
                                    </Button>
                                </Grid>
                                <Grid item>
                                    <Button disabled={this.state.inLoop} variant='contained' style={styles.burstButton} onClick={() => this.burstModeAllIntensityVisible()}>
                                        All Intensity Visible
                                    </Button>
                                </Grid>
                                <Grid item>
                                    {this.renderCameraParamsDialog()}
                                </Grid>
                            </Grid>
                        }
                        {this.state.inLoop &&
                            <Card style={{ margin: 5 }} >
                                <CardMedia component="img" src={this.state.image} />
                            </Card>
                        }
                    </DialogContent>
                    <DialogActions>
                        {this.state.inLoop &&
                            <Typography style={styles.saveCount}>
                                {`saved: ${this.state.savedPicture}/${this.state.imgNumberInput}`}
                            </Typography>
                        }
                        <Button
                            variant='contained'
                            style={{
                                color: 'white',
                                backgroundColor: this.state.inLoop ? red[900] : 'grey',
                                marginTop: 50,
                                marginRight: 20
                            }}
                            disabled={!this.state.inLoop}
                            onClick={() => this.handleBurstModeAbort()}
                        >
                            Abort Burst Mode
                        </Button>
                        <IconButton onClick={this.handleCloseBurstMode} style={styles.button}>
                            <Close />
                        </IconButton>
                    </DialogActions>
                </Dialog>
            </div>
        );
    }

    async burstModeCameraParams() {
        this.setState({ showBurstCameraInput: false });
        this.abortBurstMode = false;

        this.setState({
            inLoop: true,
            savedPicture: 0
        });

        await BurstMode.sequence({
            device: this.props.device,
            imgNumber: this.state.imgNumberInput,
            interval: this.state.intervalInput,
            pathToSave: this.state.pathToSave,
            cameraParams: this.state.params,
            abort: this.abortBurstMode
        }, (image, saveCount) => {
            this.setState({
                image: image,
                savedPicture: saveCount
            });
        }, () => {
            return this.abortBurstMode;
        });
        this.setState({ inLoop: false });
    }

    async burstModeSequenceUV() {
        this.setState({ showBurstUVInput: false });

        let intensity = this.state.intensityInput;
        let success = await this.props.device.ledsSetIntensityCommand(
            "uv",
            intensity,
        );
        this.abortBurstMode = false;
        if (success) {
            this.setState({
                inLoop: true,
                savedPicture: 0
            });

            await BurstMode.sequence({
                device: this.props.device,
                imgNumber: this.state.imgNumberInput,
                interval: this.state.intervalInput,
                pathToSave: this.state.pathToSave,
                cameraParams: this.state.params,
                abort: this.abortBurstMode
            }, (image, saveCount) => {
                this.setState({
                    image: image,
                    savedPicture: saveCount
                });
            }, () => {
                return this.abortBurstMode;
            });
            this.setState({ inLoop: false });
        }
    }

    async burstModeSequenceVisible() {
        this.setState({ showBurstVisibleInput: false });

        let value = this.state.intensityInput;
        let success = await this.props.device.ledsSetIntensityCommand(
            "visible",
            value,
        );
        this.abortBurstMode = false;
        if (success) {
            this.setState({
                inLoop: true,
                savedPicture: 0
            });

            await BurstMode.sequence({
                device: this.props.device,
                imgNumber: this.state.imgNumberInput,
                interval: this.state.intervalInput,
                pathToSave: this.state.pathToSave,
                cameraParams: this.state.params,
                abort: this.abortBurstMode
            }, (image, saveCount) => {
                this.setState({
                    image: image,
                    savedPicture: saveCount
                });
            }, () => {
                return this.abortBurstMode;
            });
            this.setState({ inLoop: false });
        }
    }

    async burstModeAllIntensityUV() {
        this.setState({
            inLoop: true,
            savePicture: 0,
            imgNumberInput: 255
        });
        this.abortBurstMode = false;

        await BurstMode.allIntensity({
            device: this.props.device,
            interval: this.state.intervalInput,
            pathToSave: this.state.pathToSave,
            cameraParams: this.state.params,
            type: "uv"
        }, (image, saveCount) => {
            this.setState({
                image: image,
                savedPicture: saveCount
            });
        }, () => {
            return this.abortBurstMode;
        });
        this.setState({ inLoop: false });
    }

    async burstModeAllIntensityVisible() {
        this.setState({
            inLoop: true,
            savePicture: 0,
            imgNumberInput: 255
        });
        this.abortBurstMode = false;

        await BurstMode.allIntensity({
            device: this.props.device,
            interval: this.state.intervalInput,
            pathToSave: this.state.pathToSave,
            cameraParams: this.state.params,
            type: "visible"
        }, (image, saveCount) => {
            this.setState({
                image: image,
                savedPicture: saveCount
            });
        }, () => {
            return this.abortBurstMode;
        });
        this.setState({ inLoop: false });
    }

    handleSwitchCropChange = (event) => {
        this.setState({fetchPrecropROI:event.target.checked});
    }

    render() {
        return (
            <Container>
                {this.renderAppBar()}
                <Grid container >
                    <Grid item xs={6}>
                        <LEDGroup type="UV" device={this.props.device} />
                        <LEDGroup type="Visible" device={this.props.device} />
                    </Grid>
                    <Grid item xs={6}>
                        <FormControlLabel label="PreCrop ?"
                            control={<Switch onChange={this.handleSwitchCropChange} checked={this.state.fetchCropPicture}></Switch>}
                        />
                        <IconButton
                            onClick={async () => await this.takePicture()}
                            style={{
                                backgroundColor: red[900],
                                color: this.state.isLoading ? red[900] : 'white'
                            }}
                        >
                            <PhotoCamera />
                        </IconButton>
                        {this.state.isLoading && <CircularProgress size={24} style={styles.loadingIcon} />}
                        <IconButton
                            onClick={async () => await this.saveFullPicture()}
                            style={{
                                backgroundColor: this.state.image[0] ? red[900] : 'grey',
                                color: 'white',
                                margin: 10
                            }}
                        >
                            {!this.state.saved && <Save />}
                            {this.state.saved && <Done />}
                        </IconButton>
                        {this.renderBurstButton()}
                        <TextField
                            id="directory-selection"
                            label="Select a directory"
                            value={this.state.pathToSave}
                            disabled={!this.state.pathToSave[0]}
                            InputProps={{ readOnly: true }}
                        />
                        <Button size='small'
                            variant='outlined'
                            style={styles.searchButton}
                            onClick={this.openDialog}
                        >
                            ...
                        </Button>
                    </Grid>
                    <Grid item xs={6}>
                        {this.renderCameraParams()}
                        <DeviceInfosWidget device={this.props.device} style={styles.info} />
                    </Grid>
                    <Grid item xs={6}>
                        <PictureCanvasRGB image={this.state.image} />
                        <Box marginTop={-5}>
                            {this.state.image[0] && this.renderHistogramButton()}
                        </Box>
                    </Grid>
                </Grid>
            </Container>
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
        marginTop: 21,
        marginLeft: -36,
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
        marginTop: -15
    },
    info: {
        margin: 2,
        marginTop: 7
    },
    saveCount: {
        marginTop: 50
    }
}

export default CalibrationScreen;