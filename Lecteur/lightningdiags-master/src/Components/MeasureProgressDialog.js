'use strict'

import React from 'react';
import { 
    ActivityIndicator,
    View,
    Modal,
    Text 
} from 'react-native';
import { CountdownCircleTimer } from 'react-native-countdown-circle-timer';

export default function MeasureProgressDialog({visible}) {
    return (<Modal 
            visible={visible}
            transparent={true}
        >
        <View style={styles.dialog}>
            <View style={styles.progressDialog}>
                <CountdownCircleTimer 
                    isPlaying
                    duration={20}
                    colors={'#B40513'}
                    size={250}
                >
                    {({ remainingTime, animatedColor }) => (
                        <View style={{
                            justifyContent:'center', 
                            alignContent:'center',
                            alignItems:'center'
                        }}>
                            <Text style={styles.title}> Analyzing&nbsp;</Text>
                            <Text style={styles.title}>Strip</Text>
                            <Text style={styles.countdownText}>{remainingTime}s</Text>
                        </View>
                    )}
                </CountdownCircleTimer>
                <Text style={styles.warning}>Do NOT remove the cassette from the reader</Text>
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
    },
    warning: {
        marginTop:'20%',
        fontSize: 22
    }
}
