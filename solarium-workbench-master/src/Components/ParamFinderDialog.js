import React from 'react';
import {
    Grid,
    TextField,
    Button,
    IconButton,
    CircularProgress,

    Switch,
    Dialog,
    DialogTitle,
    DialogActions,
    DialogContent
} from '@material-ui/core';
import { Close } from '@material-ui/icons';

import { red, green, blue } from '@material-ui/core/colors';
import K7Chooser from './K7Chooser';
const stringify = require("json-stringify-pretty-compact");


const CROPPING_PARAMS = require('../Assets/cropping-params.json');
const DEFAULT_FINDER_PARAMS = require('../Assets/default-finder-params.json');

export default function ParamFinderDialog(props) {

    let defaultParams = props.device?.isSeries3 ? DEFAULT_FINDER_PARAMS.series3 : DEFAULT_FINDER_PARAMS.series2;

    console.log("defaultParams ? ", defaultParams);

    const [showDialog, setShowDialog] = React.useState(false);
    const [finderName, setFinderName] = React.useState('');
    const [finderRunning, setFinderRunning] = React.useState(false);
    const [cassetteName, setCassetteName] = React.useState("K701BT_SE");
    const [finderParams, setFinderParams] = React.useState(stringify(defaultParams, null, 2));
    const [isValidInputParams, setIsValidInputParams] = React.useState(true);
    const [errorMessage, setErrorMessage] = React.useState("");

    const launchParamFinder = async () => {
        let paramsObj;
        try {
            paramsObj = JSON.parse(finderParams);
            setErrorMessage("");
            setIsValidInputParams(true);
        }
        catch (err) {
            setErrorMessage(`Invalid input : ${err}`);
            setIsValidInputParams(false);
            return;
        }

        setFinderRunning(true);
        await props.device.launchParamFinder(finderName, paramsObj, cassetteName);
        setFinderRunning(false);
    };

    return (<div>
        <Button variant='contained' 
            size='large'
            style={props.style} onClick={() => setShowDialog(true)}>
            Acquisition Param Finder
        </Button>
        <Dialog open={showDialog} 
            disableBackdropClick={true}
            onClose={() => setShowDialog(false)}>
            <DialogTitle>Acquisition Param Looper</DialogTitle>
            <DialogContent>
                <Grid container>
                    <Grid item xs={3}>Name</Grid>
                    <Grid item xs={9}>
                        <TextField
                            fullWidth={true}
                            multiline={false}
                            variant='outlined'
                            size='small'
                            type='text'
                            value={finderName}
                            onChange={(event) => setFinderName(event.target.value)}
                        />
                    </Grid>
                    <Grid item xs={3}>Cassette</Grid>
                    <Grid item xs={9}>
                        <K7Chooser onSelectionChanged={setCassetteName} />
                    </Grid>
                    <Grid item xs={3}>Parameters</Grid>
                    <Grid item xs={9}>
                        <TextField
                            fullWidth={true}
                            multiline={true}
                            variant='outlined'
                            size='medium'
                            type='text'
                            rows={7}
                            value={finderParams}
                            onChange={(event) => setFinderParams(event.target.value)}
                            error={!isValidInputParams}
                            helperText={isValidInputParams ? '' : errorMessage }
                        />
                    </Grid>
                </Grid>
            </DialogContent>
            <DialogActions>
                {finderRunning && <CircularProgress size={24} style={styles.loadingIcon} />}
            
                <Button variant='contained' 
                    disabled={finderRunning} 
                    style={finderRunning ? styles.disabledButton : styles.button} 
                    onClick={launchParamFinder}>
                    Launch
                </Button>
                <IconButton  disabled={finderRunning}
                    style={finderRunning ? styles.disabledButton : styles.button}
                    onClick={() => setShowDialog(false)}>
                    <Close />
                </IconButton>
            </DialogActions>
        </Dialog>
    </div>);
}

const styles = {
    title: {
        fontFamily: 'revert',
        fontWeight: 'bold',
        color: 'white'
    },
    iconButton: {
        backgroundColor: red[900],
        color: 'white',
        margin: 10
    },
    loadingIcon: {
        color: red[900],
        padding: 5
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
    disabledButton: {
        color: 'white',
        backgroundColor: '#9E9E9E',
        fontFamily: 'revert',
        marginTop: 50
    }
}