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

    % Affichage des diff�rents menus

          choice = menu('Choose a color', 'Red', 'Green', 'Blue', 'RGB');

Col = 'NB';
if choice== 1 Col = 'R';
end 
if choice == 2 Col = 'G';
end 
if choice == 3 Col = 'B';
end 
if choice == 4 Col = 'RGB';
end 
Nc = choice;

% Default to yes(otherwise maybe implement gaussian) 
choice2 = menu('Do you want to exlude too large bands ?', 'Yes', 'No');
if choice2 == 1 
    WTH = 3;
else
  WTH = 10000;
end

    choice3 = menu('Nanoparticle type ?', 'Eu', 'Au');

choice4 = menu('Number of gains ?', '1', '2');

choice5 = menu('Type of assay ?', 'Simplex', 'Multiplex');

if (choice4 == 1)
  prompt = {'Gain'};
dlg_title = 'Gain des images';
num_lines = 1;
defaultans = {'100'};
answer = inputdlg(prompt, dlg_title, num_lines, defaultans);
answer1 = str2double(answer{1});
else prompt = {'Gain max', 'Gain min'};
dlg_title = 'Gain des images';
num_lines = 1;
defaultans = {'100', '50'};
answer = inputdlg(prompt, dlg_title, num_lines, defaultans);
answer1 = str2double(answer{1});
answer2 = str2double(answer{2});
end

    % Image oppening %
    %-------------------------------------------------------------------------

    % Image folder : gain 1 
[fn1 pn1] = uigetfile('*.png');
pn1 = pn1(1 : length(pn1) - 1);
cd(pn1);
a1 = dir('*.png');
N1 = numel(a1);
warning off;
pause on;

pn = pn1(1 : length(pn1) - 2);% ROMAIN: SEEMS NOT TO WORK
cd(pn); 

% Image folder : gain 2 
if (choice4 == 2)
[fn2 pn2] = uigetfile('*.png');
pn2 = pn2(1 : length(pn2) - 1);
cd(pn2);
a2 = dir('*.png');
warning off;
pause on;
end
% Creation of the.txt file

cd(pn);
if (choice5 == 1)
  Title = [
    'Name',    ';', 'T1',     ';', 'C',     ';', 'BG_T1',    ';',
    'BG_C',    ';', 'R�_T1',  ';', 'R�_C',  ';', 'Width_T1', ';',
    'Width_C', ';', 'T/BG_1', ';', 'Gain1', ';', 'ROI1'
  ];
else
  Title = [
    'Name',    ';', 'T1',       ';', 'T2',       ';', 'T3',       ';',
    'C',       ';', 'BG_T1',    ';', 'BG_T2',    ';', 'BG_T3',    ';',
    'BG_C',    ';', 'R�_T1',    ';', 'R�_T2',    ';', 'R�_T3',    ';',
    'R�_C',    ';', 'Width_T1', ';', 'Width_T2', ';', 'Width_T3', ';',
    'Width_C', ';', 'T/BG_1',   ';', 'T/BG_2',   ';', 'T/BG_3',   ';',
    'Gain1',   ';', 'Gain2',    ';', 'Gain3',    ';', 'ROI1',     ';',
    'ROI2',    ';', 'ROI3'
  ];
end
fs=find(pn=='\');
Nomp=pn(fs(length(fs)-1)+1:fs(length(fs))-1);
NomAna=['AnalysisResults' Nomp '_' Col '.txt'];
fileID=fopen(NomAna,'w');
fprintf(fileID,Title);
fprintf(fileID,'\r\n');
NomA=[];

% Loop for all images in the folder
for i=1:N1
   Nom1=a1(i).name;
   if (choice4==2)
       Nom2=a2(i).name;
   end

   
% Load images of folder 1
cd(pn1);
Im1=double(imread(Nom1));
if Nc==4
    A1=(Im1(:,:,1)+Im1(:,:,2)+Im1(:,:,3))/3;
else
A1=Im1(:,:,Nc);
end
%comment these 2 lines if using with v1 reader
A1 = transpose(A1);
A1 = flipud(A1);
   
% Load images of folder 2
if (choice4==2)
    cd (pn2)
    Im2=double(imread(Nom2));
    if Nc==4
        A2=(Im2(:,:,1)+Im2(:,:,2)+Im2(:,:,3))/3;
    else
        A2=Im2(:,:,Nc);
    end
end

cd(pn)
%-------------------------------------------------------------------------

% Selection of the appropriate image
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Image gain 1 -----------------------------------------------------------


% Image parameters : image 1
m1=min(min(A1));
M1=max(max(A1));
S1=size(A1);
h1=S1(1);
w1=S1(2);


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

if (choice5==2)
    x2=xc;
    y2=y1+SDist+delta2;
    x3=xc;
    y3=y2+SDist+delta3;
    rectangle('Position',[x2-bw,y2-fw,bw*2,2*fw],'EdgeColor','b')
    rectangle('Position',[x3-bw,y3-fw,bw*2,2*fw],'EdgeColor','y')
    y2=min(max(y2,fw+1),h1-fw);
    y3=min(max(y3,fw+1),h1-fw);
    I1_T2=A1(y2-fw:y2+fw,x2-bw:x2+bw);
    ROIvalue1_2=sum(sum(I1_T2));
    I1_T3=A1(y3-fw:y3+fw,x3-bw:x3+bw);
    ROIvalue1_3=sum(sum(I1_T3));
end

pause(0.7);
%-------------------------------------------------------------------------

% Image gain 2 -----------------------------------------------------------

if (choice4==2)

    % Image parameters : image 2
    m2=min(min(A2));
    M2=max(max(A2));
    S2=size(A2);
    h2=S2(1);
    w2=S2(2);
    
    
    %Display image 1
    imshow(A2,[m2 M2]);
    
    
    % Control selection : click in the middle of the control band
    fh=gcf;
    k = waitforbuttonpress;
    point2 = get(gca, 'CurrentPoint');
    ca=point2(1,1);
    f=point2(1,2);
    
    % ------------------------ Bogdana modifies up to here
    % Point selection
    Xc=ca;
    Yc=f;
    X1=Xc;
    Y1=Yc+SDist;
    
    rectangle('Position',[Xc-bw,Yc-fw,bw*2,2*fw],'EdgeColor','g')
    rectangle('Position',[X1-bw,Y1-fw,bw*2,2*fw],'EdgeColor','r')
    
    
    % Replacement of the ROI if it exceeds the initial image
    Y1=min(max(Y1,fw+1),h2-fw);
    Yc=min(max(Yc,fw+1),h2-fw);
    
    
    %Pixel selection and treatment
    I2_T1=A2(Y1-fw:Y1+fw,X1-bw:X1+bw);
    ROIvalue2_1=sum(sum(I2_T1));
    I2_C=A2(Yc-fw:Yc+fw,Xc-bw:Xc+bw);
    
    if (choice5==2)
        X2=Xc;
        Y2=Y1+SDist+delta2;
        X3=Xc;
        Y3=Y2+SDist+delta3;
        rectangle('Position',[X2-bw,Y2-fw,bw*2,2*fw],'EdgeColor','b')
        rectangle('Position',[X3-bw,Y3-fw,bw*2,2*fw],'EdgeColor','y')
        Y2=min(max(Y2,fw+1),h2-fw);
        Y3=min(max(Y3,fw+1),h2-fw);
        I2_T2=A2(Y2-fw:Y2+fw,X2-bw:X2+bw);
        ROIvalue2_2=sum(sum(I2_T2));
        I2_T3=A2(Y3-fw:Y3+fw,X3-bw:X3+bw);
        ROIvalue2_3=sum(sum(I2_T3));
    end
    
    pause(0.5);

end
%-------------------------------------------------------------------------


% Image selection


if (choice4==2)
    if ROIvalue1_1<limLIN
        I_T1_Raw=I1_T1;
        Gain1=answer1;
        ROIvalue_1=ROIvalue1_1;
    else
        I_T1_Raw=I2_T1;
        Gain1=answer2;
        ROIvalue_1=ROIvalue1_1; %% Attention
    end
    
    I_T1=ConvolIm(I_T1_Raw,N2);
    
    if (choice5==2)
        if ROIvalue1_2<limLIN
            I_T2_Raw=I1_T2;
            Gain2=answer1;
            ROIvalue_2=ROIvalue1_2;
        else
            I_T2_Raw=I2_T2;
            Gain2=answer2;
            ROIvalue_2=ROIvalue1_2; %% Attention
        end
        if ROIvalue1_3<limLIN
            I_T3_Raw=I1_T3;
            Gain3=answer1;
            ROIvalue_3=ROIvalue1_3;
        else
            I_T3_Raw=I2_T3;
            Gain3=answer2;
            ROIvalue_3=ROIvalue1_3; %% Attention
        end
        
        I_T2=ConvolIm(I_T2_Raw,N2);
        I_T3=ConvolIm(I_T3_Raw,N2);
    end
    I_C_Raw=I1_C;
I_C=ConvolIm(I_C_Raw,N2);
else
    I_T1_Raw=I1_T1;
    Gain1=answer1;
    ROIvalue_1=ROIvalue1_1;  
    I_T1=ConvolIm(I_T1_Raw,N2);
    
    if (choice5==2)
        I_T2_Raw=I1_T2;
        Gain2=answer1;
        ROIvalue_2=ROIvalue1_2;
        I_T2=ConvolIm(I_T2_Raw,N2);
        I_T3_Raw=I1_T3;
        Gain3=answer1;
        ROIvalue_3=ROIvalue1_3;    
        I_T3=ConvolIm(I_T3_Raw,N2);
    end  
    I_C_Raw=I1_C;
I_C=ConvolIm(I_C_Raw,N2);
end

I_C_Raw=I1_C;
I_C=ConvolIm(I_C_Raw,N2);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Compute profiles
Profil_T1=sum(I_T1');
Profil_T1_Raw=sum(I_T1_Raw');
Profil_C=sum(I_C');
Profil_C_Raw=sum(I_C_Raw');

if (choice5==2)
    Profil_T2=sum(I_T2');
    Profil_T2_Raw=sum(I_T2_Raw');
    Profil_T3=sum(I_T3');
    Profil_T3_Raw=sum(I_T3_Raw');
end


% Profile inversion of images with gold Nps
if (choice3==2)
    Profil_T1=max(Profil_T1)-Profil_T1;
    Profil_T1_Raw=max(Profil_T1_Raw)-Profil_T1_Raw;
    Profil_C=max(Profil_C)-Profil_C;
    Profil_C_Raw=max(Profil_C_Raw)-Profil_C_Raw;
    if (choice==5)
            Profil_T2=max(Profil_T2)-Profil_T2;
            Profil_T2_Raw=max(Profil_T2_Raw)-Profil_T2_Raw;
            Profil_T3=max(Profil_T3)-Profil_T3;
            Profil_T3_Raw=max(Profil_T3_Raw)-Profil_T3_Raw;
    end
end


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


% tronquage avant fit
Profil_T1_trunc=Profil_T1(:,11:100);
Profil_C_trunc=Profil_C(:,11:100);
Px_trunc=Px(:,11:100);


% Initialisation fit
beta0_T1=[bg_T1 d_T1 1 posM_T1 sqrt(2*N2) ];
beta0_C=[bg_C d_C 1 posM_C sqrt(2*N2) ];


% Fit sans contraintes
GaussTrend = @(par,x)  par(1) + (x-10)*par(2) + par(3)*exp(-((x-par(4)).^2)/par(5)^2); 

[beta_T1 r_T1 J_T1]=nlinfit(Px_trunc,Profil_T1_trunc/M_T1,GaussTrend,beta0_T1);
[beta_C r_C J_C]=nlinfit(Px_trunc,Profil_C_trunc/M_C,GaussTrend,beta0_C);

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

if (choice5==2)
    m_T2=min(Profil_T2);
    Profil_T2=Profil_T2-m_T2;
    M_T2=max(Profil_T2);
    m_T3=min(Profil_T3);
    Profil_T3=Profil_T3-m_T3;
    M_T3=max(Profil_T3);
    posM_T2=min(find(Profil_T2==M_T2));
    posM_T3=min(find(Profil_T3==M_T3));
    bg_T2=mean(Profil_T2(1:5))/M_T2;
    bg_T3=mean(Profil_T3(1:5))/M_T3;
    d_T2=(Profil_T2(lx-5)-Profil_T2(5))/(M_T2*(lx-10));
    d_T3=(Profil_T3(lx-5)-Profil_T3(5))/(M_T3*(lx-10));
    Profil_T2_trunc=Profil_T2(:,6:26);
    Profil_T3_trunc=Profil_T3(:,6:26);
    beta0_T2=[bg_T2 d_T2 1 posM_T2 sqrt(2*N2) ];
    beta0_T3=[bg_T3 d_T3 1 posM_T3 sqrt(2*N2) ];
    [beta_T2 r_T2 J_T2]=nlinfit(Px_trunc,Profil_T2_trunc/M_T2,GaussTrend,beta0_T2);
    [beta_T3 r_T3 J_T3]=nlinfit(Px_trunc,Profil_T3_trunc/M_T3,GaussTrend,beta0_T3);
    fit_T2=M_T2*GaussTrend(beta_T2,Px)+m_T2;
    fit_T3=M_T3*GaussTrend(beta_T3,Px)+m_T3;
    MR_T2=sum(r_T2.^2);
    MR_T3=sum(r_T3.^2);
    D_T2=sum((Profil_T2/M_T2-mean(Profil_T2/M_T2)).^2);
    D_T3=sum((Profil_T3/M_T3-mean(Profil_T3/M_T3)).^2);
    R2_T2=1-MR_T2/D_T2;
    R2_T3=1-MR_T3/D_T3;
    betaBG_T2=beta_T2;
    betaBG_T2(3)=0;
    betaBG_T3=beta_T3;
    betaBG_T3(3)=0;
    BGx_T2=M_T2*GaussTrend(betaBG_T2,Px)+m_T2;
    BGx_T3=M_T3*GaussTrend(betaBG_T3,Px)+m_T3;
end

%Display image and fits 

ii=num2str(i);
NN=num2str(N1);
G1=num2str(Gain1);
if (choice5==2)
    G2=num2str(Gain2);
    G3=num2str(Gain3);
end
gain1=num2str(answer1);
if (choice4==2)
    gain2=num2str(answer2);
end

Title_Im=['Image' ' ' ii '/' NN ' - Gain ' gain1 ];
subplot(2,3,1);
set(gcf, 'Units', 'Normalized', 'OuterPosition', [0.25, 0.25, 0.75, 0.75]);
imshow(A1,[m M],'InitialMagnification',200);
rectangle('Position',[xc-bw,yc-fw,bw*2,2*fw],'EdgeColor','g')
rectangle('Position',[x1-bw,y1-fw,bw*2,2*fw],'EdgeColor','r')
if (choice5==2)
    rectangle('Position',[x2-bw,y2-fw,bw*2,2*fw],'EdgeColor','b')
    rectangle('Position',[x3-bw,y3-fw,bw*2,2*fw],'EdgeColor','y')
end
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
if (choice5==2)
    Title_T2=['Gain ' G2];
    plot(Px,Profil_T2+m_T2,'+');
    hold on
    plot(Px,Profil_T2_Raw,'k');
    plot(Px,fit_T2,'r');
    plot(Px,BGx_T2,'g');
    xlim([0 round(Px(length(Px))/10)*10]);
    xlabel('Position (pixel)');
    ylabel('Intensity (a.u.)');
    title(['\fontsize{16}Test Band Profile n�2 - ' Title_T2]);
else
    plot(Px,Profil_C+m_C,'+');
    hold on
    plot(Px,fit_C,'r');
    plot(Px,Profil_C_Raw,'k');
    plot(Px,BGx_C,'g');
    xlim([0 round(Px(length(Px))/10)*10]);
    xlabel('Position (pixel)');
    ylabel('Intensity (a.u.)');
    title('\fontsize{16}Control Band Profile');
end

if (choice4==2)
    subplot(2,3,4);
    Title_Im=['Image' ' ' ii '/' NN ' - Gain ' gain2 ];
    set(gcf, 'Units', 'Normalized', 'OuterPosition', [0.25, 0.25, 0.75, 0.75]);
    imshow(A2,[m M],'InitialMagnification',200);
    rectangle('Position',[Xc-bw,Yc-fw,bw*2,2*fw],'EdgeColor','g')
    rectangle('Position',[X1-bw,Y1-fw,bw*2,2*fw],'EdgeColor','r')
    if (choice5==2)
        rectangle('Position',[X2-bw,Y2-fw,bw*2,2*fw],'EdgeColor','b')
        rectangle('Position',[X3-bw,Y3-fw,bw*2,2*fw],'EdgeColor','y')
    end
    title(['\fontsize{16}' Title_Im]);
end

if (choice5==2)
    
    subplot(2,3,5);
    Title_T3=['Gain ' G3];
    plot(Px,Profil_T3+m_T3,'+');
    hold on
    plot(Px,Profil_T3_Raw,'k');
    plot(Px,fit_T3,'r');
    plot(Px,BGx_T3,'g');
    xlim([0 round(Px(length(Px))/10)*10]);
    xlabel('Position (pixel)');
    ylabel('Intensity (a.u.)');
    title(['\fontsize{16}Test Band Profile n�3 - ' Title_T3]);
    
    subplot(2,3,6);
    plot(Px,Profil_C+m_C,'+');
    hold on
    plot(Px,fit_C,'r');
    plot(Px,Profil_C_Raw,'k');
    plot(Px,BGx_C,'g');
    xlim([0 round(Px(length(Px))/10)*10]);
    xlabel('Position (pixel)');
    ylabel('Intensity (a.u.)');
    title('\fontsize{16}Control Band Profile');

end

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

if (choice5==2)
    W_T2=abs(beta_T2(5));
    W_T3=abs(beta_T3(5));
    
    if W_T2>WTH*sqrt(N2)
        Si_2=0;
    else
        Si_2=(beta_T2(3))*max(Profil_T2)*W_T2;
    end
    if W_T3>WTH*sqrt(N2)
        Si_3=0;
    else
        Si_3=(beta_T3(3))*max(Profil_T3)*W_T3;
    end
    
    if (round(beta_T2(4))>0 && round(beta_T2(4))<length(BGx_T2))
        Bg_T2=BGx_T2(round(beta_T2(4)))*W_T2;
    else
        round(length(BGx_T2)/2)
        Bg_T2=BGx_T2(round(length(BGx_T2)/2))*W_T2;
    end
    if (round(beta_T3(4))>0 && round(beta_T3(4))<length(BGx_T3))
        Bg_T3=BGx_T3(round(beta_T3(4)))*W_T3;
    else
        round(length(BGx_T3)/2)
        Bg_T3=BGx_T3(round(length(BGx_T3)/2))*W_T3;
    end
    
    if (beta_T2(3)<0)
        Si_2=0;
    end
    if (beta_T3(3)<0)
        Si_3=0;
    end
    
end

close all;


% Save results in a structure
Res(i).Name=Nom1;

Res(i).Test1=Si_1;
Res(i).Control=C;

Res(i).BG_T1=Bg_T1;
Res(i).BG_C=Bg_C;

Res(i).R2_T=R2_T1;
Res(i).R2_C=R2_C;

Res(i).WidthT1=W_T1;
Res(i).WidthC=W_C;

%Alternatively Strip over Control
Res(i).T_BG_T1=Si_1/Bg_T1;
Res(i).N_gain1=Gain1;
Res(i).ROI1=ROIvalue_1;


if choice5==2

    Res(i).Test2=Si_2;
    Res(i).Test3=Si_3;
    Res(i).BG_T2=Bg_T2;
    Res(i).BG_T3=Bg_T3;
    Res(i).R2_T=R2_T2;
    Res(i).R2_T=R2_T3;
    Res(i).WidthT2=W_T2;
    Res(i).WidthT3=W_T3;
    Res(i).T_BG_T2=Si_2/Bg_T2;
    Res(i).T_BG_T3=Si_3/Bg_T3;
    Res(i).N_gain2=Gain2;
    Res(i).N_gain3=Gain3;
    Res(i).ROI2=ROIvalue_2;
    Res(i).ROI3=ROIvalue_3;
end


NomA=[NomA, Nom1, ' '];

fprintf(fileID,Nom1);

fprintf(fileID,';');
fprintf(fileID,'%d',Si_1);
if choice5==2
    fprintf(fileID,';');
    fprintf(fileID,'%d',Si_2);
    fprintf(fileID,';');
    fprintf(fileID,'%d',Si_3);
end
fprintf(fileID,';');
fprintf(fileID,'%d',C);

fprintf(fileID,';');
fprintf(fileID,'%d',Bg_T1);
if choice5==2
    fprintf(fileID,';');
    fprintf(fileID,'%d',Bg_T2);
    fprintf(fileID,';');
    fprintf(fileID,'%d',Bg_T3);
end
fprintf(fileID,';');
fprintf(fileID,'%d',Bg_C);

fprintf(fileID,';');
fprintf(fileID,'%d',R2_T1);
if choice5==2
    fprintf(fileID,';');
    fprintf(fileID,'%d',R2_T2);
    fprintf(fileID,';');
    fprintf(fileID,'%d',R2_T3);
end
fprintf(fileID,';');
fprintf(fileID,'%d',R2_C);

fprintf(fileID,';');
fprintf(fileID,'%d',W_T1);
if choice5==2
    fprintf(fileID,';');
    fprintf(fileID,'%d',W_T2);
    fprintf(fileID,';');
    fprintf(fileID,'%d',W_T3);
end
fprintf(fileID,';');
fprintf(fileID,'%d',W_C);

fprintf(fileID,';');
fprintf(fileID,'%d',Si_1/Bg_T1);
if choice5==2
    fprintf(fileID,';');
    fprintf(fileID,'%d',Si_2/Bg_T2);
    fprintf(fileID,';');
    fprintf(fileID,'%d',Si_3/Bg_T3);
end
fprintf(fileID,';');
fprintf(fileID,'%d',Gain1);
if choice5==2
    fprintf(fileID,';');
    fprintf(fileID,'%d',Gain2);
    fprintf(fileID,';');
    fprintf(fileID,'%d',Gain3);
end
fprintf(fileID,';');
fprintf(fileID,'%d',ROIvalue_1);
if choice5==2
    fprintf(fileID,';');
    fprintf(fileID,'%d',ROIvalue_2);
    fprintf(fileID,';');
    fprintf(fileID,'%d',ROIvalue_3);
end

fprintf(fileID,'\r\n');


end
fclose('all');
close all;


Resu=Res;
