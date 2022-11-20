'use strict';

import RNFetchBlob from 'rn-fetch-blob';
import { checkPermissions, checkReadPermissions } from './Permissions';

async function downloadReportToDocuments(device, resultID) {
    const android = RNFetchBlob.android;

    if (!(await checkPermissions() && await checkReadPermissions())) {
        return;
    }

    let tmpUrl = device.getPdfUrl(resultID);
    console.debug('downloadReport : ', tmpUrl);

    let dirs = RNFetchBlob.fs.dirs;
    let destFolder = dirs.DownloadDir;

    let filenameDest = 'DiagInLight_Report_' + resultID + '.pdf';
    let destPath = destFolder + '/' + filenameDest;
    
    console.debug('destPath : ', destPath);
    
    let documentsDir = destFolder;
    try {
        let documentsDir = '/storage/emulated/0/Documents';
        if (!(await RNFetchBlob.fs.exists(documentsDir))) {
            await RNFetchBlob.fs.mkdir(documentsDir);
        }

        documentsDir = '/storage/emulated/0/Documents/DiagInLight';
        if (!(await RNFetchBlob.fs.exists(documentsDir))) {
            await RNFetchBlob.fs.mkdir(documentsDir);
        }
    } catch(error) {
        console.error("Could not create dirs ", error);
        documentsDir = destFolder;
    }

    try {
        let res = await RNFetchBlob.config({
            overwrite: true,
            fileCache: false,
            trusty: true,
            path: destPath
        }).fetch('GET', tmpUrl);

        if (documentsDir !== destFolder) {
            //let fileInDocFolder = documentsDir + '/' + filenameDest;
            destPath = documentsDir + '/' + filenameDest;
            let mvres = await RNFetchBlob.fs.mv(res.path(), destPath);
        }
        let resScan = await RNFetchBlob.fs.scanFile([{ path: destPath, mime: 'application/pdf' }]);
        
        await android.actionViewIntent(destPath, 'application/pdf');
    }
    catch (err) {
        console.error("download manager fail : ", err);
    }
}

export { downloadReportToDocuments };