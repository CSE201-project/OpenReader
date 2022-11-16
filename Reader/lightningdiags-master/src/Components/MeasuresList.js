import React from 'react';
import {
    StyleSheet,
    FlatList,
    Text,
    View,
    ActivityIndicator,
    Pressable,
    Alert,
    TouchableOpacity
} from 'react-native';
import { SearchBar, Button } from 'react-native-elements';
import { TouchableWithoutFeedback } from 'react-native-gesture-handler';
import MainTheme from '../MainTheme';
import mainTheme from '../MainTheme';

const styles = StyleSheet.create({

    MainContainer: {
        ...mainTheme.MainContainer,
        justifyContent: 'center',
        flex: 1,
        backgroundColor: 'white',
    },
    searchBarContainer: {
        backgroundColor: 'white',
        padding: 20,
        marginBottom: '2%'
    },
    searchBarInputContainer: {
        backgroundColor: '#d3d3d3'        
    },
    loadingIcon: {
        paddingBottom: 0,
        backgroundColor: 'white',
    },
    item: {
        padding: 10,
        flexDirection: 'row',
        // borderColor: MainTheme.Colors.red,
        //borderWidth: 1,
        // borderRadius: 20,
        margin: 5
    },
    itemID: {
        padding: 10,
        fontSize: 24,
        textAlign: 'left',
        textAlignVertical: 'center',
        marginLeft: "5%",
        color: 'black',
        flex: 2
    },
    itemInfo: {
        padding: 10,
        fontSize: 24,
        textAlignVertical: 'center',
        flex:1
    },
    itemSeparator: {
        height: 2,
        width: "90%",
        backgroundColor: "#d3d3d3",
        marginLeft: "5%",
        margin: 15
    },
    itemLoading: {
        color: "#787878"
    },
    inputStyle: {
        backgroundColor: '#d3d3d3',
        color: 'black'
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

const ItemSeparator = () => (<View style={styles.itemSeparator} />);

const Item = ({item, onSelect, onDeleteRequest}) => (
    <View style={{flexDirection:'row', flex:1, alignContent:'space-around'}}>
        <View style={{...styles.item, flex:6, alignContent:'center'}}>
            <Text style={{...styles.itemID}} >{item.id}</Text>
            <Text style={{...styles.itemInfo}} >{new Date(item.date).toLocaleDateString()}</Text>
            <Text style={{...styles.itemInfo}} >{item.nb_analysis} analysis</Text>
        </View>
        <Button onPress={onSelect} title="Open" titleStyle={styles.buttonText} buttonStyle={styles.button} />
    </View>
);

class MeasuresList extends React.Component {
    lengthRequest = 10;
    endPosition = this.lengthRequest;

    constructor(props) {
        super(props);
        this.state = {
            search: '',
            list: 'loading...',
            isLoading: true,
        };
        this.device = this.props.device;
        this.canFetch = false;
    }

    async componentDidMount() {
        this.setState({
            list: await this.getPatientsList(this.state.search, 0, this.endPosition)
        });
    }

    async getPatientsList(search, listStart, listEnd) {
        this.setState({
            isLoading: true
        });

        let resultList = await this.device.getPatientsList(search, listStart, listEnd);
        let lengthRequested = listEnd - listStart;
        this.canFetch = resultList.length == lengthRequested;

        this.setState({
            isLoading: false
        });

        return resultList;
    }

    async onEndListReached() {
        if (!this.state.isLoading && this.canFetch) {
            let patientListTail = await this.getPatientsList(this.state.search, this.endPosition, this.endPosition + this.lengthRequest);
            if (patientListTail.length > 0) {
                this.setState({
                    list: this.state.list.concat(patientListTail)
                });
                this.endPosition = this.endPosition + patientListTail.length;
            }
        }
    }

    async updateSearchInput(search) {
        this.setState({ search: search });
        this.setState({ list: await this.getPatientsList(search, 0, this.endPosition) });
    }

    async openConfirmationDialog(id) {
        Alert.alert(
            id + " will be permanently deleted",
            "Are you sure ?",
            [
                { text: "Cancel" },
                { text: "OK", onPress: async () => await this.deletePatient(id) }
            ],
        );
    }

    async deletePatient(id) {
        await this.device.deletePatient(id);
        this.setState({
            list: await this.getPatientsList(this.state.search, 0, this.endPosition)
        });
    }

    renderItem(item) {
        if (item && item.id) {
            return (<Item 
                    item={item} 
                    onSelect={() => this.props.navigation.navigate('PatientFile', { itemID: item.id }) }
                    onDeleteRequest={() => this.openConfirmationDialog(item.id) }
                />);
        } else {
            return (<View><ActivityIndicator size='small' animating={this.state.isLoading} color={styles.itemLoading.color} /></View>);
        }
    }

    render() {
        const { search } = this.state;
        return (
            <View style={styles.MainContainer}>
                <View style={styles.loadingIcon}>
                    <ActivityIndicator size='small' animating={this.state.isLoading} color='#D60513' />
                </View>
                <SearchBar
                    value={search}
                    onChangeText={async (searchInput) => this.updateSearchInput(searchInput)}
                    placeholder='Type Here...'
                    searchIcon={{ size: 24 }}
                    round={true}
                    //lightTheme={true}
                    inputStyle={styles.inputStyle}
                    containerStyle={styles.searchBarContainer}
                    inputContainerStyle={styles.searchBarInputContainer}
                    testID='searchBar'
                />
                <FlatList
                    data={this.state.list}
                    ItemSeparatorComponent={ItemSeparator}
                    renderItem={({ item }) => this.renderItem(item)}
                    keyExtractor={(item, index) => index.toString()}
                    onEndReachedThreshold={0.1}
                    onEndReached={() => this.onEndListReached()}
                    testID='flatList'
                />
            </View>
        );
    }
}


export default MeasuresList;
