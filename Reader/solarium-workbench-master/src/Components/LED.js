import React, { useState } from 'react';
import {
    Container,
    Typography,
    Slider,
    Grid,
    IconButton
} from '@material-ui/core';
import { Add, Remove } from '@material-ui/icons';
import { red } from '@material-ui/core/colors';

const LED = ({ initialIntensity, disabled, onValueChanged }) => {
    const [intensity, setIntensity] = useState(initialIntensity || 200);

    const setLEDIntensity = (value) => {
        if (value < 0 || value > 255) {
            return;
        }
        if (onValueChanged && onValueChanged(value)) {
            setIntensity(value);
        }
    };

    const setLEDIntensityWithSlider = (event, value) => {
        if (value < 0 || value > 255) {
            return;
        }
        if (onValueChanged && onValueChanged(value)) {
            setIntensity(value);
        }
    };

    return (
        <Container>
            <Grid container spacing={0} >
                <Grid item xs={2}>
                    <IconButton onClick={() => setLEDIntensity(intensity - 1)} style={styles.removeButton}>
                        <Remove />
                    </IconButton>
                </Grid>
                <Grid item xs={8}>
                    <Slider
                        value={intensity}
                        onChangeCommitted={setLEDIntensityWithSlider}
                        min={0}
                        max={255}
                        step={1}
                        style={{color: disabled ? 'grey' : red[900]}}
                        disabled={disabled}
                    />
                </Grid>
                <Grid item xs={2}>
                    <IconButton onClick={() => setLEDIntensity(intensity + 1)} style={styles.addButton}>
                        <Add />
                    </IconButton>
                </Grid>
            </Grid>
            <Typography style={styles.text} variant='subtitle2' gutterBottom>
                {intensity}
            </Typography>
        </Container>
    );
};

const styles = {
    slider: {
        color: red[900],
    },
    text: {
        marginTop: -20,
        marginBottom: 20,
        fontFamily: 'revert',
        fontSize: 16
    },
    addButton: {
        marginTop: -10,
        marginLeft: 5,
        color: red[900]
    },
    removeButton: {
        marginTop: -10,
        marginLeft: -5,
        color: red[900]
    }
}

export default LED;
