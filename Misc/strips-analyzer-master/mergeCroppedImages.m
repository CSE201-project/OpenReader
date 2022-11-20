function mergeCroppedImages(rawImagesPath,studyName,infosCroppedImages, fileNameSuffix)


% mergeCroppedImages :
%   
%   Version of the function saveCroppedImages adapted to the third version
%   of stripsAnalysis. This function save images previously cropped on one
%   figure. The images appear ordered by concentration with the RGB images
%   on the top and the R images on the bottom.
%
%
% INPUT PARAMETERS : 
%   
%   rawImagesPath : path leading to the folder with the images that are being 
%   analysed.
%   
%   studyName : number of the essay currently analysed.
%   
%   INFOSCROPPEDIMAGES : structure with main information on cropped images 
%   (eg name).
%
%
% WRITTEN BY CVE
% V4 created by TMY on 17/10/2020  


    %cd(strcat(rawImagesPath,'Cropped'))
    croppedImagesPath=strcat(rawImagesPath,'/Cropped/');
    
    % Create the figure that will be saved.
    cf=figure('Name',strcat(studyName,' : Cropped images.png'),'Visible','off');
     
    wbf = waitbar(0, 'Merging cropped images...', 'Name', 'Merging Cropped Images');
    
    % Initialization
    concentrationListNotOrdered=zeros((length(infosCroppedImages)/2),2);
    
    halfLengthInfos = length(infosCroppedImages)/2;
    
    % To order the images by concentration.
    for i=1:halfLengthInfos
        [~,~,concentration,~] = parseFileName(infosCroppedImages(i).name);
        concentrationListNotOrdered(i,1) = concentration;
    end

    [~,concentrationIndice]=sort(concentrationListNotOrdered,1);
    concentrationIndice(:,2)=concentrationIndice(:,1)+(length(infosCroppedImages)/2);
    
    
    % Plot RGB images
    for i=1:halfLengthInfos
        
        waitbar(i/halfLengthInfos * 0.5, wbf, ['Remaining ' num2str(halfLengthInfos * 2 - i)]);
        
        j=concentrationIndice(i);
        Nom=infosCroppedImages(j).name;
        [~,concentrationText,~,~] = parseFileName(Nom);
        C = concentrationText(1:length(concentrationText)-4); % Display just the concentration (without the 'ngmL')
        
        currentAx = subplot(2,halfLengthInfos,i, 'Parent', cf);
        imshow(strcat(croppedImagesPath,Nom), 'Parent', currentAx);
        title(currentAx, C,'FontSize',5);
    end
    
    
    % Plot R images
    for i=1:halfLengthInfos
        
        waitbar(i/halfLengthInfos * 0.5 + 0.5, wbf, ['Remaining ' num2str(halfLengthInfos - i)]);
        
        j=concentrationIndice(i+halfLengthInfos);
        Nom=infosCroppedImages(j).name;
        [~,concentrationText,~,~] = parseFileName(Nom);
        C = concentrationText(1:length(concentrationText)-4); % Dispay just the concentration (without the 'ngmL')
        currentAx = subplot(2,halfLengthInfos,i+halfLengthInfos, 'Parent', cf);
        imshow(strcat(croppedImagesPath,Nom), 'Parent', currentAx);
        title(currentAx, C,'FontSize',5);
    end

    close(wbf);
    
    % Save
    destFilePath=[rawImagesPath '/Results/Cropped images - ' studyName ' - ' fileNameSuffix '.png'];
    print(cf, destFilePath, '-dpng', '-r400');
    close(cf);
    
    
    %cd(rawImagesPath)

end