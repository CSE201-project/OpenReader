function [maxControlLine]=findControlLine_calibParams(lightningCondition, channel, croppedImagePath, padding, referenceNumberOfPixels)

    [currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
    execPath = strcat(currentfunctionpath,'\PicaRunner\PicaRunnerMatlab.exe');
    
    paddingStr = num2str(padding);
    referenceNumberOfPixelsStr = num2str(referenceNumberOfPixels);

    commandLine = ['"',execPath, '" --findCL "', lightningCondition, '" "', channel, '" "', croppedImagePath, '" ', paddingStr, ' ', referenceNumberOfPixelsStr];
    [~, picaOutput] = system(commandLine);
    
    maxControlLine = str2double(picaOutput);
end 