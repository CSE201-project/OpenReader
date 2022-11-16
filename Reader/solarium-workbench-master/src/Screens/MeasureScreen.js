import React from 'react';
import { Link } from 'react-router-dom';
import {
    Container,
    Typography,
    Box,
    TextField,
    Button,
    AppBar,
    Grid,
    Divider,
    CircularProgress,
    Dialog,
    DialogActions,
    DialogTitle,
    DialogContent,
    IconButton,
    Card,
    CardMedia
} from '@material-ui/core';
import { Close } from '@material-ui/icons';
import { red } from '@material-ui/core/colors';

import Profile from '../Components/Profile';
import { blobToBase64 } from '../Services/Converter';

const dialog = window.dialog;

const snapshotPayload = require('../Assets/snapshot-payload.json');
const VALID_INPUT = /^[a-zA-Z0-9-_=()]+$/i;

class MeasureScreen extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            isLoading: false,
            patientID: 'patient123',
            stripeID: 'covid',
            showResult: false,
            showProfile: false,
            measureID: '',
            measureDate: '',
            measureStripe: '',
            measureResult: '',
            measureProfile: '',
            isValidPatient: false,
            isValidStripe: false,
            openTakePictureDialog: false,
            image: ''
        };
    }

    componentDidMount() {
        this.checkTextExist();
    }

    openTakePictureDialog = () => {
        this.setState({ openTakePictureDialog: true });
    }

    closeTakePictureDialog = () => {
        this.setState({ openTakePictureDialog: false });
    }

    checkTextExist() {
        if (this.state.patientID[0])
            this.setState({ isValidPatient: true });
        if (this.state.stripeID[0])
            this.setState({ isValidStripe: true });
    }

    handlePatientIDInput = (event) => {
        let isValidInput = event.target.value.match(VALID_INPUT);
        this.setState({
            patientID: event.target.value,
            isValidPatient: isValidInput
        });
    }

    handleStripeIDInput = (event) => {
        let isValidInput = event.target.value.match(VALID_INPUT);
        this.setState({
            stripeID: event.target.value,
            isValidStripe: isValidInput
        });
    }

    getCurrentDate(analysis_date) {
        let date = new Date(analysis_date);
        let hour = String(date.getHours()).padStart(2, '0');
        let minute = String(date.getMinutes()).padStart(2, '0');
        let second = String(date.getSeconds()).padStart(2, '0');
        let day = String(date.getDate()).padStart(2, '0');
        let month = String(date.getMonth() + 1).padStart(2, '0');
        let year = date.getFullYear();

        date = `${day}/${month}/${year}, ${hour}:${minute}:${second}`;
        return date;
    }

    async displayResult(pictureType) {
        let params = {};
        params.patient_id = this.state.patientID;
        params.strip_type = this.state.stripeID;
        this.setState({
            isLoading: true,
            openTakePictureDialog: false
        });
        let patientResults = await this.props.device.getAnalysisResult(params);
        if (!patientResults) {
            this.setState({ isLoading: false });
            dialog.showErrorBox('Measure Error', '');
            return;
        } else if (patientResults.status !== "SUCCESS") {
            this.setState({ isLoading: false });
            dialog.showErrorBox('Status Error', patientResults.status);
            return;
        }
        try {
            var hasAllProfile = patientResults.profile.hasOwnProperty('red');
            var date = this.getCurrentDate(patientResults.analysis_date);
            var pictureData = await this.props.device.getFile(patientResults.id, pictureType);
            var image = await blobToBase64(pictureData);
        } catch (err) {
            console.log(err);
        }
        this.setState({
            image: image,
            showResult: true,
            showProfile: pictureType === "cropped",
            isLoading: false,
            measureID: patientResults.id,
            measureDate: date,
            measureStripe: patientResults.strip_type,
            measureResult: patientResults.result,
            measureProfile: hasAllProfile ? patientResults.profile.red : patientResults.profile,
        });
    }

    renderAppBar() {
        return (
            <AppBar position='sticky' style={styles.appBar} >
                <Grid container alignItems='center'>
                    <Grid item xs={2}>
                        <Button
                            size='large'
                            variant='contained'
                            style={styles.appBarButton}
                            component={Link}
                            to="/"
                        >
                            Back
                        </Button>
                    </Grid>
                    <Grid item xs={8}>
                        <Typography style={styles.title} variant="h4">
                            Patient Measure
                        </Typography>
                        <Typography variant='subtitle2' gutterBottom>
                            {`Connected to ${this.props.device.Name}`}
                        </Typography>
                    </Grid>
                </Grid>
            </AppBar>
        );
    }

    takePictureDialog() {
        return (
            <Dialog open={this.state.openTakePictureDialog} onClose={this.closeTakePictureDialog}>
                <DialogTitle>Image Type</DialogTitle>
                <DialogContent>
                    <Button
                        variant='contained'
                        onClick={async () => await this.displayResult("full")}
                        style={{ backgroundColor: red[900], color: "white", marginRight: 5 }}
                    >
                        Full
                    </Button>
                    <Button
                        variant='contained'
                        onClick={async () => await this.displayResult("cropped")}
                        style={{ backgroundColor: red[900], color: "white", marginLeft: 5 }}
                    >
                        Cropped
                    </Button>
                </DialogContent>
                <DialogActions>
                    <IconButton onClick={this.closeTakePictureDialog} style={{ color: "black" }}>
                        <Close />
                    </IconButton>
                </DialogActions>
            </Dialog>
        );
    }

    render() {
        return (
            <Container>
                {this.renderAppBar()}
                <Grid container>
                    <Grid item xs={6}>
                        <Box margin={1}>
                            <TextField
                                id="patient-id"
                                label='Patient ID'
                                placeholder='Patient123'
                                value={this.state.patientID}
                                onChange={this.handlePatientIDInput}
                                error={!this.state.isValidPatient}
                            />
                        </Box>
                        <Box marginBottom={2}>
                            <TextField
                                id="stripe-id"
                                label='stripe ID'
                                placeholder='covid'
                                value={this.state.stripeID}
                                onChange={this.handleStripeIDInput}
                                error={!this.state.isValidStripe}
                            />
                        </Box>
                        <Button
                            variant='outlined'
                            disabled={!(this.state.isValidPatient && this.state.isValidStripe)}
                            style={{
                                backgroundColor: red[900],
                                color: this.state.isLoading ? red[900] : 'white',
                                fontFamily: 'revert'
                            }}
                            onClick={this.openTakePictureDialog}
                        >
                            Get Measures
                        </Button>
                        {this.state.isLoading && <CircularProgress size={24} style={styles.loadingIcon} />}
                    </Grid>
                    <Grid item xs={6}>
                        <Card style={styles.image}>
                            <CardMedia component="img" src={this.state.image} />
                        </Card>
                        {this.state.showProfile &&
                            <Card style={styles.profile}>
                                {!this.state.isLoading && <Profile profile={this.state.measureProfile} color="red" />}
                            </Card>
                        }
                    </Grid>
                </Grid>
                {this.takePictureDialog()}
                {this.state.showResult &&
                    <Box>
                        <Divider style={{ margin: 20 }} />
                        <Typography>mesureID: {this.state.measureID}</Typography>
                        <Typography>patientID: {this.state.patientID}</Typography>
                        <Typography>date: {this.state.measureDate}</Typography>
                        <Typography>stripe: {this.state.measureStripe}</Typography>
                        <Typography>res: {this.state.measureResult}</Typography>
                    </Box>
                }
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
        fontFamily: 'revert'
    },
    loadingIcon: {
        color: 'white',
        position: 'absolute',
        marginTop: 5,
        marginLeft: -80,
    },
    image: {
        margin: 10,
        marginTop: 5,
    },
    profile: {
        marginLeft: -50
    }
}

export default MeasureScreen;