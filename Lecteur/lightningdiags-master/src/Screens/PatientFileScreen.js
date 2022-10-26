import React from 'react';
import PatientFile from '../Components/PatientFile';

export default function PatientFileScreen({navigation, route}) {

   return (
      <PatientFile
          navigation={navigation} 
          device={route.params.device} 
          itemID={route.params.itemID} 
      />
   );
}