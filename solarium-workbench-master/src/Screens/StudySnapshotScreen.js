import React from 'react';
import { Link } from 'react-router-dom'
import {
    Container,
    Typography,
    Button,
    CircularProgress,
    Card,
    CardContent,
    CardMedia,
    Grid,
    AppBar,
    IconButton,
    Box,
    Dialog,
    DialogTitle,
    DialogActions,
    Divider,
    Fade
} from '@material-ui/core';
import { red } from '@material-ui/core/colors';
import { PhotoCamera, Tune, Delete } from '@material-ui/icons';

import '../react/App.css';
import { blobToBase64 } from '../Services/Converter';

export default class StudySnapshotScreen extends React.Component {
    constructor(props) {
        super(props);
        
        this.currentStudy = this.props.location.studyObject;

        this.state = {
            concentrationName: props.location.concentrationName,
            isLoading: false,
            image: '',
            canDeletePicture: false,
            showDeleteSnapshotDialog: false,
            showEndStudyButton: this.currentStudy.isLastConcentration(props.location.concentrationName),
            openQuitStudyDialog: false,
            shouldFade: true,
            currentSnapshotName: '',
            currentSnapshotNumber: this.currentStudy.getSnapshotNumber(props.location.concentrationName),
            concentrationResults: null
        };
    }

    openDeleteSnapshotDialog() {
        this.setState({ showDeleteSnapshotDialog: true });
    }

    closeDeleteSnapshotDialog = () => {
        this.setState({ showDeleteSnapshotDialog: false });
    }

    displayFadeEffect() {
        this.setState({ shouldFade: false });
        setTimeout(() => {
            this.setState({ shouldFade: true });
        }, 1000);
    }

    async handleSnapshot() {
        this.setState({
            isLoading: true,
            image: ''
        });
        try {

            let currentSnapshot = this.currentStudy.addSnapshot(this.state.concentrationName);
            let snapshotPayload = this.currentStudy.getSnapshotPayload(currentSnapshot);

            let success = false;
            let [ pictureData, concentrationResults ] = await this.props.device.takeStudySnapshot(snapshotPayload);
            if (pictureData) {
                this.setState({ image: await blobToBase64(pictureData) });
                this.currentSnapshot = currentSnapshot;
                this.currentSnapshot.concentrationResults = snapshotPayload.compute_concentration ? concentrationResults : "not computed";                success = true;
            } else {
                this.currentStudy.deleteSnapshot(currentSnapshot);
                this.currentSnapshot = null;
            }
            this.setState({ 
                canDeletePicture: success,
                currentSnapshotName: this.currentSnapshot?.fileName || 'error',
                currentSnapshotNumber: this.currentSnapshot?.photoNumber || this.state.currentSnapshotNumber,
                concentrationResults: this.currentSnapshot?.concentrationResults || ''
            });
        } catch (err) {
            console.log(err);
        }

        this.setState({ isLoading: false });
    }

    handleNextButton() {
        this.displayFadeEffect();
        
        let nextConcentration = null;
                
        let list = [...this.currentStudy.concentrationList.keys()];
        let nextIndex = list.indexOf(this.state.concentrationName) + 1;
        
        if (nextIndex < list.length) {
            nextConcentration = list[nextIndex];
        }

        this.setState({
            showEndStudyButton: (nextIndex+1 === list.length),
            concentrationName: nextConcentration,
            currentSnapshotName : '',
            currentSnapshotNumber : this.currentStudy.getSnapshotNumber(nextConcentration)
        });
    }

    async deleteSnapshot() {
        this.setState({
            showDeleteSnapshotDialog : false,
            canDeletePicture : false,
            isLoading: true,
            image: ''
        });
        try {

            let success = await this.props.device.deleteStudySnapshot(this.currentStudy.name, this.currentStudy.date, this.currentSnapshot.fileName);
            let snapshotNumber = this.state.currentSnapshotNumber;
            let snapshotName = this.state.currentSnapshotName;
            if (success) {
                this.currentStudy.deleteSnapshot(this.currentSnapshot);
                snapshotNumber = this.currentStudy.getSnapshotNumber(this.state.concentrationName);
                snapshotName = 'deleted';
            }
            
            this.setState({ 
                canDeletePicture: !success,
                currentSnapshotName : snapshotName,
                currentSnapshotNumber : snapshotNumber,
                concentrationResults : null
            });
        } catch (err) {
            console.log(err);
        }

        this.setState({ isLoading: false });
    }

    renderDeleteSnapshotDialog() {
        return (
            <Dialog open={this.state.showDeleteSnapshotDialog} onClose={this.closeDeleteSnapshotDialog}>
                <DialogTitle>Delete current snapshot ?</DialogTitle>
                <DialogActions>
                    <Button onClick={async () => { this.closeDeleteSnapshotDialog(); } }>
                        Cancel
                    </Button>
                    <Button onClick={() => this.deleteSnapshot()}>
                        Confirm
                    </Button>
                </DialogActions>
            </Dialog>
        );
    }

    handleOpenQuitStudyDialog = () => {
        this.setState({openQuitStudyDialog:true});
    }

    handleCloseQuitStudyDialog = () => {
        this.setState({openQuitStudyDialog:false});
    }

    renderQuitStudyDialog = () => {
        return (
            <Dialog open={this.state.openQuitStudyDialog} onClose={this.handleCloseQuitStudyDialog}>
                <DialogTitle>Close current study ?</DialogTitle>
                <DialogActions>
                    <Button onClick={this.handleCloseQuitStudyDialog}>
                        Cancel
                    </Button>
                    <Button 
                        component={Link}
                        to="/"
                    >
                        Close Study
                    </Button>
                </DialogActions>
            </Dialog>
        );
    }

    renderAppBar() {
        return (
            <AppBar position='sticky' style={styles.appBar}>
                <Grid container alignItems='center'>
                    <Grid item xs={2}>
                        <Button
                            size='large'
                            variant='contained'
                            style={styles.appBarButton}
                            component={Link}
                            to={{
                                pathname: '/concentration-list',
                                studyObject: this.currentStudy,
                            }}
                        >
                            Back
                        </Button>
                    </Grid>
                    <Grid item xs={2} >
                        <IconButton
                            style={styles.paramsButton}
                            component={Link}
                            to={{
                                pathname: '/calibration-study',
                                caller: '/study-snapshot',
                                studyObject: this.currentStudy,
                                concentrationName: this.state.concentrationName
                            }}
                        >
                            <Tune />
                        </IconButton>
                    </Grid>
                    <Grid item xs={4}>
                        <Typography style={styles.title} variant="h4">
                            {this.currentStudy.name}
                        </Typography>
                        <Typography variant='subtitle2' gutterBottom>
                            {`Connected to ${this.props.device.Name}`}
                        </Typography>
                    </Grid>
                    {!this.state.showEndStudyButton &&
                        <Grid item xs={4}>
                            <Button
                                size='small'
                                variant='contained'
                                style={styles.appBarButton}
                                onClick={() => this.handleNextButton()}
                            >
                                Next Concentration
                            </Button>
                        </Grid>
                    }
                    {this.state.showEndStudyButton &&
                        <Grid item xs={4}>
                            <Button
                                size='large'
                                variant='contained'
                                style={styles.appBarButton}
                                onClick={this.handleOpenQuitStudyDialog}
                            >
                                End Study
                            </Button>
                        </Grid>
                    }
                </Grid>
            </AppBar>
        );
    }

    render() {
        return (
            <Container>
                {this.renderAppBar()}                
                {this.renderQuitStudyDialog()}
                <Card>
                    <CardContent>
                        <Grid container alignItems='center'>
                            <Grid item xs={6}>
                                <Typography style={{ ...styles.nameText, marginBottom: 20 }} variant="subtitle2" gutterBottom>
                                    Concentration
                                </Typography>
                                {/* <Fade in={this.state.shouldFade} > */}
                                <Typography style={{ fontSize: 40, fontWeight: 'bold', color: red[900] }} variant="subtitle2" gutterBottom>
                                    {this.state.concentrationName}
                                </Typography>
                                {/* </Fade> */}
                            </Grid>
                            <Divider style={{ marginRight: 30 }} flexItem orientation='vertical' />
                            <Grid item xs={5}>
                                <Typography style={styles.text} variant="subtitle2" gutterBottom>
                                    Destination directory
                                </Typography>
                                <Typography style={styles.path} variant="subtitle2" gutterBottom>
                                    /mnt/update/studies
                                </Typography>
                                <Divider style={{ margin: 20 }} />
                                <Typography style={styles.text} variant="subtitle2" gutterBottom>
                                    Current Snapshot : {this.state.currentSnapshotName}
                                </Typography>
                                <Typography style={{ ...styles.path, fontSize: 20 }} variant="subtitle2" gutterBottom>
                                    {this.state.currentSnapshotNumber} picture(s)<br/>x<br/>{this.currentStudy.acquisitionParams.length} measure params
                                </Typography>
                            </Grid>

                        </Grid>
                        <IconButton
                            onClick={() => this.handleSnapshot()}
                            style={{
                                backgroundColor: red[900],
                                color: this.state.isLoading ? red[900] : 'white',
                                marginRight: 2
                            }}
                            disabled={this.state.isLoading}
                        >
                            <PhotoCamera />
                        </IconButton>
                        {this.state.isLoading && <CircularProgress size={24} style={styles.loadingIcon} />}
                        <IconButton
                            onClick={() => this.openDeleteSnapshotDialog()}
                            style={{
                                backgroundColor: this.state.canDeletePicture ? red[900] : 'gray',
                                color: 'white',
                                marginLeft: 2
                            }}
                            disabled={!this.state.canDeletePicture}
                        >
                            <Delete />
                        </IconButton>
                        {this.renderDeleteSnapshotDialog()}
                    </CardContent>
                </Card>
                <Box width="100%">
                    <Card style={{display:'flex'}}>
                        <CardMedia component="img" src={this.state.image} style={styles.image}/>
                        {this.currentSnapshot && <CardContent style={{display:'flex', flexDirection:'column'}}>
                            <Typography style={{ ...styles.path, fontSize: 20 }} variant="subtitle2" gutterBottom>
                                    Concentration Predictions (ProxNoise 05/2021)
                                </Typography>
                            {this.state.concentrationResults?.split(',').map((item, key) => 
                                <Typography key={key} style={{ ...styles.path, fontSize: 15 }} variant="subtitle2" >
                                    {item}
                                </Typography>) 
                            }
                        </CardContent>}
                    </Card>
                </Box>
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
    paramsButton: {
        backgroundColor: red[900],
        color: 'white'
    },
    appBarButton: {
        backgroundColor: 'white',
        fontFamily: 'revert'
    },
    loadingIcon: {
        color: 'white',
        position: 'absolute',
        marginTop: 11,
        marginLeft: -38,

    },
    listItem: {
        backgroundColor: red[200]
    },
    button: {
        backgroundColor: red[900], // #B40513
        color: 'white'
    },
    text: {
        fontFamily: 'revert'
    },
    nameText: {
        fontFamily: 'revert',
        fontWeight: 'bold',
        fontSize: 16
    },
    path: {
        fontFamily: 'revert',
        color: red[900]
    },
    image: {
        margin: 10,
        width: '75%'
    }
}
