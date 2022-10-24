import React from 'react';
import {
    Box,
    Grid,
    Button,
    IconButton,
    MenuItem,
    Select,
    TextField,
    Dialog,
    DialogActions,
    DialogTitle,
    Slider,
    Input
} from '@material-ui/core';
import { RotateLeft } from '@material-ui/icons';
import { red, blue } from '@material-ui/core/colors';

import DEFAULT_CAMERA_PARAMS from '../Assets/default-camera-params.json';

const minLEDIntensity = 0;
const maxLEDIntensity = 255;

export default class MeasureParamsWidget extends React.Component {
    constructor(props) {
        super(props);

        this.currentParams = props.measureParams;
        this.isDeviceSeries3  = props.isDeviceSeries3 || false;

        let defaultLEDType = "uv";
        let defaultIntensity = maxLEDIntensity;
        let defaultCameraParams = Object.assign({}, this.currentParams);
        delete defaultCameraParams.LEDs_params;
        delete defaultCameraParams.isValid;
        delete defaultCameraParams.strip_packaging;

        if (this.currentParams?.LEDs_params) {
            defaultLEDType = this.currentParams.LEDs_params.visible.status === "enabled" ? "visible" : "uv";
            defaultIntensity = this.currentParams.LEDs_params.visible.status === "enabled" ? this.currentParams.LEDs_params.visible.intensity : this.currentParams.LEDs_params.uv.intensity;
        }

        this.state = {
            LEDType: defaultLEDType,
            LEDIntensity: defaultIntensity,
            cameraParams: JSON.stringify(defaultCameraParams, null, 4),
            isInputValid: true,
            inputErrorMessage: "",
            openResetDialog: false
        };
    }

    asyncSetState = async (state) => {
        await new Promise(resolve => this.setState(state, resolve));
    }

    handleSelectLEDTypeChange = async (event) => {

        let intensity = this.currentParams.LEDs_params[event.target.value].intensity;
        await this.asyncSetState({ LEDType: event.target.value, LEDIntensity: intensity });
        if (this.props.onParamsChanged) {
            this.props.onParamsChanged(this.getMeasureParams());
        }
    };

    handleSliderChange = async (event, value) => {
        await this.setLEDIntensity(value);
    };

    setLEDIntensity = async (value) => {
        if (value < minLEDIntensity) {
            value = minLEDIntensity;
        } else if (value > maxLEDIntensity) {
            value = maxLEDIntensity;
        }
        await this.asyncSetState({ LEDIntensity: value });
        if (this.props.onParamsChanged) {
            this.props.onParamsChanged(this.getMeasureParams());
        }
    };

    handleCameraParamsChange = async (event) => {
        await new Promise(resolve => this.setState({ cameraParams: event.target.value }, resolve));
        try {
            let params = JSON.parse(event.target.value);
            await this.checkParamsValidity(params);
        }
        catch (err) {
            await this.asyncSetState({ 
                isInputValid: false,
                inputErrorMessage: 'Invalid format : ' + err
            });
        }
        if (this.props.onParamsChanged) {
            this.props.onParamsChanged(this.getMeasureParams());
        }
    };

    displayInputError = (text) => {
        this.setState({
            inputErrorMessage: text
        });
    };

    checkParamsValidity = async (paramsObj) => {
        let paramsValidity = true;
        if (paramsObj.color_depth) {
            if (paramsObj.color_depth !== "8bits" && paramsObj.color_depth !== "10bits" && paramsObj.color_depth !== "12bits") {
                this.displayInputError("Invalid color depth");
                paramsValidity = false;
            }
        }
        if (paramsObj.awb) {
            if (paramsObj.awb !== "one" && paramsObj.awb !== "off" && paramsObj.awb !== "once") {
                this.displayInputError("Invalid awb");
                paramsValidity = false;
            }
        }
        if (paramsObj.color_channel) {
            if (paramsObj.color_channel !== "red" && paramsObj.color_channel !== "green" && paramsObj.color_channel !== "blue") {
                this.displayInputError("Invalid color channel");
                paramsValidity = false;
            }
        }
        for (const key in paramsObj) {
            if (key === "roi_offset" || key === "blacklevel_offset") {
                if (Number.parseInt(paramsObj[key]) === NaN) {
                    this.displayInputError("ROI Offset is not a number");
                    paramsValidity = false;
                }
            } else if (paramsObj[key] < 0 && key.startsWith("gain_")) {
                this.displayInputError("Min. value is 0");
                paramsValidity = false;
            } else if (paramsObj[key] > 100 && key.startsWith("gain_")) {
                this.displayInputError("Gain max. is 100");
                paramsValidity = false;
            } else if (paramsObj[key] > 5000 && key === 'exposure') {
                this.displayInputError("Exposure max. is 5000");
                paramsValidity = false;
            }
        }
        await this.asyncSetState({ isInputValid: paramsValidity });
    };

    resetToCameraDefault = async () => {
        let cameraParams = (this.isDeviceSeries3 ? DEFAULT_CAMERA_PARAMS.series3 : DEFAULT_CAMERA_PARAMS.series2);
        await this.asyncSetState({
            cameraParams: JSON.stringify(cameraParams, null, 4),
            isInputValid: true,
            openResetDialog: false
        });
    };

    getLEDParams = () => {
        let activeLed = {
            status: "enabled",
            intensity: this.state.LEDIntensity
        };
        let inactiveLed = {
            status: "disabled",
            intensity: 0
        };

        return {
            uv: (this.state.LEDType === "uv") ? activeLed : inactiveLed,
            visible: (this.state.LEDType === "uv") ? inactiveLed : activeLed
        };
    }

    getMeasureParams = () => {

        let params = {
            LEDs_params: this.getLEDParams(),
            isValid: true
        };
        try {
            let cameraParams = JSON.parse(this.state.cameraParams);
            params.isValid = this.state.isInputValid;
            params = { ...params, ...cameraParams };
        }
        catch (err) {
            params.isValid = false;
        }

        return params;
    }

    // -------------------------------------------

    handleOpenResetDialog = () => {
        this.setState({ openResetDialog: true });
    };

    handleCloseResetDialog = () => {
        this.setState({ openResetDialog: false });
    };

    renderResetDialog = () => {
        return (
            <Dialog open={this.state.openResetDialog} onClose={this.handleCloseResetDialog}>
                <DialogTitle>Reset camera params to default value ?</DialogTitle>
                <DialogActions>
                    <Button onClick={this.handleCloseResetDialog}>
                        Cancel
                    </Button>
                    <Button onClick={this.resetToCameraDefault}>
                        Reset
                    </Button>
                </DialogActions>
            </Dialog>
        );
    }

    handleInputChange = (event) => {
        this.setLEDIntensity(event.target.value === '' ? 0 : Number(event.target.value));
      };
    
    render() {
        return (
            <Box style={localStyle.box}>
                <Grid container spacing={2}>
                    <Grid item xs={4}>
                        <Select
                            id="demo-simple-select-outlined"
                            value={this.state.LEDType}
                            onChange={this.handleSelectLEDTypeChange}
                            label="LED Type"
                        >
                            <MenuItem value="uv">LEDs UV</MenuItem>
                            <MenuItem value="visible">LEDs Visible</MenuItem>
                        </Select>
                    </Grid>
                    <Grid item xs={6}>
                        <Slider
                            value={this.state.LEDIntensity}
                            onChangeCommitted={this.handleSliderChange}
                            min={minLEDIntensity}
                            max={maxLEDIntensity}
                            step={1}
                            style={localStyle.color}
                        />
                    </Grid>
                    <Grid item xs={2}>
                        <Input
                            value={this.state.LEDIntensity}
                            margin="dense"
                            onChange={this.handleInputChange}
                            inputProps={{
                                step: 1,
                                min: 0,
                                max: 255,
                                type: 'number'
                                //'aria-labelledby': 'input-slider',
                            }}
                        />
                    </Grid>
                    <Grid item xs={9}>
                        <TextField
                            value={this.state.cameraParams}
                            label='Camera Params'
                            variant='outlined'
                            onChange={this.handleCameraParamsChange}
                            multiline
                            rows={9}
                            fullWidth
                            error={!this.state.isInputValid}
                            helperText={this.state.isInputValid ? '' : this.state.inputErrorMessage }
                        />
                    </Grid>
                    <Grid item xs={3}>
                        <IconButton onClick={this.handleOpenResetDialog} style={{
                            ...localStyle.iconButton,
                            marginTop: 45
                        }}>
                            <RotateLeft />
                        </IconButton>
                        {this.renderResetDialog()}
                    </Grid>
                </Grid>
            </Box>
        );
    }
}

const localStyle = {
    box: {
        marginTop: 20
    },
    color: {
        color: red[900]
    },
    iconButton: {
        backgroundColor: red[900],
        color: 'white',
        margin: 10
    }    
}
