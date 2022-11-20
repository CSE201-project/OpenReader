import React from 'react';
import MeasuresList from '../Components/MeasuresList';

export default function BrowseMeasuresScreen({navigation, route}) {
    return (
        <MeasuresList 
            navigation={navigation} 
            device={route.params.device} 
        />
    );
}