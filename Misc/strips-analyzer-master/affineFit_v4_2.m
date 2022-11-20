function [affFit,affFitImage,CalculC_aff]=affineFit_v4_2(i,linesNumber,infosImages,C2C,yc,yb,yt,larg,longMask)

% AFFINNEFIT_V4 :
%   
%   Version of the function affineFit adapted to the third version
%   of stripsAnalysis. This function processes the affine fit 
%   analysis of the stripes.
%
% V4 crezated by TMY on 17/10/2020
%
% INPUT PARAMETERS : 
%   
%   I : iteration.
%
%   LINESNUMBER : number of lines (control line + number of test line).
%   
%   INFOSIMAGES : structure with main information on each images.
%   
%   C2C : list with C2C of all test(s) line(s).
%
%   YC : position of the control line
%
%   YB : position of the noise line
%
%   YT : position of the test line
%
%   LARG : length of the rectangle.
%
%   LONGMASK : length of the mask.
%
%
% OUTPUT PARAMETERS :
%
%   CALCULC_AFF : Calcul C
%
%   AFFFITIMAGE : profil fitted.
%
%   AFFFIT : trend.
%
%
% WRITTEN BY CVE


    % Model
    model_affine = @(b,x)(b(1)*x+b(2));
    
    % Options
    opts = statset('nlinfit');                      
    opts.RobustWgtFun = 'bisquare';
    
    % Initialization        
    b0=[10 20];
    midLongMask=floor(longMask/2);
    
    % Profil
    cutProfil=infosImages(i).cutProfilImage;    
    
    % Remove the lines.
    if yc<=50
        positionBCmax=infosImages(i).maxControlLine-infosImages(i).minStrip+midLongMask;
        cutFitProfilImage=NaN(1,positionBCmax-1)';
    else
        positionBCmin=infosImages(i).maxControlLine-infosImages(i).minStrip-midLongMask;
        positionBCmax=infosImages(i).maxControlLine-infosImages(i).minStrip+midLongMask;
        cutFitProfilImage=[cutProfil(1:positionBCmin);NaN(1,(positionBCmax-positionBCmin-1))'];
    end
    
    beg=positionBCmax;
    
    for l=1:(linesNumber-1)
        
        positionBTmin=infosImages(i).maxControlLine-infosImages(i).minStrip+C2C(l)-midLongMask;
        positionBTmax=infosImages(i).maxControlLine-infosImages(i).minStrip+C2C(l)+midLongMask;
        
        cutProfilToKeep = [];
        nanToAdd = [];
        if (beg < positionBTmin)
            cutProfilToKeep = cutProfil(beg:positionBTmin);
            nanToAdd = NaN(1,(positionBTmax-positionBTmin-1))';
        else
            nanToAdd = NaN(1,(positionBTmax-beg-1))';
        end

        if l==linesNumber-1
            cutFitProfilImage=[cutFitProfilImage;cutProfilToKeep;nanToAdd;cutProfil(positionBTmax:end)];
        else
            cutFitProfilImage=[cutFitProfilImage;cutProfilToKeep;nanToAdd];
        end
        beg=positionBTmax;
     end
    
    % Fit
    x=1:length(cutFitProfilImage);
    beta=nlinfit(x,cutFitProfilImage',model_affine,b0,opts);   

    % Fit + profile fitted 
    affFit=model_affine(beta,1:length(cutProfil));                           
    affFitImage=(cutProfil-affFit')-(min(cutProfil-affFit'));      

    % Calculs
    Test=mean(affFitImage(yt:yt+larg));
    Control=mean(affFitImage(yc:yc+larg));
    Bruit=mean(affFitImage(yb:yb+larg));

    CalculC_aff=(Test-Bruit)/(Control-Bruit);
    
end
