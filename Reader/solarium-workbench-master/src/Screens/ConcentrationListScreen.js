import React from 'react';
import { Link } from 'react-router-dom';
import {
  Container,
  Typography,
  Button,
  List,
  ListItem,
  ListItemText,
  ListSubheader,
  Grid,
  AppBar,
  IconButton,
} from '@material-ui/core';
import TuneIcon from '@material-ui/icons/Tune';
import { red } from '@material-ui/core/colors';
import { keys } from '@material-ui/core/styles/createBreakpoints';

class ConcentrationListScreen extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      openConfirmDialog: false
    };
    this.currentStudy = this.props.location.studyObject;
  }

  handleClose = () => {
    this.setState({ openConfirmDialog: false });
  }

  handleOpen = () => {
    this.setState({ openConfirmDialog: true });
  }

  renderListItems() {
    console.log('LIST:', this.currentStudy.concentrationList);
    return (
      //Object.keys(this.currentStudy.concentrationList).map((item, index) => (
      [...this.currentStudy.concentrationList.keys()].map((currentConcentration, index) => (
        <ListItem
          key={index}
          button
          divider
          component={Link}
          to={{
            pathname: '/study-snapshot',
            studyObject: this.currentStudy,
            concentrationName: currentConcentration,
          }}
        >
          <Grid container>
            <Grid item xs={8}>
              <Button variant='contained' style={styles.button}>
                <ListItemText primary={currentConcentration + ' ' + this.currentStudy.unit} />
              </Button>
            </Grid>
            <Grid item xs={4}>
              <ListItemText primary={`${this.currentStudy.getSnapshotNumber(currentConcentration)} image(s)`} />
            </Grid>
          </Grid>
        </ListItem>
      ))
    )
  }

  renderAppBar() {
    return (
      <AppBar position='sticky' style={styles.appBar}>
        <Grid container alignItems='center'>
          <Grid item xs={4}>
            <Button
              size='large'
              variant='contained'
              component={Link}
              style={styles.appBarButton}
              to={{
                pathname: '/init-study',
                studyObject: this.currentStudy,
              }}
            >
              Back
            </Button>
          </Grid>
          <Grid item xs={4}>
            <Typography style={styles.title} variant="h4">
              {this.currentStudy.name}
            </Typography>
            <Typography variant='subtitle2' gutterBottom>
              {`Connected to ${this.props.device.Name}`}
            </Typography>
          </Grid>
          <Grid item xs={4}>
            <IconButton
              style={styles.paramsButton}
              component={Link}
              to={{
                pathname: '/calibration-study',
                caller: '/concentration-list',
                studyObject: this.currentStudy,
                concentrationName: ''
              }}
            >
              <TuneIcon />
            </IconButton>
          </Grid>
        </Grid>
      </AppBar>
    );
  }

  render() {
    return (
      <Container>
        {this.renderAppBar()}
        <List subheader={<ListSubheader>Select a concentration to start measure</ListSubheader>}>
          {this.renderListItems()}
        </List>
      </Container>
    );
  }
}

const styles = {
  title: {
    fontFamily: 'revert',
    fontWeight: 'bold',
    color: 'white'
  },
  appBar: {
    backgroundColor: red[900]
  },
  paramsButton: {
    backgroundColor: red[900],
    color: 'white'
  },
  appBarButton: {
    backgroundColor: 'white',
    fontFamily: 'revert'
  },
  listItem: {
    backgroundColor: red[200]
  },
  button: {
    textTransform: 'none',
    backgroundColor: red[900],
    color: 'white',
    fontFamily: 'revert',
    marginLeft: 150
  }
}

export default ConcentrationListScreen;