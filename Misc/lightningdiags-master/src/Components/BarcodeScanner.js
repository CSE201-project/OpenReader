import React from "react";
import { RNCamera } from "react-native-camera";
import BarcodeMask from "react-native-barcode-mask";
import {
    ActivityIndicator,
    View,
    Modal,
    Text,
    TouchableOpacity
} from 'react-native';
import { Button, Icon } from 'react-native-elements';

const styles = {
    dialog: {
        flex: 1,
        backgroundColor: "#000000B0",
        justifyContent: "center",
        alignItems: "center"
    },
    container: {
        backgroundColor: "white",
        padding: '5%',
        borderRadius: 5,
        width: "90%",
        margin: 20,
        alignItems: "center",
        flex: 1,
        flexDirection: 'column',
    },
    preview: {
        flex: 1,
        //padding: '30%',
        width: '100%',
        height: '100%',
        overflow:'hidden'
    },
    capture: {
        flex: 0,
        backgroundColor: '#ededed',
        borderRadius: 5,
        padding: 15,
        paddingHorizontal: 20,
        alignSelf: 'center',
        margin: 20,
    },
    controlCartouche: {
        flex: 1,
        margin: '10%'
    },
    row: {
        flexDirection: 'row',
        justifyContent: 'space-evenly',
        alignItems: 'center'
    },
    controls: {
        padding: '10%'
    },
    barcode: {
        fontSize: 24,
        fontWeight:'bold'
    }
}

const PendingView = () => (
    <View
        style={{
            flex: 1,
            backgroundColor: 'lightgreen',
            justifyContent: 'center',
            alignItems: 'center',
        }}
    >
        <Text>Waiting</Text>
    </View>
);

export default function BarcodeScanner({ visible, onRequestClose, onBarcodeValidated }) {

    const [scanning, setShouldScan] = React.useState(true);
    const [currentBarcode, setCurrentBarcode] = React.useState('');
    const [currentCameraType, setCurrentCameraType] = React.useState(RNCamera.Constants.Type.back);

    const handleBarcodeRead = (event) => {
        console.log('barcode read');
        console.log(event.data);
        setCurrentBarcode(event.data);
        setShouldScan(false);
    }

    const handleBarcodeValidated = () => {
        onBarcodeValidated(currentBarcode);
        onRequestClose();
    }

    const handleRestartScan = () => {
        setCurrentBarcode('');
        setShouldScan(true);
    }

    const handleSwitchCamera = () => {
        if (currentCameraType === RNCamera.Constants.Type.back) {
            setCurrentCameraType(RNCamera.Constants.Type.front);
        }
        else {
            setCurrentCameraType(RNCamera.Constants.Type.back);
        }
    }

    return (
        <Modal visible={visible} onRequestClose={onRequestClose}>
            <View style={styles.dialog}>
                <View style={styles.container}>
                    <View></View>
                    <RNCamera
                        style={styles.preview}
                        type={currentCameraType}
                        flashMode={RNCamera.Constants.FlashMode.on}
                        captureAudio={false}
                        androidCameraPermissionOptions={{
                            title: 'Permission to use camera',
                            message: 'We need your permission to use your camera',
                            buttonPositive: 'Ok',
                            buttonNegative: 'Cancel',
                        }}
                        onBarCodeRead={scanning ? handleBarcodeRead : null}
                    >

                        {({ camera, status, recordAudioPermissionStatus }) => {
                            if (status !== 'READY') return <PendingView />;
                            return (
                                <BarcodeMask
                                    edgeColor={'#62B1F6'}
                                    showAnimatedLine={false}
                                    outerMaskOpacity={0.8}
                                    width='90%'
                                    height='90%'
                                />
                                
                            );
                        }}
                    </RNCamera>
                    <View style={{...styles.controlCartouche}}>
                        <View style={{...styles.row}}>
                        <Button
                            containerStyle={styles.controls}
                            icon={
                                <Icon
                                    name='camera-switch'
                                    type='material-community'
                                    size={50}
                                    color='white'
                                />
                            }
                            onPress={() => handleSwitchCamera()}
                        />
                        <Button
                            containerStyle={styles.controls}
                            title={scanning ? 'Scanning...' : 'Scan'}
                            icon={
                                <Icon
                                    name={scanning ? 'barcode-scan' : 'camera-retake'}
                                    type='material-community'
                                    size={50}
                                    color='white'
                                />
                            }
                            loading={scanning}                                    
                            loadingProps={{size:'large'}}
                            onPress={() => handleRestartScan()}
                        />
                        </View>
                        <View style={{...styles.row}}>
                            <View>
                                <Text>Barcode</Text>
                                <Text style={styles.barcode}>{currentBarcode}</Text>
                            </View>
                            <View>
                                <Button title='OK' onPress={() => handleBarcodeValidated()} />
                            </View>
                        </View>
                    </View>
                </View>
            </View>
        </Modal>
    )
}
