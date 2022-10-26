import React from 'react';
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

export default function SequenceToStudies(props) {

    const [showDialog, setShowDialog] = React.useState(false);
    const [sourceFoldersList, setSourcesFoldersList] = React.useState('');
    const [outputFolder, setOutputFolder] = React.useState('');
    const [conversionRunning, setRunning] = React.useState(false);

    const selectFolders = () => {
        console.log('calling shwoopendialog');
        let d = props.dialog.showOpenDialogSync({
            properties:['openDirectory', 'multiSelections']
        });
        if (d) {
            setSourcesFoldersList(d.join('\n'));
        }
    };

    const selectOutputFolder = () => {
        console.log('calling shwoopendialog');
        let d = props.dialog.showOpenDialogSync({
            properties:['openDirectory']
        });
        if (d) {
            setOutputFolder(d);
        }
    };

    const convert = async () => {
        setRunning(true);
        try {
            for (const path of sourceFoldersList.split('\n')) {
                const files = props.fs.readdirSync(path);
                for (const file of files) {
                    let destDir = outputFolder + "/" + file.replace(/.*(\d{3})\.png/, '$1');
                    props.fs.mkdirSync(destDir, { recursive: true });
                    props.fs.copyFileSync(`${path}/${file}`, `${destDir}/${file}`)
                }
            }
        } catch (err) {
            console.error(err);
        }
        alert('Conversion done');
        setRunning(false);
    };

    return (<div>
        <Button variant='contained' style={styles.burstButton} onClick={() => setShowDialog(true)} >
            Sequences to Studies
        </Button>
        <Dialog open={showDialog} onClose={() => setShowDialog(false)} >
            <DialogTitle>Sequences Folders</DialogTitle>
            <DialogContent>
                <Grid container>
                    <Grid item xs={9}>
                        <TextField
                            margin='normal'
                            fullWidth={true}
                            multiline={true}
                            value={sourceFoldersList}
                            variant='outlined'
                            label='Input Folders'
                            type='text'
                            readonly={true}
                        />
                    </Grid>
                    <Grid item xs={3}>
                        <Button onClick={() => selectFolders()} style={styles.searchButton}>
                            Search Folders
                        </Button>
                    </Grid>
                    <Grid item xs={9}>
                        <TextField
                            margin='normal'
                            fullWidth={true}
                            variant='outlined'
                            label='Output Folder'
                            type='text'
                            value={outputFolder}
                        />
                    </Grid>
                    <Grid item xs={3}>
                        <Button onClick={() => selectOutputFolder()} style={styles.searchButton}>
                            Select output folder
                        </Button>
                    </Grid>
                </Grid>
            </DialogContent>
            <DialogActions>
                <Button variant='contained' style={styles.button} disabled={conversionRunning} onClick={() => convert()}>
                    Convert
                </Button>
                <IconButton onClick={() => setShowDialog(false)} style={styles.button}>
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
        marginLeft: 2,
        color: 'white',
        backgroundColor: red[900]
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