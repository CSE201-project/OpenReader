import React from 'react';
import { HashRouter as Router, Switch, Route } from 'react-router-dom';

import HomeScreen from '../Screens/HomeScreen';
import InitStudyScreen from '../Screens/InitStudyScreen'
import DeviceControlScreen from '../Screens/DeviceControlScreen';
import ConcentrationListScreen from '../Screens/ConcentrationListScreen';
import UpdaterScreen from '../Screens/UpdaterScreen';
import MeasureScreen from '../Screens/MeasureScreen';
import PatientListScreen from '../Screens/PatientListScreen';
import AnalysisListScreen from '../Screens/AnalysisListScreen';
import AnalysisResultScreen from '../Screens/AnalysisResultScreen';
import StudySnapshotScreen from "../Screens/StudySnapshotScreen";
import StudyCalibrationScreen from "../Screens/StudyCalibrationScreen";
import SequencesScreen from "../Screens/SequencesScreen";

import SolariumDevice from '../Services/SolariumDevice';
import './App.css';

class App extends React.Component {
  constructor(props) {
    super(props);
    this.device = new SolariumDevice('192.168.20.1');
  }

  render() {
    return (
      <div className="App">
        <Router>
          <Switch>
            <Route exact path='/' render={(props) => <HomeScreen {...props} device={this.device} />}  />
            <Route path='/device-control' render={(props) => <DeviceControlScreen {...props} device={this.device} />} />
            <Route path='/init-study' render={(props) => <InitStudyScreen {...props} device={this.device} />} />
            <Route path='/concentration-list' render={(props) => <ConcentrationListScreen {...props} device={this.device} />} />
            <Route path='/study-snapshot' render={(props) => <StudySnapshotScreen {...props} device={this.device} />} />
            <Route path='/updater' render={(props) => <UpdaterScreen {...props} device={this.device} />} />
            <Route path='/measure' render={(props) => <MeasureScreen {...props} device={this.device} />} />
            <Route path='/patient-list' render={(props) => <PatientListScreen {...props} device={this.device} />} />
            <Route path='/analysis-list' render={(props) => <AnalysisListScreen {...props} device={this.device} />} />
            <Route path='/analysis-result' render={(props) => <AnalysisResultScreen {...props} device={this.device} />} />
            <Route path='/calibration-study' render={(props) => <StudyCalibrationScreen {...props} device={this.device} />} />
            <Route path='/sequences' render={(props) => <SequencesScreen {...props} device={this.device} />} />            
          </Switch>
        </Router>
      </div>
    );
  }
}

export default App;
