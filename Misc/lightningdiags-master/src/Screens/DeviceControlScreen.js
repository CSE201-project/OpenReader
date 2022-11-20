import {
    ActivityIndicator,
    Dimensions,
    ScrollView,
    StyleSheet,
} from 'react-native';
import LEDGroup from '../Components/LEDGroup';
import React, {useState} from 'react';
import mainTheme from '../MainTheme';
import {Button, Image} from 'react-native-elements';
import ImageZoom from 'react-native-image-pan-zoom';

const DeviceControlScreen = ({navigation, route}) => {
    const [picture, setPicture] = useState({uri: null});
    const [pictureLoading, setPictureLoading] = useState(false);

    const takePicture = async () => {
        setPictureLoading(true);
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

            let data = await route.params.device.takePictureAsBase64();
            picture = {
                uri: data,
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
        setPictureLoading(false);
    };

    return (
        <ScrollView style={style.mainView}>
            <LEDGroup
                type="UV"
                style={[style.section, style.ledGroup]}
                device={route.params.device}
            />
            <LEDGroup
                type="Visible"
                style={[style.section, style.ledGroup]}
                device={route.params.device}
            />
            <Button
                title="Start Measure"
                containerStyle={[style.section, style.buttonContainer]}
                onPress={async () => await takePicture()}
                loading={pictureLoading}
            />
            <ImageZoom
                style={style.section}
                cropWidth={Dimensions.get('window').width}
                cropHeight={Dimensions.get('window').width}
                imageWidth={Dimensions.get('window').width}
                imageHeight={Dimensions.get('window').width}>
                <Image
                    source={picture}
                    style={{
                        width: Dimensions.get('window').width,
                        height: Dimensions.get('window').width,
                        resizeMode: 'contain',
                    }}
                    PlaceholderContent={<ActivityIndicator />}
                />
            </ImageZoom>
        </ScrollView>
    );
};

const style = StyleSheet.create({
    mainView: {
        ...mainTheme.MainView,
        padding: 20,
        flex: 1,
    },
    section: {
        flex: 1,
    },
    sectionTitle: {
        fontSize: 20,
        alignContent: 'center',
    },
    row: {
        flex: 1,
        flexDirection: 'row',
    },
    icon: {
        width: 40,
        height: 40,
        resizeMode: 'contain',
        margin: 10,
    },
    ledGroup: {
        margin: 10,
    },
    buttonContainer: {
        margin: 10,
    },
});

export default DeviceControlScreen;
