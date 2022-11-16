'use strict';
import React from 'react';
import {Text, View, StyleSheet, ScrollView, SafeAreaView} from 'react-native';
import {Button} from 'react-native-elements';

import mainTheme from '../MainTheme';
import LumedixLogo from '../Components/LumedixLogo';
import DeviceStatus from '../Components/DeviceStatus';
import {version as appVersion} from '../../package.json';

export default function HomeScreen({navigation, route}) {

    const [deviceConnected, setdeviceConnected] = React.useState(route.params.device.Connected);


    return (
        <View style={style.mainView}>
            <LumedixLogo style={style.logo} />
            <Text style={style.subtitle}>
                by <Text style={{
                    fontStyle:'italic', 
                    fontSize: 32
                    }}>Lumedix</Text>
            </Text>
            <DeviceStatus
                viewStyle={style.buttonContainer}
                device={route.params.device}
                titleStyle={style.buttons}
                onConnectionStatusChanged={(success) => setdeviceConnected(success)}
            />
            <Button
                title="New Measure"
                titleStyle={style.buttons}
                containerStyle={style.buttonContainer}
                onPress={() => {
                    navigation.navigate('Measure');
                }}
                disabled={!deviceConnected}
            />
            <Button
                title="Results Archives"
                titleStyle={style.buttons}
                containerStyle={style.buttonContainer}
                onPress={() => {
                    navigation.navigate('BrowseMeasures');
                }}
                disabled={!deviceConnected}
            />
            <View style={style.filler} />
            <Text>v{appVersion}</Text>
        </View>
    );
}

const style = StyleSheet.create({
    mainView: {
        ...mainTheme.MainView,
        padding: 20,
        flex: 1,
        alignContent: 'space-around',
        justifyContent: 'center',
    },
    logo: {},
    subtitle: {
        flex: 1,
        alignSelf: 'center',
        margin: 10,
        fontSize: 28
    },
    buttonContainer: {
        flex: 1,
        marginLeft: '30%',
        marginRight: '30%'
    },
    buttons:{
        fontSize: 30,
        margin: '5%'
    },  
    filler: {
        flex: 1,
    },
});
