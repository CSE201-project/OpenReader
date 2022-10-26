import React from 'react';
import {
    ScrollView,
    View,
    Text,
    StyleSheet,
    Dimensions,
    ActivityIndicator,
    Modal,
    Pressable,
    BackHandler
} from 'react-native';
import { Button, Divider, Image } from 'react-native-elements';

import { blobToBase64 } from '../Services/Converter';
import { downloadReportToDocuments } from '../Services/FSTools';
import mainTheme from '../MainTheme'

import Profile from '../Components/Profile';
import { useFocusEffect } from '@react-navigation/core';
import MainTheme from '../MainTheme';

function getCurrentDate(analysis_date) {
    let date = new Date(analysis_date);
    let hour = String(date.getHours()).padStart(2, '0');
    let minute = String(date.getMinutes()).padStart(2, '0');
    let second = String(date.getSeconds()).padStart(2, '0');
    let day = String(date.getDate()).padStart(2, '0');
    let month = String(date.getMonth() + 1).padStart(2, '0');
    let year = date.getFullYear();

    date = `${day}/${month}/${year}, ${hour}:${minute}:${second}`;
    return date;
};

function isPositiveResult(result) {
    return (result !== "Negative");
}

const ResultDetails = ({title, result, quantity}) => {
    return (<View style={{margin:0, alignSelf:'stretch'}}>
        <Text style={styles.resultTitle}>{title}</Text>
        <View style={{flexDirection:'row'}}>
            <View style={{minWidth:100}}>
                <Text>Result : </Text>
                <Text style={{backgroundColor:'#efefef'}}>Message : </Text>
                <Text>Concentration : </Text>
                <Text style={{backgroundColor:'#efefef'}}>Concentration : </Text>
            </View>
            <View>
                <Text>{result}</Text>
                <Text style={{backgroundColor:'#efefef'}}>{quantity.message}</Text>
                <Text>{quantity.prediction1}</Text>
                <Text style={{backgroundColor:'#efefef'}}>{quantity.prediction2}</Text>
            </View>           
        </View></View>);
};

export default function MeasureResultScreen({ navigation, route }) {

    let { id, analysis_date, strip_type, results } = route.params.patientResults;

    const [displayProfile, setDisplayProfile] = React.useState(false);
    const [loadingProfile, setLoadingProfile] = React.useState(false);
    const [showPicture, setShowPicture] = React.useState(false);
    const [picture, setPicture] = React.useState({ uri: null });
    const [displayResultDetails, setDisplayResultDetails] = React.useState(false);
    const [displayReport, setDisplayReport] = React.useState(false);
    const [resultID] = React.useState(id);

    
    console.log('Result ID : ', id);
    let profile = results?.prox_noise?.profile;
    let result = results?.prox_noise?.result;
    let quantity = results?.prox_noise?.quantity;
    let patient_id = route.params.patientID;
    let device = route.params.device;
    let date = getCurrentDate(analysis_date);

    useFocusEffect(
        React.useCallback(() => {
            const onBackPress = () => {
                navigation.popToTop();
                return true;
            }

            BackHandler.addEventListener('hardwareBackPress', onBackPress);
            return () => BackHandler.removeEventListener('hardwareBackPress', onBackPress);
        })
    )
   
    const fetchPicture = async () => {
        setShowPicture(true);
        try {
            let picture = {
                uri: null,
                headers: {
                    Pragma: 'no-cache',
                },
                body: '{}',
                cache: 'reload',
            };

            setPicture(picture);

            let pictureData = await route.params.device.getFile(resultID, "cropped");
            let image = await blobToBase64(pictureData);
            picture = {
                uri: image,
                headers: {
                    Pragma: 'no-cache',
                },
                body: '{}',
                cache: 'reload',
            };
            setPicture(picture);
        } catch (e) {
            console.error('Could not load image from device : ', e);
        }
    };

    const downloadReport = async () => {
        console.log('calling downloadReport');
        await downloadReportToDocuments(device, resultID);
    };

    return (
        <ScrollView style={styles.MainContainer}>
            <View style={styles.section}>
                <Text style={styles.itemTitle}>Patient ID</Text>
                <Text style={{...styles.item, ...styles.patientId}}>{patient_id}</Text>
                <View style={styles.itemSeparator} />
            </View>
            <View style={styles.section}>
                <Text style={styles.itemTitle}>Type</Text>
                <Text style={styles.item}>{strip_type}</Text>
                <View style={styles.itemSeparator} />
            </View>
            <View style={styles.section}>
                <Text style={styles.itemTitle}>Date</Text>
                <Text style={styles.item}>{date}</Text>
                <View style={styles.itemSeparator} />
            </View>
            <View style={styles.section}>
                <Text style={styles.itemTitle}>Result</Text>
                <Pressable onPress={() => setDisplayResultDetails(!displayResultDetails)} >
                    <Text style={isPositiveResult(quantity.message) ? styles.resultPositiveText : styles.resultNegativeText}>{quantity.message}</Text>
                </Pressable>
                <View style={styles.itemSeparator} />
            </View>
            <View style={styles.section}>
                <View style={styles.buttonBox}>
                    {/* <Button
                        title="Show Picture"
                        containerStyle={styles.buttonContainer}
                        onPress={async () => await fetchPicture()}
                        loading={loadingProfile}
                    />
                    <Button
                        title="Show Profile"
                        containerStyle={styles.buttonContainer}
                        onPress={() => !loadingProfile && setDisplayProfile(!displayProfile)}
                        loading={loadingProfile}
                    /> */}
                    <Button
                        title="Download Full Report"
                        titleStyle={styles.button}
                        containerStyle={styles.buttonContainer}
                        onPress={async () => await downloadReport()}
                    />
                </View>
            </View>
            <Modal
                    animationType="slide"
                    transparent={true}
                    visible={showPicture}
                    onRequestClose={() => { setShowPicture(!showPicture) }}
                >
                    <View style={styles.centeredView}>
                        <View style={styles.modalView}>
                            <Image
                                source={picture}
                                style={{
                                    width: Dimensions.get('window').width,
                                    height: Dimensions.get('window').height,
                                    resizeMode: 'contain',
                                }}
                                PlaceholderContent={<ActivityIndicator />}
                            />
                            <Pressable
                                style={[styles.button, styles.buttonClose]}
                                onPress={() => setShowPicture(!showPicture)}
                            >
                                <Text style={styles.textStyle}>Hide</Text>
                            </Pressable>
                        </View>
                    </View>
                </Modal>
            <Modal
                animationType="slide"
                transparent={false}
                visible={displayProfile}
                onRequestClose={() => { setDisplayProfile(false) }}
            >
                <View style={styles.centeredView}>
                <View style={styles.modalView}>
                    {displayProfile && <Profile profile={profile} />}
                </View></View>
            </Modal>
            <Modal
                animationType="slide"
                transparent={false}
                visible={displayResultDetails}
                onRequestClose={() => { setDisplayResultDetails(false) }}
            >
                <View style={styles.modalView}>
                    {<ResultDetails title={'ProxNoise'} result={results?.prox_noise?.result} quantity={results?.prox_noise?.quantity} />}
                    {<ResultDetails title={'ExpoFit'} result={results?.expo_fit?.result} quantity={results?.expo_fit?.quantity} />}
                    {<ResultDetails title={'Poly2'} result={results?.poly2?.result} quantity={results?.poly2?.quantity} />}
                    {<ResultDetails title={'Test-ProxNoise'} result={results?.test_minus_noise?.result} quantity={results?.test_minus_noise?.quantity} />}
                </View>
            </Modal>
        </ScrollView>
    );
}

const styles = StyleSheet.create({
    MainContainer: {
        ...mainTheme.MainContainer,
        //justifyContent: 'center',
        flex: 1,
        backgroundColor: 'white',
        padding: '5%'
    },
    section: {
        marginTop: '5%',
        marginBottom: '2%'
    },
    itemTitle: {
        marginTop: 20,
        fontSize: 25,
        color: '#7a7a7a',
        fontWeight: 'bold',
        flex: 1,
        alignSelf: 'flex-start',
        marginLeft: '15%'
    },
    item: {
        alignSelf: 'center',
        flex: 1,
        fontSize: 30,
        fontWeight: 'bold'
    },
    patientId: {
        fontSize: 35
    },
    itemSeparator: {
        height: 1.5,
        width: "70%",
        backgroundColor: "#d3d3d3",
        marginLeft: "15%",
    },
    resultPositiveText: {
        color: MainTheme.Colors.red,
        alignSelf: 'center',
        flex: 1,
        fontSize: 42,
        fontWeight: 'bold'
    },
    resultNegativeText: {
        color: '#000',
        alignSelf: 'center',
        flex: 1,
        fontSize: 42,
        fontWeight: 'bold'
    },
    resultTitle: {
        color: '#B40513',
        alignSelf: 'flex-start',
        fontSize: 18,
        fontWeight: 'bold'
    },
    buttonContainer: {
        flex: 1,
        marginTop: 20,
        margin: 20,
        borderWidth: 1
    },
    centeredView: {
        flex: 1,
        justifyContent: "center",
        alignItems: "center",
        marginTop: 22
    },
    modalView: {
        margin: 20,
        backgroundColor: "white",
        borderRadius: 20,
        padding: 15,
        alignItems: "center",
        shadowColor: "#000",
        shadowOffset: {
            width: 0,
            height: 2
        },
        shadowOpacity: 0.25,
        shadowRadius: 4,
        elevation: 5
    },
    button: {
        
        //padding: 10,
        //elevation: 2
    },
    buttonOpen: {
        backgroundColor: "#B40513",
    },
    buttonClose: {
        backgroundColor: "#B40513",
        marginTop: 5
    },
    textStyle: {
        color: "white",
        fontWeight: "bold",
        textAlign: "center",
        fontSize: 15
    },
    modalText: {
        marginBottom: 15,
        textAlign: "center"
    },
    buttonBox: {
        flex: 1,
        flexDirection: 'row',
        alignContent: 'center',
        justifyContent:'center',
        marginLeft: '30%',
        marginRight: '30%'
    },
    pdf: {
        flex:1,
        width: Dimensions.get('window').width,
        height:Dimensions.get('window').height
    }
});