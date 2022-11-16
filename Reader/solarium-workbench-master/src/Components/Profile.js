import React from 'react';
import { Chart, Line } from 'react-chartjs-2';

class Profile extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
        this.profile = props.profile;
        this.color = props.color;
        Chart.defaults.global.legend.display = false;
    }

    getLabels() {
        let profileLength = this.profile.length;
        let labels = [];
        for (let i = 0; i < profileLength; i++) {
            labels[i] = i;
        }
        return labels;
    }

    getColor() {
        if (this.color === 'red')
            return 'rgba(180,5,19,1)';
        else if (this.color === 'blue')
            return 'rgb(13,71,161,1)';
        else if (this.color === 'green')
            return 'rgb(27,94,32,1)';
    }

    render() {
        const data = {
            labels: this.getLabels(),
            datasets: [
                {
                    label: 'Value',
                    fill: false,
                    lineTension: 0.1,
                    borderColor: this.getColor(),
                    pointRadius: 1,
                    data: this.profile,
                }
            ],
        };
        const options = {
            scales: {
                yAxes: [{
                    scaleLabel: {
                        display: true,
                        labelString: 'Normalized pixel value'
                    }
                }],
                xAxes: [{
                    scaleLabel: {
                        display: true,
                        labelString: 'Pixels'
                    }
                }],
            }
        }
        return (
            <div>
                <Line id={this.color} data={data} options={options} />
            </div>
        );
    }
}

export default Profile;