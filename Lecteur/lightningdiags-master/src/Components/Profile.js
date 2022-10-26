import React from 'react';
import {
    View,
    Dimensions
} from 'react-native';
import { LineChart } from "react-native-chart-kit";

class Profile extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
        this.profile = props.profile;
    }

    getLabels() {
        let profileLength = this.profile.length;
        let xAxisPos = 0;
        let xAxisInterval = 100;
        let labels = [];
        for (let i = 0; i < profileLength; i++) {
            if (i % 100 === 0) {
                labels[i] = xAxisPos;
                xAxisPos += xAxisInterval;
            } else {
                labels[i] = '';
            }
        }
        return labels;
    }

    render() {
        return (
            <View>
                <LineChart
                    data={{
                        labels: this.getLabels(),
                        datasets: [
                            {
                                data: this.profile
                            }
                        ]
                    }}
                    width={Dimensions.get("window").width}
                    height={250}
                    yAxisInterval={100}
                    chartConfig={{
                        backgroundColor: "white",
                        backgroundGradientFrom: "white",
                        backgroundGradientTo: "white",
                        decimalPlaces: 2,
                        color: (opacity = 0) => `rgba(0, 0, 0, ${opacity})`,
                        labelColor: (opacity = 1) => `rgba(0, 0, 0, ${opacity})`,
                        style: {
                            borderRadius: 16
                        },
                        propsForDots: {
                            r: "0",
                            strokeWidth: "2",
                            stroke: "black"
                        }
                    }}
                    bezier
                    style={{
                        marginLeft: -15,
                        borderRadius: 0,
                    }}
                />
            </View>

        );
    }
}

export default Profile;