import React, {useState} from 'react';
import {Button, Slider} from 'react-native-elements';
import {View, StyleSheet, Text} from 'react-native';
import {Colors} from '../MainTheme';
import LED from './LED';

const LEDGroup = ({type, device, style}) => {
    const [enabled, setEnable] = useState(false);
    const [intensity, setIntensity] = useState(127);

    const enableGroup = async (enable) => {
        let success = await device.ledsEnableCommand(
            type.toLowerCase(),
            enable,
        );
        if (success) {
            setEnable(enable);
        }
    };

    const setLEDIntensity = async (value) => {
        let success = await device.ledsSetIntensityCommand(
            type.toLowerCase(),
            value,
        );
        if (success) {
            setIntensity(value);
        }
        return success;
    };

    return (
        <View style={style}>
            <Button
                title={`LEDs ${type}`}
                onPress={async () => {
                    await enableGroup(!enabled);
                }}
                buttonStyle={
                    enabled ? innerStyle.buttonOn : innerStyle.buttonOff
                }
            />
            <LED
                initialIntensity={intensity}
                onValueChanged={(value) => setLEDIntensity(value)}
                disabled={!enabled}
            />
            {/*<LED*/}
            {/*    initialIntensity={intensity}*/}
            {/*    onValueChanged={(value) => setLEDIntensity(value)}*/}
            {/*    disabled={!enabled}*/}
            {/*/>*/}
        </View>
    );
};

const innerStyle = StyleSheet.create({
    buttonOn: {
        backgroundColor: Colors.red,
    },
    buttonOff: {
        backgroundColor: Colors.grey,
    },
});

export default LEDGroup;
