% fix 'createExcelLoopSummary' is not found
[currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
addpath(currentfunctionpath)

% Get the loop files
[excelFile,loopFilesPath]=uigetfile('*.xlsx','Select a result file of a loop to rebuild the summary');
if isequal(excelFile, 0)
    return;
end

wbf = waitbar(0, 'Reading Infos...', 'Name', 'Rebuilding Loop Summary');

cd(loopFilesPath);

% Results_9547-3F9-118A_20210123-105543_Rectangle Padding25
% AnalysisResults_v4_FF170C1_22-Jan-2021_11-42-12_loop-Rectangle Padding46
k = strfind(excelFile, '_');
if (length(k) == 3)
   idxStudy = [ (k(1)+1) (k(2)-1) ];
   idxDateTime = [ (k(2)+1) (k(3)-1) ];
else
   idxStudy = [ (k(2)+1) (k(3)-1) ];
   idxDateTime = [ (k(3)+1) (k(5)-1) ];
end

% Fetching params
studyName = excelFile(idxStudy(1):idxStudy(2)); 
analysisDateTime = excelFile(idxDateTime(1):idxDateTime(2));

studyParams = readmatrix(strcat(loopFilesPath, excelFile), 'Sheet','Params','Range','B3:B27','UseExcel',ispc); 
[resultsPath,~,~] = fileparts(loopFilesPath(1:length(loopFilesPath)-1));
[rawImagesPath,~,~] = fileparts(resultsPath);

[excelFilePath,loopName,ext] = fileparts(excelFile);

loopPattern = loopName(1:length(loopName) - 4);
allResultFiles = dir(strcat(loopPattern, "*.xlsx"));

maxLoop = length(allResultFiles);
allResultingValues = {maxLoop};
allResultsFilePath = [""];

for i=1:maxLoop    
    [tokens, ~] = regexp(allResultFiles(i).name, "([a-zA-Z ]*)([0-9]{1,3}(\.)?(([0-9]{1,3}))?)\.xlsx$", "tokens");
    allResultFiles(i).paramName = tokens{1}{1};
    allResultFiles(i).paramValue = str2double(tokens{1}{2});  
end

filesTable = sortrows(struct2table(allResultFiles), 'paramValue');
paramName = filesTable.paramName{1};

for i=1:maxLoop
    
    waitbar(0.1 + ((i-1)/maxLoop*0.8), wbf, strcat("Reading file ", num2str(i), "/", num2str(maxLoop)));
    if strcmp(paramName,"Spot diameter")
        allResultingValues{i} = readcell(filesTable.name{i}, 'Sheet','Summary','Range','A7:E79','UseExcel',ispc);
    else
        allResultingValues{i} = readcell(filesTable.name{i}, 'Sheet','Summary','Range','A7:E84','UseExcel',ispc);    
    end
    allResultsFilePath(i) = strcat(string(loopFilesPath), string(filesTable.name{i}));
end

paramStartValue = filesTable.paramValue(1);
paramValueIncrement = filesTable.paramValue(2) - filesTable.paramValue(1);

waitbar(0.9, wbf, 'Building summary');

if strcmp(paramName, "Spot diameter")
    paramList = [ "Spot diameter" studyParams(3)];
    nbSpotType = 6;
    
    createMultiplexLoopSummary(allResultsFilePath, allResultingValues, paramList, paramName, maxLoop, paramStartValue, paramValueIncrement, rawImagesPath, studyName, analysisDateTime, nbSpotType);
else
    linesNumber = studyParams(1);
    paramList = [ "Rectangle Width" studyParams(25); "Rectangle Padding" studyParams(24); "Mask Size" studyParams(21)];
    
    createExcelLoopSummary(allResultsFilePath, allResultingValues, paramList, paramName, maxLoop, paramStartValue, paramValueIncrement, rawImagesPath, studyName, analysisDateTime, linesNumber)
end
close(wbf);