import React from 'react';
import {
    Typography,
    Box,
    Button,
    IconButton,
    Card,
    Dialog,
    DialogActions,
    DialogContent
} from '@material-ui/core';
import { Close } from '@material-ui/icons';
import { red, green, blue } from '@material-ui/core/colors';

const imghist = require('../Services/ImageHistograms.js');
const Jimp = window.Jimp;


const InfoOnChannel = (props) => {
    return (<Card style={{ ...props.style }}>
        <Typography>min: {props.histogramObj.min}</Typography>
        <Typography>max: {props.histogramObj.max.toFixed(2)}</Typography>
        <Typography>moy: {props.histogramObj.moy.toFixed(2)}</Typography>
        <Typography>med: {props.histogramObj.med.toFixed(2)}</Typography>
        <Typography>mod: {props.histogramObj.mod}</Typography>
    </Card>);
}

const HistoSVG = (props) => {
    return (<Box style={{ backgroundColor: 'lightgray', borderStyle: 'solid' }}>
        <span dangerouslySetInnerHTML={{ __html: props.svgstring }} />
    </Box>);
}

export default function HistogramDialog(props) {

    let defaultHistoObj = {
        svgstring: '',
        min: 0,
        max: 0,
        moy: 0,
        med: 0,
        mod: 0
    };

    const [showHistogram, setShowHistogram] = React.useState(false);
    const [histRedObject, sethistRedObject] = React.useState(defaultHistoObj);
    const [histGreenObject, sethistGreenObject] = React.useState(defaultHistoObj);
    const [histBlueObject, sethistBlueObject] = React.useState(defaultHistoObj);

    React.useEffect(() => {
        async function computeHisto() {
            let colorDepth = props.color_depth;

            try {
                let photo = await Jimp.read(await props.photo.arrayBuffer());
                sethistRedObject(imghist.histogramRGB(imghist.colorChannels.Red, photo, parseInt(colorDepth)));
                sethistGreenObject(imghist.histogramRGB(imghist.colorChannels.Green, photo, parseInt(colorDepth)));
                sethistBlueObject(imghist.histogramRGB(imghist.colorChannels.Blue, photo, parseInt(colorDepth)));
            }
            catch {
            }
        }
        computeHisto();
    }, []);

    const handleOpenHisto = () => {
        setShowHistogram(true);
    };

    const handleCloseHistogram = () => {
        setShowHistogram(false);
    };

    return (
        <div>
        <Button style={styles.button} size='large' variant='contained' onClick={handleOpenHisto}>
            Histogram
        </Button>
        <Dialog open={showHistogram} onClose={handleCloseHistogram}>
            <DialogContent>
                <HistoSVG svgstring={histRedObject.svgstring} />
                <HistoSVG svgstring={histGreenObject.svgstring} />
                <HistoSVG svgstring={histBlueObject.svgstring} />                
            </DialogContent>
            <DialogActions>
                <InfoOnChannel histogramObj={histRedObject} style={{ backgroundColor: red[700], marginRight: 25 }} />
                <InfoOnChannel histogramObj={histGreenObject} style={{ backgroundColor: green[700], marginRight: 25 }} />
                <InfoOnChannel histogramObj={histBlueObject} style={{ backgroundColor: blue[700], marginRight: 50 }} />

                <IconButton onClick={handleCloseHistogram} style={styles.button}>
                    <Close />
                </IconButton>
            </DialogActions>
        </Dialog>
        </div>);
}

const styles = {
    button: {
        color: 'white',
        backgroundColor: red[900],
        fontFamily: 'revert',
        marginTop: 50
    }
}