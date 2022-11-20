function [infosImages,infosCroppedImages,hasGeneratedImages,listOfBadCropped]=checkCroppedImages_v4(rawImagesPath,infosImages,prototype,cassette,LEDType,colorChannel,numberOfPixels,focal)

% CHECKCROPPEDIMAGES_v4 :
%   
%   This function check if the images have already been 
%   cropped by comparing the number of images in the main folder with 
%   images to be analysed and the images in the Results folder if it 
%   exists. If it does not exist, the file is created and images are 
%   cropped.
%
%
% INPUT PARAMETERS : 
%   
%   rawImagesPath : path leading to the folder with the images that are being 
%   analysed.
%   
%   INFOSIMAGES : structure with main information on the images (eg name).
%
%   PROTOTYPE : serie of prototypes that have been used. Use to parameter
%   the cropping.
%   
%   CASSETTE : with or without. Use to parameter the cropping.
%
%   LEDType : UV/Visible
%
%   PIXELSSTUDIES : if working in the visible, chose the pixels to work on
%   (R,G,B).
%
%
% OUTPUT PARAMETERS :
%
%   INFOSIMAGES : strucure with main information on the images completed
%   with the information on cropped images.
%
%   INFOSCROPPEDIMAGES : strucure with main information on the cropped 
%   images.

    listOfBadCropped = [];

    colorChannelPrefix = colorChannel(1:1);
    shouldCropRGBFiles = true;
    shouldCropColorChannelFiles = true;

    % Information to the user
    wbf = waitbar(0, 'Checking cropped images...', 'Name', 'Cropping Images');
    
    %cd(rawImagesPath)
    rawImagesNumber=length(infosImages);
    croppedImagesPath=strcat(rawImagesPath, 'Cropped\');
    
    for i=1:rawImagesNumber
        infosImages(i).rgbCroppedPath = strcat(croppedImagesPath, 'RGB_', infosImages(i).name);
    end


    % Check if the Cropped folder exist in the current folder
    if exist(croppedImagesPath, 'dir')==7 
    
        rgbFilePattern = fullfile(croppedImagesPath,'RGB_*.png');
        colorChannelFilesPattern = fullfile(croppedImagesPath,[colorChannelPrefix '_*.png']);
        
        % Does it have all the RGB files ?
        rgbCroppedImages = dir(rgbFilePattern);
        if length(rgbCroppedImages) == rawImagesNumber
            
            waitbar(0.1, wbf, {'Cropped images found...', 'Loading RGB files...'})
            shouldCropRGBFiles = false;
            
            % loading rgb cropped files
            for i=1:rawImagesNumber
                
                % Load the rgb cropped image in the structure infosImages.
                Name=strcat(croppedImagesPath, '/', rgbCroppedImages(i).name);
                [rawImage, colorMap, alphaChannel] = imread(Name);
                infosImages(i).rgbCroppedImage=rawImage;
               
                waitbar(i/rawImagesNumber * 0.5, wbf, {'Loading RBG files...', ['Remaining ' num2str(rawImagesNumber - i)]});
            end
            
            % Does it have all the right color files ?
            colorChannelCroppedImages = dir(colorChannelFilesPattern);
            
            if length(colorChannelCroppedImages) == rawImagesNumber
                waitbar(0.5, wbf, {'Cropped images found...', ['Loading ' colorChannelPrefix ' channel files...']})
                shouldCropColorChannelFiles = false;
            
                % loading color channel cropped files
                for i=1:rawImagesNumber

                    % Load the rgb cropped image in the structure infosImages.
                    Name=strcat(croppedImagesPath, '/', colorChannelCroppedImages(i).name);
                    [rawImage, colorMap, alphaChannel] = imread(Name);
                    infosImages(i).rCroppedImage=rawImage;

                    waitbar(i/rawImagesNumber * 0.5 + 0.5, wbf, {['Loading ' colorChannelPrefix ' channel files...'], ['Remaining ' num2str(rawImagesNumber - i)]});
                end
            else
                delete(colorChannelFilesPattern);
            end
        else
            delete(rgbFilePattern);
            delete(colorChannelFilesPattern);
        end
    else
        % Create the folder.
        mkdir(croppedImagesPath);
    end
        
    if (shouldCropRGBFiles)
        
        waitbar(0.1, wbf, {'No Previous Cropped RGB images found...', 'Cropping RGB files...'})
        
        [currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
        execPath = strcat(currentfunctionpath,'\PicaRunner\PicaRunnerMatlab.exe');

        for i=1:rawImagesNumber

            % Get the image
            Name=infosImages(i).name;
            disp(Name);
            imagePath = strcat(rawImagesPath, Name);

            if strcmp(prototype,'ProtoV2')
                deviceType = '--V2';
            elseif strcmp(prototype,'ProtoV3')
                deviceType = '--V3';
            else
                deviceType = '--V1';
            end
            
            if strcmp(LEDType,'Visible')
                lightningCondition = '--Visible';
            else
                lightningCondition = '--UV';
            end

            commandLine = ['"',execPath, '" --crop "', imagePath, '" "', croppedImagesPath, '" ', num2str(numberOfPixels), ' "', cassette, '" "',focal,'" ', deviceType, ' ', lightningCondition];
            system(commandLine);

            infosImages(i).rgbCroppedImage=imread(infosImages(i).rgbCroppedPath);

            waitbar(i/rawImagesNumber * 0.5, wbf, {'Cropping RGB files...', ['Remaining ' num2str(rawImagesNumber - i)]});
        end
    end

    if (shouldCropColorChannelFiles)
        
        waitbar(0.1, wbf, {['No Previous Cropped ' colorChannelPrefix ' images found...'], ['Generating ' colorChannelPrefix ' channel files...']})
        
        for i=1:rawImagesNumber
            
            if strcmp (colorChannel,'Red')
                infosImages(i).rCroppedImage = infosImages(i).rgbCroppedImage(:,:,1);
            elseif strcmp (colorChannel,'Green')
                infosImages(i).rCroppedImage = infosImages(i).rgbCroppedImage(:,:,2);
            elseif strcmp (colorChannel,'Blue')
                infosImages(i).rCroppedImage = infosImages(i).rgbCroppedImage(:,:,3);
            end
            
            % Save the images in the "Cropped" folder
            imwrite(infosImages(i).rCroppedImage,strcat(croppedImagesPath, '/', colorChannelPrefix, '_', infosImages(i).name));
            
            waitbar(i/rawImagesNumber * 0.5 + 0.5, wbf, {['Generating ' colorChannelPrefix ' channel files...'],['Remaining ' num2str(rawImagesNumber - i)]});
        end
        
        dirRGB=dir(strcat(croppedImagesPath,'/RGB_*.png'));
        dirR=dir(strcat(croppedImagesPath,'/', colorChannelPrefix, '_*.png'));
        infosCroppedImages=cat(1, dirRGB, dirR);
        
        listOfBadCropped = checkCroppedImagesSize(infosImages, infosCroppedImages, cassette, numberOfPixels);
    end
        
    close(wbf);
    
    hasGeneratedImages = shouldCropRGBFiles || shouldCropColorChannelFiles;

    dirRGB=dir(strcat(croppedImagesPath,'/RGB_*.png'));
    dirR=dir(strcat(croppedImagesPath,'/', colorChannelPrefix, '_*.png'));
    infosCroppedImages=cat(1, dirRGB, dirR);
    
end