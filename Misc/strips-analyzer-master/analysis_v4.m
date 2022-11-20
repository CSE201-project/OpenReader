function [infosImages]=analysis_v4(j,infosImages,generalInfos,rawImagesPath,filenameExcel,calibrationParams,displayedImageAxis,axlist, elapsedTimeUI)

% ANALYSIS_v4 :
%   
%   Version of the function analysis adapted to the third version
%   of stripsAnalysis. This function analyses the serie.
%
%
% INPUT PARAMETERS : 
%   
%   J : iteration.
%   
%   INFOSIMAGES : structure with main information on each images.
%
%   GENERALINFOS : strcuture with general information to run the code.
%
%   NEWPATH : path to the images to be analyzed.
%
%   FILENAMEEXCEL : name of the Exel file.
%
%   INFORMATION_TEXT : to display information on the current analysis. 
%
%   FIG_TEXT : to display the name of the current image. 
%
%   ZMIN : minimum of the control line research area.
%
%   ZMAX : maximum of the control line research area.
%
%   C2C : list with all C2C for each test line.
%
%   LINESNUMBER : number of test(s) line(s) + control line.
%
%   LINELENGTH_SLIDER : length of the analysis rectangle.
%
%   LINEWIDTH_SLIDER : width of the analysis rectangle.
%
%   MASK_SLIDER : size of the mask.
%
%   AXLIST : list of the axis used on the interface.
%
%
% OUTPUT PARAMETERS
%
%   INFOSIMAGES : structure with main information on each images.
%
%
% WRITTEN BY CVE
% V4 created by TMY on 17/10/2020  

    % Display image cropped
    currentImage = infosImages(j);
    displayedImage=currentImage.rCroppedImage;
    displayedImageAxis.Position = [0.3 0 0.35 0.95];
    imagesc(displayedImageAxis,displayedImage);
    colormap gray

    timerVal = tic;

    lengthTLPx = floor(calibrationParams.referenceNumberOfPixels * calibrationParams.LengthTLMm);
    distanceCLtoTLPx = floor(calibrationParams.referenceNumberOfPixels * calibrationParams.DistanceCLtoTLMm);
    
    [h, w] = size(displayedImage);
    maxCLSearchArea = floor(h/3);
    minCLSearchArea = 10;
    
    % Find control line
    infosImages(j).maxControlLine = findControlLine_calibParams(generalInfos.lightningCondition, generalInfos.channel, currentImage.rgbCroppedPath,calibrationParams.AnalysisRectangle.Padding,calibrationParams.referenceNumberOfPixels);
    %Find position of last test line
    positionLastTestLine = infosImages(j).maxControlLine + distanceCLtoTLPx + floor(0.2 * calibrationParams.referenceNumberOfPixels);
    infosImages = cutProfils_v4(infosImages,j,minCLSearchArea,maxCLSearchArea, calibrationParams.AnalysisRectangle.Padding, floor(lengthTLPx/2), generalInfos.lightningCondition, positionLastTestLine, calibrationParams.referenceNumberOfPixels, calibrationParams.RegionOfInterestLength);
    
    elapsed = toc(timerVal);
    disp(['Find control line : ' num2str(elapsed) ' s']);

    % Display rectangle and get value of the rectangles
    drawRectangleOnDisplayedImage(currentImage, infosImages(j).maxControlLine, calibrationParams, displayedImageAxis, 2, 'clearPreviousRectangles');
    
    % Analysis
    timerVal = tic;
    
    elapsed = toc(timerVal);
    disp(['Analysis : ' num2str(elapsed) ' s']);
    
end
