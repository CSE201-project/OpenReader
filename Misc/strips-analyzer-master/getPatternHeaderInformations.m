function [nbGridLines, spacingH, spacingV, theoricSpotDiameter] = getPatternHeaderInformations(patternFilePath)
    
    [currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
    execPath = strcat(currentfunctionpath,'\PicaRunner\PicaRunnerMatlab.exe');

     commandLine = ['"',execPath, '" --getPatternHeader "', patternFilePath, '"'];
    [~, picaOutput] = system(commandLine);

    resultsAsString = strsplit(picaOutput);

    nbGridLines = str2double(resultsAsString(2));
    spacingH = str2double(resultsAsString(3));
    spacingV = str2double(resultsAsString(4));
    theoricSpotDiameter = str2double(resultsAsString(5));

end