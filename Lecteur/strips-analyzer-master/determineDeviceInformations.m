function [prototype, focal, numberOfPixels]=determineDeviceInformations(rawImagesPath)

    [currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
    execPath = strcat(currentfunctionpath,'\PicaRunner\PicaRunnerMatlab.exe');
    
    configDevicesPath=strcat(currentfunctionpath, '\Config devices.xlsx');
    
    rawImagesPath = eraseBetween(rawImagesPath,length(rawImagesPath),length(rawImagesPath));

    commandLine = ['"',execPath, '" --deviceInfos "', rawImagesPath, '" "',configDevicesPath,'"'];
    [~, picaOutput] = system(commandLine);
    
    resultsAsString = strsplit(picaOutput);
    prototypeCell = resultsAsString(1);
    focalCell = resultsAsString(2);
    
    prototype = prototypeCell{1, 1};
    focal = focalCell{1, 1};
    numberOfPixels = str2double(resultsAsString(3));

end 