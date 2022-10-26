import React from 'react';
import {render, fireEvent} from 'react-native-testing-library';

import MeasuresList from '../../src/Components/MeasuresList';
const largePatientList = require('../assets/patients_list_large.json');
const smallPatientList = require('../assets/patients_list_small.json');
const onePatientList = require('../assets/patients_list_one.json');

class DummySolariumDevice {
    async getPatientList(search, start, end) {
        try {
            return largePatientList;
        } catch (error) {
            console.log(error);
            return null;
        }
    }
}

describe('<MeasuresList />', function() {

    it('Should call getPatientList() with searchBar text', (done) => {
        const initialListStart = 0;
        const initialListEnd = 15;
        const messageText = 'patient23';

        const device = new DummySolariumDevice();
        device.getPatientList = jest.fn();
        const {getByTestId} = render(<MeasuresList device={device} />);

        fireEvent.changeText(getByTestId('searchBar'), messageText);
        expect(getByTestId('searchBar').props.value).toEqual(messageText);
        expect(device.getPatientList).toHaveBeenCalledWith(messageText, initialListStart, initialListEnd);
        done();
    });

    it('Should call next patient file screen', async (done) => {
        const flushPromises = () => new Promise(setImmediate);
        const device = new DummySolariumDevice();
        const navigation = {
            navigate: jest.fn(),
        }
        const {getByTestId} = render(<MeasuresList navigation={navigation} device={device} />);
        
        await flushPromises();

        fireEvent.press(getByTestId('touchableElementpatient1'));
        expect(navigation.navigate).toHaveBeenCalled();
        done();
    });
});