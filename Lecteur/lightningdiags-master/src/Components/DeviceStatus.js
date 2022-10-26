import React, { useEffect } from 'react';
import {ActivityIndicator, Alert, StyleSheet, Text, View} from 'react-native';
import {Button, Divider, Overlay} from 'react-native-elements';
import Icon from 'react-native-vector-icons/MaterialCommunityIcons';

import {Colors} from '../MainTheme';

export default function DeviceStatus({device, viewStyle, titleStyle, onConnectionStatusChanged}) {
    
    const [isConnected, setConnected] = React.useState(device.Connected);
    const [isConnecting, setConnecting] = React.useState(false);
    const [buttonText, setButtonText] = React.useState("Disconnected");

    const tryConnection = async () => {
        setConnecting(true);
        try {
            await new Promise(resolve => setTimeout(() => {
                resolve();
            }, 2000));
            let success = await device.connect();
            setConnected(success);
            setButtonText(success ? ' ' + device.Name : " Connection error");
            onConnectionStatusChanged(success);
        } catch (error) {
            Alert.alert("Connection error", error);
        }
        setConnecting(false);
    }

    useEffect(async () => {
        if (!isConnected) {
            await tryConnection();
        }
    }, [isConnected]);

    return (
        <View style={viewStyle}>
            <Button
                icon={<Icon
                    name={isConnected ? 'link' : 'link-off'}
                    size={25}
                    color={isConnected ? 'white' : 'red'}
                    containerStyle={innerStyle.buttonIcon}
                    iconStyle={innerStyle.buttonIcon}
                    iconContainerStyle={innerStyle.buttonIcon}
                />}
                title={buttonText}
                titleStyle={titleStyle}
                buttonStyle={
                    isConnected
                        ? innerStyle.buttonConnected
                        : innerStyle.button
                }
                iconContainerStyle={innerStyle.buttonIcon}
                onPress={async () => await tryConnection()}
                loading={isConnecting}
                disabled={isConnected}
            />
        </View>
    );
}

const innerStyle = StyleSheet.create({
    title: {
        fontSize: 18,
    },
    button: {
        color: Colors.orange,
        backgroundColor: Colors.grey,
    },
    buttonConnected: {
        color: Colors.blue,
        backgroundColor: Colors.red,
    },
    buttonIcon: {
        margin: '10%',
        padding: 30,
    },
});
