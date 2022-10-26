function [extremConcentrationList]=concentrationCalibration_v4(infosImages,concentrationList,meas)

% CCONENTRATIONCALIBRATIONLIST_v4 :
%   
%   Version of the function creatConcentrationList adapted to the third 
%   version of stripsAnalysis. This function get the higher / lower 
%   concentration of the essay.
%
%
% INPUT PARAMETERS : 
%   
%   INFOSIMAGES : structure with main informations on the images.
%   
%   N : number of images to be analysed.
%
%   CONCENTRATIONLIST : list with the concentration used in the essay.
%
%   MEAS : kind of measure.
%
%
% OUTPUT PARAMETERS :
%
%   MAXCONCENTRATIONLIST : maximum of the concentration used.
%
%
% WRITTEN BY CVE
% V4 created by TMY on 17/10/2020  


    % Initialization
    extremConcentrationList=[];
    
    % Check which analysis is it to get the right concentrations.
    if strcmp(meas,'Sandwich')
            for i=length(concentrationList):-1:1
                concentrationForCalibration = concentrationList(i);
                if concentrationForCalibration < 1000
                    break;
                end
            end    
        maxNumberOfImages = 5;
        for i=1:length(infosImages)
            
            % Get the concentration of the image.            
            C = infosImages(i).FileInfo.Concentration;

            % If it is the maximum concentration, save the indice of the
            % image in the list.
            
            if C==concentrationForCalibration
                extremConcentrationList=[extremConcentrationList,i];
                if length(extremConcentrationList) == maxNumberOfImages
                    break;
                end
            end

        end
        
    elseif strcmp (meas,'Competition/Inhibition')
        maxNumberOfImages = 5;

        for i=1:length(infosImages)

            % Get the concentration of the image.                
            C = infosImages(i).FileInfo.Concentration;

            % If it is the minimum concentration, save the indice of the
            % image in the list.
            if C==min(concentrationList)
                extremConcentrationList=[extremConcentrationList,i];
                if length(extremConcentrationList) == maxNumberOfImages
                    break;
                end
            end

        end
        
    end

end