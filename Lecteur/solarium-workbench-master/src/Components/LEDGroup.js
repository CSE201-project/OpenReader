import React, {useState} from 'react';
import {
    Container,
    Button, 
} from '@material-ui/core';
import { red } from '@material-ui/core/colors';

import LED from './LED';

const LEDGroup = ({type, device, style}) => {
    const [enabled, setEnable] = useState(device.LEDs[type.toLowerCase()].enabled);
    const [loading, setLoading] = useState(false);
    const [intensity, setIntensity] = useState(device.LEDs[type.toLowerCase()].intensity);


    const enableGroup = async (enable) => {
        let success = await device.ledsEnableCommand(
            type.toLowerCase(),
            enable,
            intensity
        );
        if (success) {
            setEnable(enable);
        }
    };

    const setLEDIntensity = async (value) => {
        if (enabled) {
            setLoading(true);
            let success = await device.ledsSetIntensityCommand(
                type.toLowerCase(),
                value,
            );
            if (success) {
                setIntensity(value);
            }
            setLoading(false);
            return success;
        }
        else {             
            setIntensity(value);
            return true; 
        }
    };

    return (
        <Container>
            <Button
                onClick={async () => {await enableGroup(!enabled)}}
                variant='contained'
                style={{
                    marginTop: 5,
                    backgroundColor: enabled ? red[900] : 'grey',
                    color: 'white',
                    fontFamily: 'revert'
                }}
            >
                {`Leds ${type}`}
            </Button>
            <LED
                initialIntensity={intensity}
                onValueChanged={(value) => setLEDIntensity(value)}
                disabled={loading}
            />
        </Container>
    );
};

export default LEDGroup;
