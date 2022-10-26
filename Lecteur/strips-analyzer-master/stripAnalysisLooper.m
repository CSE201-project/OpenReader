%% --- Get images and save preliminary information ---
%% Version 4.0
% Get the currentpath
[currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
addpath(currentfunctionpath)
cd(currentfunctionpath)

generalInfos.currentAppVersion = getCurrentVersion();

% Get the images
[~,rawImagesPath]=uigetfile('*.png','Select an image in the folder with images to analyze :');
cd(rawImagesPath);

% Preliminary questions
[meas,cassette,condition,nanoparticules,pixelsStudied]=dialogbox_v4;
linesNumber=4;

[prototype, focal, numberOfPixels]=determineDeviceInformations(rawImagesPath);
initialParams = [string(linesNumber);string(meas);string(prototype);string(cassette);string(condition);string(focal);string(nanoparticules);string(pixelsStudied)];


% Save the number of images
infosImages = dir('*.png');
generalInfos.N = length(infosImages);
for i=1:generalInfos.N
    [studyName, concentrationText, concentration, imageNumber]=parseFileName(infosImages(i).name);
    infosImages(i).FileInfo.StudyName = studyName;
    infosImages(i).FileInfo.ConcentrationText = concentrationText;
    infosImages(i).FileInfo.Concentration = concentration;
    infosImages(i).FileInfo.ImageNumber = imageNumber;
end

lenConcentrationText = length(infosImages(1).FileInfo.ConcentrationText);
concentrationUnit = infosImages(1).FileInfo.ConcentrationText(lenConcentrationText-3:lenConcentrationText);
initialParams = [initialParams;concentrationUnit];

% Move text files
textFilesPath = strcat(rawImagesPath, '/Textes');
if exist(textFilesPath, 'dir') ~= 7
    mkdir(textFilesPath);
end
textFiles = dir('*.txt');
if ~isempty(textFiles)
    movefile('*.txt', textFilesPath);    
end

% Get and save the name of the essay
studyName = infosImages(1).FileInfo.StudyName;
generalInfos.name=studyName;
generalInfos.lightningCondition=condition;
generalInfos.channel=pixelsStudied;

[h,m,s]=hms(datetime('now'));
h=num2str(h);
m=num2str(m);
s=num2str(floor(s));
generalInfos.analysisDateTime = strcat(date,'_',h,'-',m,'-',s);

% Initialization
generalInfos.sumC2C1=0;
generalInfos.sumC2C2=0;
generalInfos.sumC2C3=0;
generalInfos.sumC2C4=0; 
generalInfos.compteur=1;

% create the 'Results' folder in rawImagesPath
cd(rawImagesPath)
[status,msg,msgID]=mkdir('Results');
[status,msg,msgID]=mkdir('Results/Loops');

%% --- Save cropped images and profils ---

% Check if the images have already been cropped. If not, crop the images and save them
[infosImages,infosCroppedImages,hasGeneratedImages, listOfBadCropped]=checkCroppedImages_v4(rawImagesPath,infosImages,prototype,cassette,condition,pixelsStudied,numberOfPixels,focal);
if length(listOfBadCropped) > 0
    disp("");
    disp("----------------------------------------------------");
    disp("Problem during cropping, some images don't have right width");
    disp("----------------------------------------------------");
    for i=1:length(listOfBadCropped)
        disp(listOfBadCropped(i));
    end
    error("Please check these images before re-run the application");
end

% Save cropped images on the same pdf
if (hasGeneratedImages)
    mergeCroppedImages(rawImagesPath,studyName,infosCroppedImages, ['RGB_And_' pixelsStudied]);
end
 

infosImages = orderImagesByConcentration(infosImages, meas);

% Save rough profils on the same graph
mergeRawImageProfiles(strcat(rawImagesPath, '/Results/Loops'),studyName,infosImages, generalInfos.analysisDateTime, condition);


%% --- Initialize the Excel file ---

% Get the concentrations and save them in a list
[concentrationList]=createConcentrationList_v4(infosImages,generalInfos.N); 



cd(rawImagesPath)

%% --- Get the images with the highest/lowest concentration ---

% Get the maximal concentration, then the indice of those concentrations and save them in a list
[extremConcentrationList]=concentrationCalibration_v4(infosImages,concentrationList,meas);


%% --- Interface ---

currentAppVersion = getCurrentVersion();

if strcmp(meas,'Multiplex')
    [currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
    patternFilePath = strcat(rawImagesPath, 'pattern.txt');
    if ~isfile(patternFilePath)
        copyfile(strcat(currentfunctionpath, '\pattern.txt'), patternFilePath);
    end
    graphicInterfaceMultiplexLooper(rawImagesPath,studyName,infosImages,generalInfos,initialParams, numberOfPixels, patternFilePath)
else
    graphicInterfaceLooper(rawImagesPath,studyName,infosImages,generalInfos,initialParams,concentrationList, extremConcentrationList,linesNumber, numberOfPixels)
end