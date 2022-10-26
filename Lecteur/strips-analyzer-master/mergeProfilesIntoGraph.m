function mergeProfilesIntoGraph(infosImages, studyName, graphPath, graphName, fileNameSuffix, getProfile, getProfileAbscisse, waitbarFigure, wbPercentStart, wbPercentRange)

color_list={'k','g','r','b','y','m','c','--g','--r','--b','--y','--m','--c'};
%cd(strcat(rawImagesPath,'Results'))


cf = figure('Name',[studyName ' : ' graphName],'Visible','off');

myAxes = axes(cf);
myAxes.Toolbar.Visible= 'off';
myAxes.Interactions = [];

if isempty(waitbarFigure)
    wbf = waitbar(0, ['Study ' studyName], 'Name', ['Generating graph ' graphName]);
    wbPercentStart = 0;
    wbPercentRange = 0.99;
else
    wbf = waitbarFigure;
end


% Initialization
C0=999;
c=0;
concentrationList = [];
concentrationMap = containers.Map({42},{'TheAnswer'});
p=[];
listei=[];

N=length(infosImages);
for i=1:N
    
    waitbar(wbPercentStart + (i/N - 1/N) * wbPercentRange, wbf, ['Remaining ' num2str(N - i)]);
    
    % Get the concentration of the image.
    concentration = infosImages(i).FileInfo.Concentration;
    
    % Check if it is the first image with this concentration. If yes,
    % change of color.
    if C0~=concentration
        c=c+1;
        clr=color_list{c};
        C0=concentration;
        concentrationList=[concentrationList,concentration];
        concentrationMap(concentration) = infosImages(i).FileInfo.ConcentrationText;
        listei=[listei,i];
    end
    
    % Get profil.
    imageProfil=getProfile(infosImages(i));
    abscisses = getProfileAbscisse(infosImages(i));
    
    % Plot the profil.
    p(i)=plot(myAxes, abscisses,imageProfil,clr);
    hold on
    
end
hold off

% List for the legend.
concentrationLegendList = "";

%concentrationList = sort(concentrationList);
for i=1:length(concentrationList)
    concentrationLegendList=[concentrationLegendList,concentrationMap(concentrationList(i))];
    %concentrationLegendList(i) = concentrationMap(concentrationList(i));
end

listeplot=[];

for i=1:length(listei)
    listeplot=[listeplot,p(listei(i))];
end

waitbar(wbPercentStart + wbPercentRange, wbf, 'Saving...');

% Plot the legend.
legend(myAxes, listeplot, cellstr(concentrationLegendList(2:end)));

filename = strcat(graphPath, '/', graphName, '-',studyName,'-',fileNameSuffix, '.png');
print(cf, filename, '-dpng', '-r400');
close(cf)

if (isempty(waitbarFigure))
    close(wbf);
end

end