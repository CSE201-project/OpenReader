'use strict'

import React from 'react';
import { 
    ActivityIndicator,
    Alert,
    View,
    Modal,
    Text 
} from 'react-native';
import { CountdownCircleTimer } from 'react-native-countdown-circle-timer';
import { Button } from 'react-native-elements';

const DEFAULT_INCUBATION_TIME = 30 * 60;

function secondsToHumanReadableTime(seconds)
{
    let result = '';
    if (seconds > 60) {
        let mn = Math.floor(seconds / 60);
        seconds = seconds - (mn * 60);
        result = `${mn}m `;
    }
    result = `${result}${seconds}s`;
    return result;
}

export default function AutonomousModeDialog({visible, onCompleted}) {

    const completedWait = () => {
        onCompleted();
    }

    const confirmSkip = async () => {
        let res = await new Promise(resolve => {
            Alert.alert(
            "Skip incubation time ?",
            'The analysis may be altered if the sample did not have time to migrate',
            [
                {
                    text: "Cancel",
                    onPress: () => resolve(false),
                    style: "cancel",
                },
                {
                    text: "OK",
                    onPress: () => resolve(true)
                },
            ]
        )});
        if (res) {
            onCompleted();
        }
    }

    return (<Modal 
            visible={visible}
            transparent={true}
        >
        <View style={styles.dialog}>
            <View style={styles.progressDialog}>
                <CountdownCircleTimer 
                    isPlaying
                    duration={DEFAULT_INCUBATION_TIME}
                    colors={'#B40513'}
                    size={250}
                    onComplete={completedWait}
                >
                    {({ remainingTime, animatedColor }) => (
                        <View style={{
                            justifyContent:'center', 
                            alignContent:'center',
                            alignItems:'center'
                        }}>
                            <Text style={styles.title}> Waiting </Text>
                            <Text style={styles.title}>Time</Text>
                            <Text style={styles.countdownText}>{secondsToHumanReadableTime(remainingTime)}</Text>
                        </View>
                    )}
                </CountdownCircleTimer>
                <View style={{padding:'10%'}}>
                    <Button 
                        title='Skip Incubation'
                        onPress={async () => await confirmSkip()}
                    />
                </View>
            </View>
        </View>
    </Modal>)
}

const styles = {
    dialog: {
        flex:1,
        backgroundColor:"#00000040",
        justifyContent:"center",
        alignItems:"center"        
    },
    progressDialog: {
        backgroundColor:"white",
        padding:'20%',
        borderRadius:20, 
        width:"80%", 
        justifyContent:"center",
        alignItems:"center"
    },
    title: {
        fontSize:24
    },
    countdownText: {
        fontSize:38
    }
}
