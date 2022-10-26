import React from 'react';
import {
    View,
    Text,
    StyleSheet,
    Alert,
    ActivityIndicator,
    Dimensions
} from 'react-native';
import { Button } from 'react-native-elements';
import ProgressBar from 'react-native-progress/Bar';
import mainTheme from '../MainTheme'

class TakeMeasure extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            incubationIndicator: `Incubation for '${props.stripeID}'`,
            incubationProgress: 0,
            isIncubating: true,
            isLoading: false
        };
    }

    componentDidMount() {
        this.updateIncubationProgress();
    }

    sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    async updateIncubationProgress() {
        for (let progress = 0; progress < 1; progress += 0.1) {
            if (!this.state.isIncubating) {
                this.setState({
                    incubationIndicator: 'Incubation aborted',
                    incubationProgress: 1
                });
                break;
            }
            this.setState({ incubationProgress: progress });
            await this.sleep(1000);
            if (this.state.incubationProgress > 0.9) {
                this.setState({
                    incubationIndicator: 'Incubation done',
                    isIncubating: false
                });
            }
        }
    }

    abortIncubation() {
        this.setState({ isIncubating: false });
    }

    openConfirmationDialog() {
        if (!this.state.isIncubating)
            return;
        Alert.alert(
            "Abort Incubation",
            "Are you sure ?",
            [
                { text: "Cancel" },
                { text: "OK", onPress: () => this.abortIncubation() }
            ],
        );
    }

    openDeviceErrorDialog() {
        Alert.alert(
            "Measure Error",
            "Please try again",
            [
                { text: "OK" },
            ],
        );
    }

    openStatusErrorDialog(status) {
        Alert.alert(
            "Status Error",
            status,
            [
                { text: "OK" },
            ]
        )
    }

    async getAnalysisResults() {
        let params = {};
        params.patient_id = this.props.patientID;
        params.strip_type = this.props.stripeID;
        this.setState({ isLoading: true });
        let patientResults = await this.props.device.takeMeasure(params);
        this.setState({ isLoading: false });
        if (!patientResults) {
            this.openDeviceErrorDialog();
        } else if (patientResults.status !== "SUCCESS") {
            this.openStatusErrorDialog(patientResults.status);
        } else {
            this.props.navigation.navigate('MeasureResult', { patientResults: patientResults, patientID: this.props.patientID });
        }
    }

    render() {
        const progressWidth = Dimensions.get('window').width - 25;
        return (
            <View style={styles.MainContainer}>
                <Text style={styles.title} >{this.state.incubationIndicator}</Text>
                <ProgressBar progress={this.state.incubationProgress} width={progressWidth} height={25} color='rgba(180,5,19,1)' style={{ margin: 10 }} />
                <Button
                    title="Abort Incubation"
                    containerStyle={styles.buttonContainer}
                    onPress={() => this.openConfirmationDialog()}
                />
                <Button
                    title="Take Measure"
                    containerStyle={styles.buttonContainer}
                    disabled={this.state.isIncubating}
                    onPress={async () => await this.getAnalysisResults()}
                />
                <View style={styles.loadingIcon}>
                    <ActivityIndicator size='large' animating={this.state.isLoading} color={mainTheme.Colors.red} />
                </View>
            </View>
        );
    }
}

const styles = StyleSheet.create({
    MainContainer: {
        ...mainTheme.MainContainer,
        backgroundColor: 'white',
        flex: 1
    },
    buttonContainer: {
        margin: 50
    },
    loadingIcon: {
        paddingBottom: 0,
        backgroundColor: 'white'
    },
    title: {
        fontSize: 25,
        marginTop: 50,
        marginBottom: 10,
        textAlign: 'center',
        color: 'black',
        fontWeight: 'bold',
    }
});

export default TakeMeasure;