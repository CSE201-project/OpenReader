'use strict';
import {
    PermissionsAndroid,
} from 'react-native';

async function checkPermissions() {
    try {
        const granted = await PermissionsAndroid.request(
          PermissionsAndroid.PERMISSIONS.WRITE_EXTERNAL_STORAGE,
          {
            title: "Writing to disk permission",
            message:
              "Cool Photo App needs access to your storage " +
              "so you can save the PDF locally.",
            buttonNeutral: "Ask Me Later",
            buttonNegative: "Cancel",
            buttonPositive: "OK"
          }
        );
        if (granted === PermissionsAndroid.RESULTS.GRANTED) {
          console.log("You can use the storage");
          return true;
        } else {
          console.log("Storage permission denied");
        }
    } catch (err) {
        console.warn(err);
    }
    return false;
}

async function checkReadPermissions() {
    try {
        const granted = await PermissionsAndroid.request(
          PermissionsAndroid.PERMISSIONS.READ_EXTERNAL_STORAGE,
          {
            title: "Reading from disk permission",
            message:
              "Cool Photo App needs access to your storage " +
              "so you can save the PDF locally.",
            buttonNeutral: "Ask Me Later",
            buttonNegative: "Cancel",
            buttonPositive: "OK"
          }
        );
        if (granted === PermissionsAndroid.RESULTS.GRANTED) {
          console.log("You can use the storage");
          return true;
        } else {
          console.log("Storage permission denied");
        }
    } catch (err) {
        console.warn(err);
    }
    return false;
}

export {checkPermissions, checkReadPermissions};