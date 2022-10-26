import React from 'react';
import {Image, Text, View, StyleSheet} from 'react-native';

export default function LumedixLogo({style}) {
    return (
        <View style={[innerStyle.view, {...style}]}>
            <Image
                source={require('../Assets/round_icon_lumedix.png')}
                style={innerStyle.logo}
            />
            <Text style={innerStyle.title}>Diag In Light</Text>
        </View>
    );
}

const innerStyle = StyleSheet.create({
    view: {
        flex: 1,
        flexDirection: 'row',
        alignContent: 'space-around',
        alignItems: 'center',
        justifyContent: 'center',
    },
    logo: {
        width: 50,
        height: 50,
        marginRight: 20,
        resizeMode: 'contain',
    },
    title: {
        fontSize: 32,
        fontWeight: 'bold',
        textAlignVertical: 'center',
    },
});
