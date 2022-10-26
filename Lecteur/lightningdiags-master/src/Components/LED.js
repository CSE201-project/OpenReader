import React, {useState} from 'react';
import {View, StyleSheet, Text} from 'react-native';
import {Button, Slider} from 'react-native-elements';

import {Colors} from '../MainTheme';

const LED = ({initialIntensity, disabled, onValueChanged}) => {
    const [intensity, setIntensity] = useState(initialIntensity || 200);

    const setLEDIntensity = (value) => {
        if (value < 0 || value > 255) {
            return;
        }
        if (onValueChanged && onValueChanged(value)) {
            setIntensity(value);
        }
    };

    return (
        <View>
            <Slider
                value={intensity}
                onValueChange={(value) => setLEDIntensity(value)}
                minimumValue={0}
                maximumValue={255}
                step={1}
                disabled={disabled}
                thumbTintColor={disabled ? Colors.grey : Colors.red}
            />
            <View style={innerStyle.row}>
                <Button
                    containerStyle={innerStyle.rowItem}
                    title=" - "
                    disabled={disabled}
                    disabledTitleStyle={{color: Colors.grey}}
                    onPress={() => setLEDIntensity(intensity - 1)}
                />
                <Text style={innerStyle.rowItemLarge}>Value : {intensity}</Text>
                <Button
                    containerStyle={innerStyle.rowItem}
                    title=" + "
                    disabled={disabled}
                    disabledTitleStyle={{color: Colors.grey}}
                    onPress={() => setLEDIntensity(intensity + 1)}
                />
            </View>
        </View>
    );
};

const innerStyle = StyleSheet.create({
    row: {
        flex: 1,
        flexDirection: 'row',
        alignContent: 'center',
    },
    rowItem: {
        flex: 1,
    },
    rowItemLarge: {
        flex: 2,
        textAlignVertical: 'center',
        textAlign: 'center',
    },
});

export default LED;
