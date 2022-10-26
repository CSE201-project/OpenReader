import React from 'react';
import {
    Grid,
    TableContainer,
    Table,
    TableBody,
    TableRow,
    TableCell,
    Switch,
    FormControlLabel
} from '@material-ui/core';

const emptyDeviceInfos = {
    name: null,
    version: null,
    temperature_sensors: [
       {
           temperature: null,
           humidity: null
       },
       {
            temperature: null,
            humidity: null
        }
    ],
    light_sensors: [
        {
            visible: null,
            uv: null
        },
        {
            visible: null,
            uv: null
        }
    ],
    battery: {
        status: null,
        charge: null
    }
};


export default function DeviceInfosWidget(props) {
    
    const [deviceInfos, setDeviceInfos] = React.useState(emptyDeviceInfos);
    const [refreshInterval, setRefreshInterval] = React.useState(null);

    const refreshDeviceInfos = () => {
        return setInterval(async () => {
            try {
                let info = await props.device.info();
                setDeviceInfos(info ?? emptyDeviceInfos);
            }
            catch (err) {
                let dummy = emptyDeviceInfos;
                dummy.name = "Error";
                dummy.version = err;
                setDeviceInfos(dummy);
            }
        }, 1000);
    };

    const handleSwitchChange = (event) => {
        if (event.target.checked) {
            setRefreshInterval(refreshDeviceInfos);
        }
        else {
            clearInterval(refreshInterval);
            setRefreshInterval(null);
        }
    };

    return (
        <Grid container style={localStyle.container}>
            <Grid item xs={6}>Device Infos</Grid>
            <Grid item xs={6}>
                <FormControlLabel label="Auto-refresh"
                    control={<Switch onChange={handleSwitchChange}></Switch>}
                />
            </Grid>
            <Grid item xs={12}>
                <TableContainer>
                    <Table>
                        <TableBody>
                            <TableRow>
                                <TableCell>Name</TableCell>
                                <TableCell align="right" colSpan={2}>{deviceInfos.name}</TableCell>
                            </TableRow>
                            <TableRow>
                                <TableCell>Version</TableCell>
                                <TableCell align="right" colSpan={2}>{deviceInfos?.version}</TableCell>
                            </TableRow>
                            <TableRow>
                                <TableCell>Temperature</TableCell>
                                <TableCell align="right">{deviceInfos.temperature_sensors[0].temperature ?? null}</TableCell>
                                <TableCell align="right">{deviceInfos.temperature_sensors[1].temperature ?? null}</TableCell>
                            </TableRow>
                            <TableRow>
                                <TableCell>Humidity</TableCell>
                                <TableCell align="right">{deviceInfos.temperature_sensors[0].humidity ?? null}</TableCell>
                                <TableCell align="right">{deviceInfos.temperature_sensors[1].humidity ?? null}</TableCell>
                            </TableRow>
                            <TableRow>
                                <TableCell>UV sensors</TableCell>
                                <TableCell align="right">{deviceInfos.light_sensors[0].uv ?? null}</TableCell>
                                <TableCell align="right">{deviceInfos.light_sensors[1].uv ?? null}</TableCell>
                            </TableRow>
                            <TableRow>
                                <TableCell>ALS sensors</TableCell>
                                <TableCell align="right">{deviceInfos.light_sensors[0].visible ?? null}</TableCell>
                                <TableCell align="right">{deviceInfos.light_sensors[1].visible ?? null}</TableCell>
                            </TableRow>
                            <TableRow>
                                <TableCell>Battery</TableCell>
                                <TableCell align="right">{deviceInfos.battery.status ?? null}</TableCell>
                                <TableCell align="right">{deviceInfos.battery.charge ?? null}</TableCell>
                            </TableRow>
                        </TableBody>
                    </Table>
                </TableContainer>
            </Grid>
        </Grid>
    );
}

const localStyle = {
    container: {
        marginTop : 20
    }
}