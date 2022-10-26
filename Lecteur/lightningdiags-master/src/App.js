/**
 * @format
 * @flow strict-local
 */

import 'react-native-gesture-handler';
import React from 'react';

import {NavigationContainer} from '@react-navigation/native';
import {createStackNavigator} from '@react-navigation/stack';

import {ThemeProvider} from 'react-native-elements';
import mainTheme from './MainTheme';

import HomeScreen from './Screens/HomeScreen';
import NewMeasureScreen from './Screens/NewMeasureScreen';
import BrowseMeasuresScreen from './Screens/BrowseMeasuresScreen';
import PatientFileScreen from './Screens/PatientFileScreen';
import MeasureResultScreen from './Screens/MeasureResultScreen';

const Stack = createStackNavigator();

import SolariumDevice from './Services/SolariumDevice';
import DeviceControlScreen from './Screens/DeviceControlScreen';

const device = new SolariumDevice('192.168.20.1');
//const device = new SolariumDevice('192.168.1.14');

class App extends React.Component {
    constructor(props) {
        super(props);
    }

    render() {
        return (
            <NavigationContainer>
                <ThemeProvider theme={mainTheme}>
                    <Stack.Navigator>
                        <Stack.Screen
                            name="Home"
                            component={HomeScreen}
                            initialParams={{device: device}}
                            options={{title:''}}
                        />
                        <Stack.Screen
                            name="Measure"
                            component={NewMeasureScreen}
                            initialParams={{device: device}}
                            options={{ title: "New Measure" }}
                        />
                        <Stack.Screen
                            name="DeviceControl"
                            component={DeviceControlScreen}
                            initialParams={{device: device}}
                            options={{ title: "Device Control" }}
                        />
                         <Stack.Screen
                            name="BrowseMeasures"
                            component={BrowseMeasuresScreen}
                            initialParams={{device: device}}
                            options={{ title: "Results" }}
                        />
                        <Stack.Screen
                            name="PatientFile"
                            component={PatientFileScreen}
                            initialParams={{device: device}}
                            options={{ title: "Patient File" }}
                        />
                        <Stack.Screen
                            name="MeasureResult"
                            component={MeasureResultScreen}
                            initialParams={{device: device}}
                            options={{ title: "Measure Result" }}
                        />
                    </Stack.Navigator>
                </ThemeProvider>
            </NavigationContainer>
        );
    }
}

export default App;
