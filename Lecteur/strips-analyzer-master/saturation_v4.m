function saturation_v4(cutProfil,filenameExcel,newpath)

% SATURATION_v4 :
%   
%   Version of the function saturation adapted to the third version
%   of stripsAnalysis. This function check if there are too many pixels at
%   255. If so, indicates it to the user in the final Excel file.
%
%
% INPUT PARAMETERS : 
%   
%   CUTPROFIL : profile of the image to be tested.
%
%   FILENAMEEXCEL : name of the Excel file that will be written.
%
%   NEWPATH : path in which are the photos.
%
%
% WRITTEN BY CVE
% V4 created by TMY on 17/10/2020  


    l=length(find(cutProfil>250));
    
    if l>10
        message=strcat('BE CAREFUL, TOO MANY SATURATED PIXELS');
        cd(strcat(newpath,'Results'))
        safe_writeMatrix(message,filenameExcel,1,'D2');
    end

    cd(newpath)

end 