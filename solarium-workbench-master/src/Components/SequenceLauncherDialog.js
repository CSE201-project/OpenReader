import React, { useRef } from 'react';
import {
    Grid,
    TextField,
    Button,
    IconButton,
    CircularProgress,
    Dialog,
    DialogTitle,
    DialogActions,
    DialogContent
} from '@material-ui/core';
import { Close } from '@material-ui/icons';
import { red, green, blue } from '@material-ui/core/colors';

function computeDefaultName() {
    let d = new Date().toISOString().split('T');
    let name = "sequence-" + d[0].replaceAll('-','') + '-' + d[1].replace(/(\d{2}):(\d{2}):(\d{2}).*/, "$1$2$3");
    return name;        
}

export default function SequenceLauncherDialog(props) {

    const [showDialog, setShowDialog] = React.useState(false);
    const [sequenceIntervalSeconds, setSequenceIntervalSeconds] = React.useState(10);
    const [sequenceCount, setSequenceCount] = React.useState(10);
    const [sequenceInitialCount, setSequenceInitialCount] = React.useState(10);
    const [sequenceRunning, setSequenceRunning] = React.useState(false);
    const [sequenceName, setSequenceName] = React.useState(computeDefaultName());

    const handleOpenDialog = () => {
        setSequenceName(computeDefaultName());
        setShowDialog(true);
    };

    const handleCloseDialog = () => {
        if (!sequenceRunning) {
            setShowDialog(false);
        }
    };

    const handleInitialCount = (event) => {
        setSequenceCount(event.target.value);        
        setSequenceInitialCount(event.target.value);
    }

    const sequenceTick = () => {
        if (sequenceCount === 1) {
            setSequenceRunning(false);
            setSequenceCount(sequenceInitialCount);
        } else {
            setSequenceCount(sequenceCount - 1);
        }
    }

    React.useEffect(() => {
        if (sequenceRunning) {
            
            let t = setInterval(() => { sequenceTick(); }, sequenceIntervalSeconds * 1000);
            return () => clearInterval(t);
        }
    },  [sequenceRunning, sequenceCount]);

    const launchSequence = () => {
        setSequenceRunning(true);
        if (props.onLaunchSequence) {
            props.onLaunchSequence(sequenceName, sequenceIntervalSeconds, sequenceCount);
        }
    }

    return (<div>
        <Button variant='contained' style={styles.burstButton} onClick={() => handleOpenDialog()} >
            Launch Sequence
        </Button>
        <Dialog open={showDialog} onClose={() => handleCloseDialog()} >
            <DialogTitle>Sequence</DialogTitle>
            <DialogContent>
                <Grid container>
                    <Grid item xs={12}>
                        <TextField
                            margin='normal'
                            fullWidth={true}
                            value={sequenceName}
                            onChange={(event) => {setSequenceName(event.target.value)}}
                            variant='outlined'
                            label='Name'
                            type='text'
                            disabled={sequenceRunning}
                        />
                    </Grid>
                    <Grid item xs={6}>
                        <TextField
                            margin='normal'
                            fullWidth={true}
                            value={sequenceIntervalSeconds}
                            onChange={(event) => {setSequenceIntervalSeconds(event.target.value)}}
                            variant='outlined'
                            label='Interval (Seconds)'
                            type='number'
                            disabled={sequenceRunning}
                        />
                    </Grid>
                    <Grid item xs={6}>
                        <TextField
                            margin='normal'
                            fullWidth={true}
                            value={sequenceCount}
                            onChange={handleInitialCount}
                            variant='outlined'
                            label='Pictures count'
                            type='number'
                            disabled={sequenceRunning}
                        />
                    </Grid>
                </Grid>
            </DialogContent>
            <DialogActions>
                <Button variant='contained' style={sequenceRunning ? styles.disabledButton : styles.button} onClick={launchSequence} disabled={sequenceRunning}>
                    Start Sequence
                </Button>
                <IconButton onClick={() => handleCloseDialog()} style={sequenceRunning ? styles.disabledButton : styles.button}>
                    {!sequenceRunning && <Close />}
                    {sequenceRunning && <CircularProgress size={24} style={styles.loadingIcon} />}
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
        color: 'white'
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
};