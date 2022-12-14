% - check max in upper part rather than manual for control strip
% - DO NOT MODIFY THIS FILE - we keep it as a reference, modify the TO MODIFY version
%{
Default parameters are : RGB, EXCLUDE TOO LARGE BANDS, Eu NANOPARTICLES, NUMBER OF GAINS=1, SIMPLEX, DEFAULT VALUES FOR GAIN
choice == 4
choice2 == 1
choice3 == 1
choice4 == 1
choice5 == 1
and use default values for gains
%}

function Resu = GetProfileROIFit2_Color()

    clear all;
close all;
clc;
%{
% % % % % % % % % % % % % % % % %

    AJUSTER LA VALEUR DE SDist ET DES DELTA POUR CHAQUE LOT DE BANDELETTES

    % ----------Notation
    : T = test,
      C = control %
          ----------Notation : _x : x est la position de la bande en partant du
                                    haut(juste apr�s la CAS) vers le bas %
          Images 1 = images prises au gain le plus fort % Images 2 =
                         images prises au gain le plus faible

                         % Definition of the analysis parameters
    : Modifier les valeurs si besoin !! %
      ----------------------------------------------------N2 = 130;
% Convolution Gaussian variance SDist = 350;
% Test - control distance delta2 = 1;%increment for the second ROI vertical position
%}
SDist = 350;
N2 = 130;
delta2 = 1;
delta3=1;%increment for the third ROI vertical position 
fw=round(sqrt(N2))*5;
bw = 50;
limLIN = 75000;
m = 1;
M = 255;
% ----------------------------------------------------

Col = 'RGB';

% Default to "exlude too large bands" (otherwise maybe implement gaussian) 
WTH = 3;

% Number of gains = 1

    % Image oppening %
    %-------------------------------------------------------------------------

    % Image folder : gain 1 
% Instead of chosing the image every time it's faster to declare it's path
% file = "/Users/spark/Desktop/Pictures";  % Here just paste a Picture file path 
% [pn, ~, ~] = fileparts(file);  % pn1: path

fprintf("CHOSE THE OpenReader PROJECT FOLDER!");
proj_path = uigetdir();  % Choose directory of OpenReader project folder

pn = proj_path + "/Pictures";
cd(pn);
a1 = dir('*.png');
N1 = numel(a1);
warning off;
pause on;

% Creation of the.txt file

cd(pn);
% Type of assay = 'Simplex'
  Title = [
    'Name',    ';', 'T1',     ';', 'C',     ';', 'BG_T1',    ';', 'BG_C',    ';', 'R�_T1',  ';', 'R�_C',  ';', 'Width_T1', ';', 'Width_C', ';', 'T/BG_1', ';', 'Gain1', ';', 'ROI1'
  ];

% Opening results txt file for pic 1
fs = a1(1).name;
Nomp=fs(strfind(fs, 'N') + 1 : strfind(fs, '.') - 1);  % image number (sybbols from 'N' to '.' in the image name)
NomAna=['AnalysisResults' Nomp '_' Col '.txt'];
fileID=fopen(NomAna,'w');
fprintf(fileID,Title);
fprintf(fileID,'\r\n');
NomA=[];

% First image
Nom1=a1(1).name;

   
% Load images of folder 1
Im1=double(imread(Nom1));  % pixels array n x m x 3
% Col = 'RGB'
A1=(Im1(:,:,1)+Im1(:,:,2)+Im1(:,:,3))/3;

%comment these 2 lines if using with v1 reader 
A1 = transpose(A1);  
A1 = flipud(A1);  % We use v2
   
%-------------------------------------------------------------------------

% Selection of the appropriate image
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Image gain 1 -----------------------------------------------------------


% Image parameters : image 1
m1=min(min(A1));  % M = min(A) returns the minimum elements of an array.
M1=max(max(A1));
S1=size(A1);
h1=S1(1);  % number of raws
w1=S1(2);  % number of columns


%Display image 1

imshow(A1,[m1 M1]);


% Control selection : click in the middle of the control band
fh=gcf;
k = waitforbuttonpress;
point2 = get(gca, 'CurrentPoint');
ca=point2(1,1);
f=point2(1,2);


% Point selection
xc=ca;
yc=f;
x1=xc;
y1=yc+SDist;

rectangle('Position',[xc-bw,yc-fw,bw*2,2*fw],'EdgeColor','g')
rectangle('Position',[x1-bw,y1-fw,bw*2,2*fw],'EdgeColor','r')


% Replacement of the ROI if it exceeds the initial image
y1=min(max(y1,fw+1),h1-fw);
yc=min(max(yc,fw+1),h1-fw);


%Pixel selection and treatment
I1_T1=A1(y1-fw:y1+fw,x1-bw:x1+bw);
ROIvalue1_1=sum(sum(I1_T1));
I1_C=A1(yc-fw:yc+fw,xc-bw:xc+bw);


pause(0.7);
%-------------------------------------------------------------------------

% Bogdana modified up to here
cd(proj_path + "/Analysis_with_convol");
% Image selection

I_T1_Raw=I1_T1;
Gain1=1;
ROIvalue_1=ROIvalue1_1;  
I_T1=ConvolIm(I_T1_Raw,N2);
I_C_Raw=I1_C;
I_C=ConvolIm(I_C_Raw,N2);
I_C_Raw=I1_C;
I_C=ConvolIm(I_C_Raw,N2);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Compute profiles
Profil_T1=sum(I_T1');
Profil_T1_Raw=sum(I_T1_Raw');
Profil_C=sum(I_C');
Profil_C_Raw=sum(I_C_Raw');


% Profile inversion of images with gold Nps
% removed as we use Eu Nps


% Parameters for normalization
m_T1=min(Profil_T1);
Profil_T1=Profil_T1-m_T1;
M_T1=max(Profil_T1);

m_C=min(Profil_C);
Profil_C=Profil_C-m_C;
M_C=max(Profil_C);

lx=length(Profil_C)-1;
Px=0:lx;


% Guiding parameters for the fit
posM_T1=min(find(Profil_T1==M_T1)); 
posM_C=min(find(Profil_C==M_C));

bg_T1=mean(Profil_T1(1:10))/M_T1; 
bg_C=mean(Profil_C(1:10))/M_C;

d_T1=(Profil_T1(lx-10)-Profil_T1(10))/(M_T1*(lx-20));
d_C=(Profil_C(lx-10)-Profil_C(10))/(M_C*(lx-20));


% truncature before fit
Profil_T1_trunc=Profil_T1(:,11:100);
Profil_C_trunc=Profil_C(:,11:100);
Px_trunc=Px(:,11:100);


% Initialisation fit
beta0_T1=[bg_T1 d_T1 1 posM_T1 sqrt(2*N2) ];
beta0_C=[bg_C d_C 1 posM_C sqrt(2*N2) ];


% Fit without constraint
GaussTrend = @(par,x)  par(1) + (x-10)*par(2) + par(3)*exp(-((x-par(4)).^2)/par(5)^2); 

[beta_T1, r_T1, J_T1]=nlinfit(Px_trunc,Profil_T1_trunc/M_T1,GaussTrend,beta0_T1);
[beta_C, r_C, J_C]=nlinfit(Px_trunc,Profil_C_trunc/M_C,GaussTrend,beta0_C);

fit_T1=M_T1*GaussTrend(beta_T1,Px)+m_T1;
fit_C=M_C*GaussTrend(beta_C,Px)+m_C;


% Estimate of the fit quality

MR_T1=sum(r_T1.^2);
MR_C=sum(r_C.^2);

D_T1=sum((Profil_T1/M_T1-mean(Profil_T1/M_T1)).^2);
D_C=sum((Profil_C/M_C-mean(Profil_C/M_C)).^2);

R2_T1=1-MR_T1/D_T1; %R2 means R�
R2_C=1-MR_C/D_C;


% Background calculation

betaBG_T1=beta_T1;
betaBG_T1(3)=0;
betaBG_C=beta_C;
betaBG_C(3)=0;

BGx_T1=M_T1*GaussTrend(betaBG_T1,Px)+m_T1;
BGx_C=M_C*GaussTrend(betaBG_C,Px)+m_C;


%Display image and fits 

ii=num2str(i);
NN=num2str(N1);
G1=num2str(Gain1);
gain1=num2str(1);
Title_Im=['Image' ' ' ii '/' NN ' - Gain ' gain1 ];
subplot(2,3,1);
set(gcf, 'Units', 'Normalized', 'OuterPosition', [0.25, 0.25, 0.75, 0.75]);
imshow(A1,[m M],'InitialMagnification',200);
rectangle('Position',[xc-bw,yc-fw,bw*2,2*fw],'EdgeColor','g')
rectangle('Position',[x1-bw,y1-fw,bw*2,2*fw],'EdgeColor','r')
title(['\fontsize{16}' Title_Im]);

subplot(2,3,2);
Title_T1=['Gain ' G1];
plot(Px,Profil_T1+m_T1,'+');
hold on
plot(Px,Profil_T1_Raw,'k');
plot(Px,fit_T1,'r');
plot(Px,BGx_T1,'g');
xlim([0 round(Px(length(Px))/10)*10]);
xlabel('Position (pixel)');
ylabel('Intensity (a.u.)');
title(['\fontsize{16}Test Band Profile n�1 - ' Title_T1]);
% ------------------------- Romain modifies up to here
subplot(2,3,3);
    
%Choice5 !=2
plot(Px,Profil_C+m_C,'+');
hold on
plot(Px,fit_C,'r');
plot(Px,Profil_C_Raw,'k');
plot(Px,BGx_C,'g');
xlim([0 round(Px(length(Px))/10)*10]);
xlabel('Position (pixel)');
ylabel('Intensity (a.u.)');
title('\fontsize{16}Control Band Profile');





%---Exclusion criterion based on the band width

W_T1=abs(beta_T1(5));

if W_T1>WTH*sqrt(N2)
    Si_1=0;
else
Si_1=(beta_T1(3))*max(Profil_T1)*W_T1;
end

W_C=abs(beta_C(5));
C=(beta_C(3))*max(Profil_C)*W_C;


% Calcul du BG

if (round(beta_T1(4))>0 && round(beta_T1(4))<length(BGx_T1))
Bg_T1=BGx_T1(round(beta_T1(4)))*W_T1;
else
    round(length(BGx_T1)/2)
    Bg_T1=BGx_T1(round(length(BGx_T1)/2))*W_T1;
end

Bg_C=BGx_C(round(beta_C(4)))*W_C;


% Eliminate negative signals

if (beta_T1(3)<0) 
    Si_1=0;
end
if (beta_C(3)<0)
    C=0;
end



close all;


% Save results in a structure
Res(1).Name=Nom1;

Res(1).Test1=Si_1;
Res(1).Control=C;

Res(1).BG_T1=Bg_T1;
Res(1).BG_C=Bg_C;

Res(1).R2_T=R2_T1;
Res(1).R2_C=R2_C;

Res(1).WidthT1=W_T1;
Res(1).WidthC=W_C;

%Alternatively Strip over Control
Res(1).T_BG_T1=Si_1/Bg_T1;
Res(1).N_gain1=Gain1;
Res(1).ROI1=ROIvalue_1;





NomA=[NomA, Nom1, ' '];

fprintf(fileID,Nom1);

fprintf(fileID,';');
fprintf(fileID,'%d',Si_1);

fprintf(fileID,';');
fprintf(fileID,'%d',C);

fprintf(fileID,';');
fprintf(fileID,'%d',Bg_T1);

fprintf(fileID,';');
fprintf(fileID,'%d',Bg_C);

fprintf(fileID,';');
fprintf(fileID,'%d',R2_T1);

fprintf(fileID,';');
fprintf(fileID,'%d',R2_C);

fprintf(fileID,';');
fprintf(fileID,'%d',W_T1);

fprintf(fileID,';');
fprintf(fileID,'%d',W_C);

fprintf(fileID,';');
fprintf(fileID,'%d',Si_1/Bg_T1);

fprintf(fileID,';');
fprintf(fileID,'%d',Gain1);

fprintf(fileID,';');
fprintf(fileID,'%d',ROIvalue_1);


fprintf(fileID,'\r\n');



fclose(fileID);
%close all;


Resu=Res;
end
