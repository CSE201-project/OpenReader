import React from 'react';
import { Link } from 'react-router-dom';
import {
  Container,
  Typography,
  Box,
  TextField,
  Button,
  Grid,
  CircularProgress
} from '@material-ui/core';
import { red, green, blue } from '@material-ui/core/colors';
import ParamFinderDialog from '../Components/ParamFinderDialog';

const isValidHostname = require('is-valid-hostname');
const dialog = window.dialog;

class HomeScreen extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      isLoading: false,
      deviceIP: props.device.Host,
      isConnected: false,
      isValidIP: true,
    };
  }

  componentDidMount() {
    if (this.props.device.IsConnected)
      this.connectToDevice();
  }

  handleDeviceIPInput = (event) => {
    let ip = event.target.value;
    this.setState({
      deviceIP: ip,
      isValidIP: isValidHostname(ip)
    });
    this.props.device.setHost(event.target.value);
  }

  async connectToDevice() {
    this.setState({ isLoading: true });
    let success = await this.props.device.connect();
    this.setState({ isLoading: false, isConnected: success });
    if (!success) {
      dialog.showErrorBox('Connection Error', 'Solarium device not found');
    }
  }

  render() {
    return (
      <Grid container>
        <Grid item xs={12}>
          <Typography style={styles.title} variant="h4" gutterBottom>
            Workbench
          </Typography>
        </Grid>
        <Grid item xs={12} alignItems="flex-end">
          <Box paddingBottom={5}>
            <TextField
              id="input-device-ip"
              label="Device IP"
              placeholder={`Ex: 192.168.20.1`}
              value={this.state.deviceIP}
              onChange={this.handleDeviceIPInput}
              error={!this.state.isValidIP}
              helperText={this.state.isValidIP ? "" : "Invalid Hostname"}
            />
            {this.state.isLoading && <CircularProgress size={24} style={styles.loadingIcon} />}
            <Button
              style={{
                 ...styles.button,
                backgroundColor: this.state.isConnected ? green[900] : blue[900],
                color: this.state.isLoading ? blue[900] : 'white'
              }}
              variant='contained'
              disabled={this.state.isLoading || !this.state.isValidIP}
              onClick={async () => await this.connectToDevice()}
            >
              Connect
          </Button>            
          {this.state.isConnected && <Typography>{`Connected to ${this.props.device.Name} (${this.props.device.solariumVersion})`}</Typography>}
          </Box>
        </Grid>
        <Grid item xs={4}>
          <Box padding={1}>
            <Button
              variant='contained'
              size='large'
              style={this.state.isConnected ? styles.button : styles.buttonDisabled}
              disabled={!this.state.isConnected}
              component={Link}
              to='/measure'
            >
              Patient Measure
            </Button>
          </Box>
          <Box padding={1}>
            <Button
              size='large'
              variant='contained'
              style={this.state.isConnected ? styles.button : styles.buttonDisabled}
              disabled={!this.state.isConnected}
              component={Link}
              to="/patient-list"
            >
              Browse
          </Button>
          </Box>
        </Grid>

        <Grid item xs={4}>
          <Box padding={1}>
            <Button
              style={this.state.isConnected ? styles.button : styles.buttonDisabled}
              size='large'
              variant='contained'
              disabled={!this.state.isConnected}
              component={Link}
              to={{
                pathname: '/device-control',
                caller: '/',
                studyObject: ''
              }}
            >
              Device Control
              </Button>
          </Box>
          <Box padding={1}>
            <ParamFinderDialog 
              device={this.props.device} 
              key={this.props.device.Name}
              style={this.state.isConnected ? styles.button : styles.buttonDisabled}/>
          </Box>
          <Box padding={1}>
            <Button
              style={styles.button}
              size='large'
              variant='contained'
              component={Link}
              to='/updater'>
              Updater
            </Button>
          </Box>


        </Grid>
        <Grid item xs={4}>
          <Box padding={1}>
            <Button
              style={this.state.isConnected ? styles.button : styles.buttonDisabled}
              size='large'
              variant='contained'
              disabled={!this.state.isConnected}
              component={Link}
              to='/init-study'
            >
              Analyse Use Test
          </Button>
          </Box>
          <Box padding={1}>
            <Button
              style={this.state.isConnected ? styles.button : styles.buttonDisabled}
              size='large'
              variant='contained'
              // disabled={!this.state.isConnected}
              component={Link}
              to='/sequences'
            >
              Sequences
          </Button>
          </Box>
        </Grid>

      </Grid>
    );
  }
}

export default HomeScreen;

const styles = {
  title: {
    fontFamily: 'fantasy',
    color: red[900]
  },
  loadingIcon: {
    color: red[900],
    padding: 5 //,
    // position: 'absolute',
    // marginTop: 5,
    // marginLeft: -65,
  },
  button: {
    backgroundColor: red[900],
    color: 'white',
    fontFamily: 'revert'
  },
  buttonDisabled: {
    backgroundColor: 'grey',
    color: 'white',
    fontFamily: 'revert'
  }
}
