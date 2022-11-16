import React from 'react';
import { Link } from 'react-router-dom';
import {
    Container,
    Typography,
    AppBar,
    Grid,
    Button,
    Card,
    CardMedia,
    CircularProgress
} from '@material-ui/core';
import { red } from '@material-ui/core/colors';

import { blobToBase64 } from '../Services/Converter';

class AnalysisResultScreen extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            image: '',
            isLoading: false
        };
        this.patientId = this.props.location.patientId;
        this.measure = this.props.location.measures[0];
    }

    async takePicture() {
        this.setState({
            openDialog: true,
            isLoading: true
        });
        try {
            let pictureData = await this.props.device.getFile(this.measure.id, "full");
            let image = await blobToBase64(pictureData);
            this.setState({ image: image });
        } catch (e) {
            console.error('Could not load image from device : ', e);
        }
        this.setState({ isLoading: false });
    };

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
                            to={{
                                pathname: '/analysis-list',
                                patientId: this.patientId
                            }}
                        >
                            Back
                        </Button>
                    </Grid>
                    <Grid item xs={8}>
                        <Typography style={styles.title} variant="h4">
                            {this.patientId} results
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
        let date = new Date(this.measure['analysis_date']).toLocaleDateString();
        return (
            <Container>
                {this.renderAppBar()}
                <Grid container>
                    <Grid item xs={4}>
                        <Typography style={styles.itemTitle}>Type</Typography>
                        <Typography style={styles.item}>{this.measure['strip_type']}</Typography>
                    </Grid>
                    <Grid item xs={4}>
                        <Typography style={styles.itemTitle}>Date</Typography>
                        <Typography style={styles.item}>{date}</Typography>
                    </Grid>
                    <Grid item xs={4}>
                        <Typography style={styles.itemTitle}>Result</Typography>
                        <Typography style={styles.item}>{this.measure['result']}</Typography>
                    </Grid>
                </Grid>
                <Button
                    variant='contained'
                    style={this.state.isLoading ? styles.buttonLoading : styles.button}
                    onClick={async () => await this.takePicture()}
                >
                    Show Picture
                </Button>
                {this.state.isLoading && <CircularProgress size={24} style={styles.loadingIcon} />}
                <Card style={styles.image} >
                    <CardMedia component="img" src={this.state.image} />
                </Card>
            </Container>
        );
    }
}

export default AnalysisResultScreen;

const styles = {
    button: {
        color: 'white',
        backgroundColor: red[900],
        fontFamily: 'revert',
        margin: 10
    },
    buttonLoading: {
        color: red[900],
        backgroundColor: red[900],
        fontFamily: 'revert',
        margin: 10
    },
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
    loadingIcon: {
        color: 'white',
        position: 'absolute',
        marginTop: 15,
        marginLeft: -90
    },
    itemTitle: {
        fontSize: 25,
        fontWeight: 'bold'
    },
    item: {
        fontSize: 27,
        fontWeight: 'bold',
        color: red[900]
    },
    image: {
        width: "35%",
        justifyItems: 'center',
        marginLeft: "32%"
    }
}