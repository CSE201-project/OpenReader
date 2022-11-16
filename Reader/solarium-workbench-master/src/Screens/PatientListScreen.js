import React from 'react';
import { Link } from 'react-router-dom';
import {
  Container,
  Typography,
  AppBar,
  Grid,
  Button,
  List,
  ListItem,
  ListItemText,
  Dialog,
  DialogActions,
  DialogTitle,
  IconButton
} from '@material-ui/core';
import { red } from '@material-ui/core/colors';
import { Delete } from '@material-ui/icons';
import SearchBar from 'material-ui-search-bar';

class PatientListScreen extends React.Component {
  lengthRequest = 15;
  endPosition = this.lengthRequest;

  constructor(props) {
    super(props);
    this.state = {
      search: '',
      list: '',
      isLoading: true,
      openDialog: false
    };
    this.canFetch = false;
  }

  async componentDidMount() {
    this.setState({
      list: await this.getPatientsList(this.state.search, 0, this.endPosition)
    })
  }

  openDialog(id) {
    this.setState({ openDialog: true });
    this.patientId = id;
  }

  closeDialog = () => {
    this.setState({ openDialog: false });
  }

  async updateSearchInput(search) {
    this.setState({ search: search });
    this.setState({ list: await this.getPatientsList(search, 0, this.endPosition) });
  }

  async getPatientsList(search, listStart, listEnd) {
    this.setState({
      isLoading: true
    });

    let resultList = await this.props.device.getPatientsList(search, listStart, listEnd);
    let lengthRequested = listEnd - listStart;
    this.canFetch = resultList.length === lengthRequested;

    this.setState({
      isLoading: false
    });

    return resultList;
  }

  async deletePatient(id) {
    await this.props.device.deletePatient(id);
    this.setState({
      list: await this.getPatientsList(this.state.search, 0, this.endPosition),
      openDialog: false
    });
  }

  async loadMorePatients() {
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

  onReachedEnd = async (event) => {
    let scrollValue = event.target.scrollHeight - event.target.scrollTop;
    if (Math.round(scrollValue) === event.target.clientHeight) {
      await this.loadMorePatients();
    }
  }

  confirmationDialog() {
    return (
      <Dialog open={this.state.openDialog} onClose={this.closeDialog}>
        <DialogTitle>Delete {this.patientId}</DialogTitle>
        <DialogActions>
          <Button onClick={this.closeDialog}>
            Cancel
          </Button>
          <Button onClick={() => this.deletePatient(this.patientId)}>
            Confirm
          </Button>
        </DialogActions>
      </Dialog>
    );
  }

  renderListItems() {
    return (
      Object.keys(this.state.list).map((item) => (
        <ListItem
          key={this.state.list[item].id}
          button
          divider
        >
          <Grid container>
            <Grid item xs={4}>
              <Button
                variant='contained'
                style={styles.patientButton}
                fullWidth
                component={Link}
                to={{
                  pathname: '/analysis-list',
                  patientId: this.state.list[item].id,
                }}
              >
                <ListItemText
                  primary={this.state.list[item].id}
                  secondary={new Date(this.state.list[item].date).toLocaleDateString()}
                />
              </Button>
            </Grid>
            <Grid item xs={4}>
              <Typography style={styles.nbAnalysis}>
                {this.state.list[item].nb_analysis} analysis saved
                </Typography>
            </Grid>
            <Grid item xs={4}>
              <IconButton style={styles.deleteButton} onClick={() => this.openDialog(this.state.list[item].id)} >
                <Delete />
              </IconButton>
            </Grid>
          </Grid>
        </ListItem>
      ))
    )
  }

  renderAppBar() {
    return (
      <AppBar position='sticky' style={styles.appBar} >
        <Grid container alignItems='center'>
          <Grid item xs={2}>
            <Button
              size='large'
              variant='contained'
              style={styles.appBarButton}
              component={Link}
              to="/"
            >
              Back
            </Button>
          </Grid>
          <Grid item xs={8}>
            <Typography style={styles.title} variant="h4">
              Patient List
            </Typography>
            <Typography variant='subtitle2' gutterBottom>
              {`Connected to ${this.props.device.Name}`}
            </Typography>
          </Grid>
        </Grid>
      </AppBar>
    );
  }

  render() {
    return (
      <Container>
        {this.renderAppBar()}
        <SearchBar
          value={this.state.search}
          onChange={async (searchInput) => await this.updateSearchInput(searchInput)}
          style={styles.searchBar}
          onCancelSearch={async () => await this.updateSearchInput('')}
        >
        </SearchBar>
        <div style={styles.scrollBar} onScroll={this.onReachedEnd}>
          <List>
            {this.renderListItems()}
            {this.confirmationDialog()}
          </List>
        </div>
      </Container>
    );
  }
}

export default PatientListScreen;

const styles = {
  title: {
    fontFamily: 'revert',
    fontWeight: 'bold',
    color: 'white'
  },
  appBar: {
    backgroundColor: red[900]
  },
  appBarButton: {
    backgroundColor: 'white',
    fontFamily: 'revert'
  },
  searchBar: {
    margin: 10
  },
  patientButton: {
    textTransform: 'none',
    backgroundColor: red[800],
    color: 'white',
  },
  deleteButton: {
    color: red[900],
    marginLeft: 60,
    marginTop: 10
  },
  nbAnalysis: {
    marginLeft: "40%",
    marginTop: "10%"
  },
  scrollBar: {
    maxHeight: 400,
    overflowY: 'scroll'
  }
}