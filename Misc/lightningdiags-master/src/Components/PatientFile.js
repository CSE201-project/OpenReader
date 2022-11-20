import React from 'react';
import {
    View,
    Text,
    FlatList,
    StyleSheet,
    Pressable,
    Alert
} from 'react-native';
import { Button, Icon } from 'react-native-elements';
import { TouchableWithoutFeedback } from 'react-native-gesture-handler';
import mainTheme from '../MainTheme'

const ItemSeparator = () => (<View style={styles.itemSeparator} />);

const Item = ({item, onSelect, onDeleteRequest}) => (
    <View style={{ 
        marginLeft: 20, 
        marginRight: 20, 
        padding: 20,
        flex:1, 
        justifyContent:'space-between',
        flexDirection: 'row'
     }}
    >
        <View style={{flex:5}}>
            {/* <TouchableWithoutFeedback onPress={onSelect}> */}
                <Text style={styles.item}>{item.strip_type}</Text>
                <Text>{new Date(item.analysis_date).toLocaleDateString()} {new Date(item.analysis_date).toLocaleTimeString()}</Text>
            {/* </TouchableWithoutFeedback> */}
        </View>
        <Button onPress={onSelect} title="Open" titleStyle={styles.buttonText} buttonStyle={styles.button} />
        {/* <View style={{flex:1, alignContent:'center', alignItems:'center'}}>
            <Pressable
                style={styles.button}
                onPress={onSelect}
            >
                <Text style={{ color: 'white' }}>Open</Text>
            </Pressable>
        </View> */}
    </View>
);

class PatientFile extends React.Component {
    device = this.props.device;
    itemID = this.props.itemID;

    constructor(props) {
        super(props);
        this.state = {
            file: 'loading...',
        };
    }

    async componentDidMount() {
        this.setState({
            file: await this.device.getPatientAnalysis(this.itemID),
        });
    }

    async getPatientMeasure(measureID) {
        let result = await this.device.getAnalysisFile(measureID);
        return result;
    }

    async openConfirmationDialog(item) {
        let d = new Date(item.analysis_date);
        Alert.alert(
            item.strip_type + " from " + d.toLocaleDateString() + " " + d.toLocaleTimeString() + " will be deleted",
            "Are you sure ?",
            [
                { text: "Cancel" },
                { text: "OK", onPress: async () => await this.deleteAnalysis(item.id) }
            ],
        );
    }

    async deleteAnalysis(id) {
        await this.device.deleteAnalysis(id);
        this.setState({
            file: await this.device.getPatientAnalysis(this.itemID),
        });
    }

    render() {
        let date = new Date(this.state.file['date']).toLocaleDateString();
        return (
            <View style={styles.MainContainer}>
                <View style={styles.idContainer}>
                    <Text style={styles.id}>{this.itemID}</Text>
                    <Text style={{ fontSize: 15 }}>Created {date}</Text>
                </View>
                <Button
                    title="New Measure"
                    titleStyle={styles.buttonNewMeasureTitle}
                    icon={<Icon type={'material'} size={36} name={'add'} color={'#fff'} />}
                    containerStyle={styles.buttonNewMeasureContainer}
                    onPress={() => { this.props.navigation.navigate('Measure', { patientID: this.itemID }) }}
                />
                <FlatList
                    data={this.state.file['measures']}
                    ItemSeparatorComponent={ItemSeparator}                    
                    renderItem={({ item }) => (<Item 
                        item={item} 
                        onSelect={async () => { this.props.navigation.navigate('MeasureResult', { patientID: this.itemID, patientResults: await this.getPatientMeasure(item.id) }) }} 
                        onDeleteRequest={async () => await this.openConfirmationDialog(item)}
                    />)}
                    keyExtractor={(item, index) => index.toString()}
                    style={styles.list}
                />
            </View>
        );
    }
}

const styles = StyleSheet.create({

    MainContainer: {
        ...mainTheme.MainContainer,
        justifyContent: 'space-evenly',
        alignItems: 'stretch',        
        flex: 1,
        backgroundColor: 'white',
    },
    list: {
        flex: 5
    },
    item: {
        // padding: 10,
        fontSize: 24,
        // height: 44,
        color: 'black',
        // backgroundColor: '#f2efef',
        // marginLeft: "-20%"
    },
    buttonNewMeasureContainer: {
        marginLeft: '20%',
        marginRight: '20%',
        marginBottom: '4%',
        borderRadius: 20,
    },
    buttonNewMeasureTitle: {
        fontSize: 24
    },
    idContainer: {
        //backgroundColor: '#d3d3d3',
        alignItems: 'center',
        padding: 50
    },
    id: {
        color: 'black',
        fontSize: 34,
        fontWeight: 'bold',
    },
    itemSeparator: {
        height: 2,
        width: "90%",
        backgroundColor: "#d3d3d3",
        marginLeft: "5%",
        margin: 10
    },
    button: {
        flex:1,
        alignItems:'center', 
        justifyContent:'center',
        borderRadius: 20,
        marginRight: 20,
        overflow: 'hidden',
        backgroundColor: "#B40513",
        width: 100
    },
    buttonText: {
        fontSize: 24
    }
});

export default PatientFile;