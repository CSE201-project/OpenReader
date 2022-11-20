'use strict';

import React, { useState } from "react";
import { View, Image, Text, Pressable } from "react-native";
import MainTheme from "../MainTheme";

const Matrixes = [
    {
        id: 'covid-blood',
        text: 'Blood',
        icon: require('../Assets/blood.png')
    },
    {
        id: 'covid-serum',
        text: 'Serum',
        icon: require('../Assets/serum.png')
    }
];

const styles = {
    row: {
        flex: 1,
        flexDirection: 'row',
        alignItems: 'center',
        justifyContent: 'center'
    },
    matrixButton: {
        fontSize: 28,
        color:'#000'
    },
    matrixSelectedButton: {
        fontSize: 28,
        color:'#fff',
        backgroundColor: MainTheme.Colors.red
    },
    icon: {
        width: 40,
        height: 40,
        resizeMode: 'contain',
        margin: 10,
    }
};

export default function MatrixSelector({defaultMatrix, onMatrixChanged}) {

    const [currentMatrix, setCurrentMatrix] = useState(defaultMatrix || Matrixes[0].id);

    const handleClick = (matrix) => {
        setCurrentMatrix(matrix.id);
        if (onMatrixChanged) {
            onMatrixChanged(matrix.id);
        }
    }

    const getStyle = (id) => {
        return (currentMatrix === id) ? styles.matrixSelectedButton : styles.matrixButton;
    }

    return (
        <View style={{...styles.row,
            borderColor : '#000',
            borderWidth : 1,
            width: '98%',
            alignSelf: 'center'
        }}>
            {Matrixes.map((m, i) => (                
                <Pressable 
                    key={i}
                    style={{
                        ...styles.row,
                        padding:'2%',
                        ...getStyle(m.id)}}
                    onPress={() => handleClick(m)}
                >

                    <Image source={m.icon} style={styles.icon} />
                    <Text style={getStyle(m.id)}>{m.text}</Text>
                </Pressable>
            ))}
        </View>
    )
}