'use strict';
import React from 'react';
import {
    Text,
    View,
    StyleSheet,
    Image,
    TextInput,
    ScrollView,
    ActivityIndicator,
    Alert,
    Pressable
} from 'react-native';
import { Button, ButtonGroup } from 'react-native-elements';

import mainTheme from '../MainTheme';
import MeasureProgressDialog from '../Components/MeasureProgressDialog';
import BarcodeScanner from '../Components/BarcodeScanner';
import AutonomousModeDialog from '../Components/AutonomousModeDialog';
import MatrixSelector from '../Components/MatrixSelector';

const VALID_INPUT = /^[a-zA-Z0-9-_=()]+$/i;
const DEFAULT_PATIENTID = 'Patient001';
const DEFAULT_STRIP_BATCH_ID = 'B2101_001';
const DEFAULT_MATRIX = 'covid-blood';
const NB_ANALYSIS_LEFT_THRESHOLD = 50;

const ItemSeparator = () => <View style={styles.itemSeparator} />

export default function NewMeasureScreen({ navigation, route }) {
    let defaultInput = route.params.patientID ? route.params.patientID : DEFAULT_PATIENTID;
    const [patientID, setPatientID] = React.useState(defaultInput);
    const [stripBatchID, setStripBatchID] = React.useState(DEFAULT_STRIP_BATCH_ID);
    const [matrixType, setMatrixType] = React.useState(DEFAULT_MATRIX);
    const [analysingStrip, setAnalysingStrip] = React.useState(false);
    const [showBarcodeScanner, setShowBarcodeScanner] = React.useState(false);
    const [showAutonomousDialog, setShowAutonomousDialog] = React.useState(false);

    function openDeviceErrorDialog(err) {
        let errMsg = err || "Input may be invalid";
        Alert.alert(
            "Measure Error",
            errMsg,
            [
                { text: "OK" },
            ],
        );
    }

    function openStatusErrorDialog(status) {
        Alert.alert(
            "Status Error",
            status,
            [
                { text: "OK" },
            ]
        )
    }

    const checkRemainingSpace = async () => {
        let nbOfAnalysisRemaining = await route.params.device.getNbOfAnalysisRemaining();
        if (nbOfAnalysisRemaining < NB_ANALYSIS_LEFT_THRESHOLD) {
            return new Promise(resolve => {
                Alert.alert(
                "Warning : Low Disk Space on Reader",
                `\nOnly ${nbOfAnalysisRemaining} analysis possible, continue anyway ?\n\nPlease contact Lumedix : coviblood@lumedix.com`,
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
        } else {
            return Promise.resolve(true);
        }     
    }

    const checkCassetteInReader = async () => {
        return new Promise(resolve => {
            Alert.alert(
            "Please ensure the cassette is inside the reader",
            '',
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
    }

    const runAnalysis = async () => {        
        
        if (!patientID.match(VALID_INPUT)) { 
            openDeviceErrorDialog("Invalid patient ID");
            return;
        }

        if (!stripBatchID.match(VALID_INPUT)) {
            openDeviceErrorDialog("Invalid Strip Batch N°");
            return;
        }

        let params = {
            patient_id: patientID,
            strip_batch_id: stripBatchID,
            strip_type: matrixType,
            date: new Date().toISOString()
        };
          
        console.log("analysis params:", params);
        setAnalysingStrip(true);
        let patientResults = await route.params.device.takeMeasure(params);
        setAnalysingStrip(false);

        if (!patientResults) {
            openDeviceErrorDialog("Reader internal error");
            return;
        }
        else if (patientResults.status !== "SUCCESS") {
            openStatusErrorDialog(patientResults.status);
            return;
        }
        navigation.navigate('MeasureResult', { patientResults: patientResults, patientID: patientID });
    }

    const autonomousModeCompleted = async () => {        
        setShowAutonomousDialog(false);
        await runAnalysis();
    }

    const runAutomaticAnalysis = async () => {
        if (!await checkRemainingSpace()) {
            return;
        }

        if (!await checkCassetteInReader()) {
            return;
        }

        setShowAutonomousDialog(true);
    }

    const runDirectAnalysis = async () => {
        if (!await checkRemainingSpace()) {
            return;
        }

        if (!await checkCassetteInReader()) {
            return;
        }
        
        await runAnalysis();
    }

    return (
        <ScrollView style={styles.mainView}>
            <AutonomousModeDialog visible={showAutonomousDialog} onCompleted={autonomousModeCompleted} />
            <MeasureProgressDialog visible={analysingStrip} />
            <BarcodeScanner visible={showBarcodeScanner}
                onRequestClose={() => setShowBarcodeScanner(false)}
                onBarcodeValidated={(barcode) => setPatientID(barcode)}
            />
            <View style={styles.section}>
                <View style={styles.row}>
                    <Image
                        source={require('../Assets/patient_icon.png')}
                        style={styles.icon}
                    />
                    <Text style={styles.sectionTitle}>ID Patient</Text>
                </View>
                <View style={styles.row}>

                    <TextInput style={styles.input} placeholder="..." value={patientID} onChangeText={text => setPatientID(text)} />
                    <Pressable onPress={() => setShowBarcodeScanner(true)}>
                        <Image
                            source={require('../Assets/barcode_scan.png')}
                            style={styles.icon}
                        />
                    </Pressable>
                </View>
            </View>
            <View style={styles.section}>
                <View style={styles.row}>
                    <Image
                        source={require('../Assets/batch_no_icon.png')}
                        style={styles.icon}
                    />
                    <Text style={styles.sectionTitle}>Strip Batch N°</Text>
                </View>
                <View style={styles.row}>

                    <TextInput style={styles.input} value={stripBatchID} onChangeText={text => setStripBatchID(text)} />
                    <Pressable onPress={() => setShowBarcodeScanner(true)}>
                        <Image
                            source={require('../Assets/barcode_scan.png')}
                            style={styles.icon}
                        />
                    </Pressable>
                </View>
            </View>
            <View style={styles.section}>
                <View style={styles.row}>
                    <Image
                        source={require('../Assets/strip_icon.png')}
                        style={styles.icon}
                    />
                    <Text style={styles.sectionTitle}>Matrix Type</Text>
                </View>
                <MatrixSelector defaultMatrix={matrixType} onMatrixChanged={(id) => setMatrixType(id)}/>
            </View>
            <View style={styles.section}>
                <View style={styles.row}>
                </View>
                <View style={{...styles.row, margin:'10%', justifyContent: 'space-evenly'}}>
                    <Button
                        title="Automatic Mode"
                        titleStyle={styles.runButton}
                        containerStyle={styles.buttonContainer}
                        onPress={async () => await runAutomaticAnalysis(true)}
                    />
                    <Button
                        title="Take Measure"
                        titleStyle={styles.runButton}
                        containerStyle={styles.buttonContainer}
                        onPress={async () => await runDirectAnalysis()}
                    />
                    {/* <Button
                title="Device Control"
                containerStyle={styles.buttonContainer}
                onPress={() => navigation.navigate('DeviceControl')}
            /> */}
                </View>

            </View>
        </ScrollView>
    );
}

const styles = StyleSheet.create({
    mainView: {
        ...mainTheme.MainView,
        padding: 20,
        flex: 1,
    },
    section: {
        flex: 1,
        margin: '2%',
        justifyContent: 'center',
        alignItems : 'flex-start'
    },
    sectionTitle: {
        fontSize: 28,
        alignContent: 'center',
        color: '#777'
    },
    itemSeparator: {
        height: 1.5,
        width: "70%",
        backgroundColor: "#d3d3d3",
        marginLeft: "15%",
    },
    row: {
        flex: 1,
        flexDirection: 'row',
        alignItems: 'center'
    },
    icon: {
        width: 40,
        height: 40,
        resizeMode: 'contain',
        margin: 10,
    },
    input: {
        flex: 2,
        height: 60,
        fontSize: 24,
        borderColor: 'gray',
        borderWidth: 1,
        margin: 10,
    },
    filler: {
        flex: 2,
    },
    buttonContainer: {
        padding: 50,        
    },
    runButton: {
        fontSize: 30,
        margin: 30
    },
    loadingIcon: {
        paddingBottom: 0,
        backgroundColor: 'white',
        marginTop: 15
    }
});
