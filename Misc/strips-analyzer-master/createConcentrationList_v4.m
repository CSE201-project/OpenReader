function [concentrationlist]=createConcentrationList_v4(infosImages,N)

% CREATECONENTRATIONLIST_v4 :
%   
%   Version of the function creatConcentrationList adapted to the third 
%   version of stripsAnalysis. This function get the concentrations
%   used in the essay.
%
%
% INPUT PARAMETERS : 
%   
%   INFOSIMAGES : structure with main informations on the images.
%   
%   N : number of images to be analysed.
%
%
% OUTPUT PARAMETERS :
%
%   CONCENTRATIONLIST : list with the concentration used in the essay.
%
%
% WRITTEN BY CVE
% V4 created by TMY on 17/10/2020  

    
    %Initialization
    concentrationlist=[];
    
    
    for i=1:N
        
        % Get the concentration of the image.
        C = infosImages(i).FileInfo.Concentration;
        
        % Check if the concentration is already in the list.
        liste_logique=C==concentrationlist;
        infosImages(i).concentration=C;
        
        % If not, add it.
        if isempty(find(liste_logique,1))
            concentrationlist=[concentrationlist, C];
        end
        
    end
    concentrationlist = sort(concentrationlist);
    
end