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
    Dialog,
    DialogActions,
    DialogTitle
} from '@material-ui/core';
import { red } from '@material-ui/core/colors';

import RemoteStudy from '../Services/RemoteStudy';

const concentrationUnitPattern = /^[a-zA-Z]{4}$/i
const concentrationPattern = /^\s*[-+]?(\d+\.?\d*|\d*\.?\d+)([eE][-+]?\d+)?\s*$/g;

class InitStudyScreen extends React.Component {
    constructor(props) {
        super(props);
        this.currentStudy = props.location.studyObject ?? new RemoteStudy('', '', 'pgml',this.props.device.Name);
        this.state = {
            studyName : this.currentStudy.name,
            concentrationUnit : this.currentStudy.unit,
            concentrationList : this.getConcentrationListString(this.currentStudy.concentrationList),
            isConcentrationUnitValid: true,
            isConcentrationListValid: true,
            openQuitStudyDialog: false
        };        
    }

    checkInputsValidity() {
        let validInput = this.state.studyName.length &&
            this.state.isConcentrationListValid &&
            this.state.concentrationList.length &&
            this.state.isConcentrationUnitValid
        return validInput;
    }

    updateConcentration(listObject) {
        let list = this.state.concentrationList.split('\n');
        let listArray = Object.keys(listObject);
        for (let i = 0; listArray[i]; i++) {
            if (parseFloat(listArray[i]) !== parseFloat(list[i])) {
                delete Object.assign(listObject, { [parseFloat(list[i])]: listObject[listArray[i]] })[listArray[i]];
            } else {
                delete Object.assign(listObject, { [listArray[i]]: listObject[listArray[i]] })[listArray[i]];
            }
        }
        return listObject;
    }

    getConcentrationListString(concentrationMap) {
        let list = [...concentrationMap.keys()].join('\n');
        return list;
    }

    handleStudyNameInput = (event) => {
        this.setState({ studyName: event.target.value });
        this.currentStudy.name = event.target.value;
    }

    handleConcentrationUnitInput = (event) => {
        this.setState({ 
            concentrationUnit: event.target.value,
            isConcentrationUnitValid: event.target.value.match(concentrationUnitPattern)
        });
        this.currentStudy.unit = event.target.value;
    }

    handleConcentrationListInput = (event) => {
        let concentrationListInput = event.target.value; 
        let isValidInput = concentrationListInput.split("\n").reduce((acc, line) => acc && (line.length === 0 || line.match(concentrationPattern)), true);
        this.setState({ 
            concentrationList: concentrationListInput,
            isConcentrationListValid : isValidInput
        });
        if (isValidInput) {
            this.currentStudy.setConcentrationList(concentrationListInput);
        }
    }

    lookForExistingStudy() {
        if (this.currentStudy) {
            return this.currentStudy;
        }
        return new RemoteStudy(this.state.studyName, this.state.concentrationList, this.state.concentrationUnit,this.props.device.Name);
    }

    updateList() {
        let listObject = this.currentStudy.concentrationList;

        let oldListArray = Object.keys(listObject);
        let newListArray = this.state.concentrationList.split('\n');

        for (let i = 0; newListArray[i]; i++) {
            newListArray[i] = newListArray[i] + this.state.concentrationUnit;
        }

        let newObject = {};

        for (let i = 0; oldListArray[i]; i++) {
            if (parseFloat(newListArray[i]) !== parseFloat(oldListArray[i])) {
                let newPair = { [newListArray[i]]: listObject[oldListArray[i]] };
                newObject = { ...newObject, ...newPair }
            } else {
                newObject = { ...newObject, [oldListArray[i]]: listObject[oldListArray[i]] }
            }
        }

        return newObject;
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
            <AppBar position='sticky' style={styles.appBar} >
                <Grid container alignItems='center'>
                    <Grid item xs={2}>
                        <Button
                            size='large'
                            variant='contained'
                            style={styles.appBarButton}
                            onClick={this.handleOpenQuitStudyDialog}
                        >
                            Back
                        </Button>
                    </Grid>
                    <Grid item xs={8}>
                        <Typography style={styles.title} variant="h4">
                            Start Measures
                        </Typography>
                        <Typography variant='subtitle2' gutterBottom>
                            {`Connected to ${this.props.device.Name}`}
                        </Typography>
                    </Grid>
                    <Grid item xs={2}>
                        <Button
                        size='large'
                        style={{...styles.appBarButton, backgroundColor: this.checkInputsValidity() ? 'white' : 'grey'}}
                        component={Link}
                        disabled={!this.checkInputsValidity()}
                        to={{
                            pathname: '/calibration-study',
                            caller: '/init-study',
                            studyObject: this.lookForExistingStudy(),
                            concentrationName: ''
                        }}
                    >
                        Calibration
                    </Button>
                    </Grid>
                </Grid>
            </AppBar>
        );
    }

    render() {
        return (
            <Container>
                {this.renderAppBar()}
                {this.renderQuitStudyDialog()}
                <Box margin={5}>
                </Box>
                <Box marginBottom={2} >
                    <TextField
                        id="input-study-name"
                        label="Study name"
                        value={this.state.studyName}
                        onChange={this.handleStudyNameInput}
                    />
                </Box>
                <Box margin={2} >
                    <TextField
                        id="input-concentration-unit"
                        label='Unit'
                        placeholder='Ex: ngml'
                        value={this.state.concentrationUnit}
                        onChange={this.handleConcentrationUnitInput}
                        error={!this.state.isConcentrationUnitValid}
                        helperText={this.state.isConcentrationUnitValid ? "" : "Unit must be 4 letters"}
                    />
                </Box>
                <Box margin={2}>
                    <TextField
                        id="input-concentration"
                        label="Concentration(s)"
                        multiline
                        rows={9}
                        variant="outlined"
                        value={this.state.concentrationList}
                        onChange={this.handleConcentrationListInput}
                        error={!this.state.isConcentrationListValid}
                        helperText={this.state.isConcentrationListValid ? "" : "Concentration must be a number, ie: \"0.25\",\"1e-12\" "}
                    />
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
    appBarButton: {
        backgroundColor: 'white',
        fontFamily: 'revert'
    }
}

export default InitStudyScreen;