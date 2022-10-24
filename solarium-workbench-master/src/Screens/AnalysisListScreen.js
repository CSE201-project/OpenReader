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
    DialogTitle,
    DialogActions,
    IconButton
} from '@material-ui/core';
import { red } from '@material-ui/core/colors';
import { Delete } from '@material-ui/icons';

class AnalysisListScreen extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            list: "",
            openDialog: false
        };
        this.patientId = this.props.location.patientId
    }

    async componentDidMount() {
        this.setState({
            list: await this.props.device.getPatientAnalysis(this.patientId),
        });
    }

    openDialog(id) {
        this.setState({ openDialog: true });
        this.analysisId = id;
    }

    closeDialog = () => {
        this.setState({ openDialog: false });
    }

    getPatientMeasure(id) {
        var measureResults = this.state.list['measures'].filter(object => {
            return object.id === id;
        })
        return measureResults;
    }

    async deleteAnalysis(id) {
        await this.props.device.deleteAnalysis(id);
        this.setState({
            list: await this.props.device.getPatientAnalysis(this.patientId),
            openDialog: false
        });
    }

    confirmationDialog() {
        return (
            <Dialog open={this.state.openDialog} onClose={this.closeDialog}>
                <DialogTitle>Delete {this.analysisId}</DialogTitle>
                <DialogActions>
                    <Button onClick={this.closeDialog}>
                        Cancel
                    </Button>
                    <Button onClick={() => this.deleteAnalysis(this.analysisId)}>
                        Confirm
                    </Button>
                </DialogActions>
            </Dialog>
        );
    }

    renderListItems() {
        return (
            Object.keys(this.state.list['measures']).map((item) => (
                <ListItem
                    key={this.state.list['measures'][item].id}
                    button
                    divider
                >
                    <Grid container>
                        <Grid item xs={8}>
                            <Button
                                variant='contained'
                                style={styles.analysisButton}
                                fullWidth
                                component={Link}
                                to={{
                                    pathname: '/analysis-result',
                                    patientId: this.patientId,
                                    measures: this.getPatientMeasure(this.state.list['measures'][item].id)
                                }}
                            >
                                <ListItemText 
                                    primary={this.state.list['measures'][item].strip_type}
                                    secondary={new Date(this.state.list['measures'][item].analysis_date).toLocaleDateString()}
                                />
                            </Button>
                        </Grid>
                        <Grid item xs={4}>
                            <IconButton style={styles.deleteButton} onClick={() => this.openDialog(this.state.list['measures'][item].id)} >
                                <Delete/>
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
                            to="/patient-list"
                        >
                            Back
                        </Button>
                    </Grid>
                    <Grid item xs={8}>
                        <Typography style={styles.title} variant="h4">
                            {this.patientId}
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
                <List>
                    {this.state.list && this.state.list['measures'].length && this.renderListItems()}
                    {this.confirmationDialog()}
                </List>
            </Container>
        );
    }
}

export default AnalysisListScreen;

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
    analysisButton: {
        textTransform: 'none',
        backgroundColor: red[800],
        color: 'white',
      },
      deleteButton: {
        color: red[900],
        marginLeft: 60,
        marginTop: 10
      }    
}