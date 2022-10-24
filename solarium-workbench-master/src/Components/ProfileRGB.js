import React from 'react';
import { Chart, Line } from 'react-chartjs-2';

class ProfileRGB extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
        this.profiles = props.profiles;
        Chart.defaults.global.legend.display = false;
    }

    getLabels() {
        let profileLength = this.profiles.red.length;
        let labels = [];
        for (let i = 0; i < profileLength; i++) {
            labels[i] = i;
        }
        return labels;
    }

    render() {
        const data = {
            labels: this.getLabels(),
            datasets: [
                {
                    label: 'Red',
                    fill: false,
                    lineTension: 0.1,
                    borderColor: 'rgba(180,5,19,1)',
                    pointRadius: 1,
                    data: this.profiles.red,
                },
                {
                    label: 'Green',
                    fill: false,
                    lineTension: 0.1,
                    borderColor: 'rgb(27,94,32,1)',
                    pointRadius: 1,
                    data: this.profiles.green,
                },
                {
                    label: 'Blue',
                    fill: false,
                    lineTension: 0.1,
                    borderColor: 'rgb(13,71,161,1)',
                    pointRadius: 1,
                    data: this.profiles.blue,
                }
            ],
        };
        const options = {
            scales: {
                yAxes: [{
                    scaleLabel: {
                        display: true,
                        labelString: 'Normalized pixel value'
                    },
                    ticks: {
                        max: 1.0
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
                <Line id='profileRGBCanvas' data={data} options={options} />
            </div>
        );
    }
}

export default ProfileRGB;