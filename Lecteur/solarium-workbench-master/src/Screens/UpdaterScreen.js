import React from 'react';
import { Link } from 'react-router-dom';
import {
    AppBar,
    Box,
    Grid,
    Button,
    IconButton,
    TextField,
    Typography,
    LinearProgress
} from '@material-ui/core';
import { ArrowBackOutlined } from '@material-ui/icons';
import { red } from '@material-ui/core/colors';

import { pushFileToDevice } from '../Services/DeviceUpdater';

const path = window.path;
const dialog = window.dialog;
const NodeSSH = window.nodeSsh;

export default class UpdaterScreen extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            deviceIP: props.device.Host,
            deviceName: props.device.Name,
            deviceSolariumVersion: props.device.solariumVersion,
            fileToPush: '',
            updateProgress: 0,
            isUpdating: false
        }
    }

    async openFileChooser() {
        let result = await dialog.showOpenDialog({
            title: 'Select a Solarium binary file',
            properties: ['openFile']
        });
        if (!result.canceled) {
            this.setState({ fileToPush: result.filePaths[0] });
        }
    }

    async pushFileToDevice() {
        if (this.state.fileToPush === "") {
            await this.openFileChooser();
            if (this.state.fileToPush === "") {
                dialog.showErrorBox('No file selected', 'Please select a file to push');
                return;
            }
        }

        if (this.state.deviceIP !== "") {
            this.setState({ isUpdating: true });
            let result = await pushFileToDevice(NodeSSH,
                this.state.fileToPush,
                this.state.deviceIP,
                (progressValue) => {
                    this.setState({ updateProgress: progressValue });
                });
            if (result.success) {
                let info = await this.props.device.info();
                if (info) {
                    this.props.device.solariumVersion = info.version;
                    this.setState({
                        deviceSolariumVersion: info.version,
                        deviceName: info.name
                    });
                } else {
                    this.setState({ deviceSolariumVersion: '#err' });
                    dialog.showErrorBox('Error while updating', 'Could not restart Solarium service');
                }
            } else {
                dialog.showErrorBox('Error while updating', result.message);
            }
            this.setState({ isUpdating: false });
        }
    }

    renderAppBar() {
        return (
            <AppBar position='sticky' style={styles.appBar}>
                <Grid container alignItems='center'>
                    <Grid item xs={4}>
                        <IconButton
                            style={styles.appBarButton}
                            component={Link}
                            to={{
                                pathname: '/'
                            }}
                        >
                            <ArrowBackOutlined />
                        </IconButton>
                    </Grid>
                    <Grid item xs={4}>
                        <Typography style={styles.title} variant="h4" gutterBottom>
                            Updater
                        </Typography>
                    </Grid>
                    <Grid item xs={4}>
                    </Grid>
                </Grid>
            </AppBar>
        );
    }

    renderFileSelection() {
        return (
            <Box paddingTop={2}>
                <TextField
                    id="file-selection"
                    label="Select a file"
                    value={path.basename(this.state.fileToPush)}
                    disabled
                />
                <Button
                    size='small'
                    variant='outlined'
                    style={{ marginTop: 10, marginLeft: 2 }}
                    onClick={async () => { await this.openFileChooser(); }}
                >Choose</Button><br />
                <Button
                    style={styles.button}
                    size='medium'
                    variant='contained'
                    onClick={async () => { await this.pushFileToDevice(); }}
                >
                    Push binary to device !
                    </Button>
            </Box>);
    }

    render() {
        return (
            <Grid container >
                {this.renderAppBar()}
                <Grid item xs={2}></Grid>
                <Grid item xs={8}>
                    <Box paddingTop={10} paddingBottom={12}>
                        <Grid container>
                            <Grid>
                                <Typography>Device IP : {this.state.deviceIP}</Typography>
                            </Grid>
                        </Grid>
                        <Grid container>
                            <Grid>
                                <Typography>Device Name : {this.state.deviceName}</Typography>
                            </Grid>
                        </Grid>
                        <Grid container>
                            <Grid>
                                <Typography>Solarium Version : {this.state.deviceSolariumVersion}</Typography>
                            </Grid>
                        </Grid>
                        {!this.state.isUpdating && this.renderFileSelection()}
                        {this.state.isUpdating && <Box paddingTop={10}><LinearProgress variant="determinate" value={this.state.updateProgress} /></Box>}
                    </Box>
                </Grid>
                <Grid item xs={2}></Grid>
            </Grid>
        );
    }
}

const styles = {
    title: {
        fontFamily: 'revert',
        fontWeight: 'bold',
        color: 'white',
        paddingTop: 10
    },
    appBar: {
        backgroundColor: red[900]
    },
    appBarButton: {
        backgroundColor: 'white',
        color: red[900],
        fontFamily: 'revert'
    }
}