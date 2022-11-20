function [ meas,cassette,condition,nanoparticules,pixelsStudied] = dialogbox_v4

% DIALOGBOX_v4 :
%
%   Version of the function dialogbox adapted to the third version of
%   stripsAnalysis . This function opens a dialogbox through which the user
%   indicates the rights parameters : the number of strip(s), the kind of
%   mesure, which prototype is used, if there is a cassette used or not and
%   in which conditions has been done the mesure.
%
%
% OUTPUT PARAMETERS :
%
%   STRIPSNUMBER : number of strips (lines) to analyse (test(s) line(s) +
%   control line).
%
%   MESURE : sandwich ou competition/inhibition.
%
%   PROTOTYPE : serie of prototype used (1 or 2).
%
%   CASSETTE : without/with.
%
%   CONDITION : visible/UV.
%
%   NANOPARTICULES : nanoparticules used for the analysis.
%
%   PIXELSSTUDIED : to know on which pixel (rgb) to work on.
%
%
% WRITTEN BY CVE
% V4 created by TMY on 17/10/2020

if (ispc)
    fontSizeVeryLittle = 10;
    fontSizeLittle = 12;
    fontSizeMedium = 14;
else
    fontSizeLittle = 12;
    fontSizeMedium = 16;
end

d = dialog('Units','normalized','Position',[0.2 0.35 0.3 0.5],'Name','Preliminary issues');

uicontrol('Parent',d,'FontSize',fontSizeMedium,'Style','text','Units','normalized',...
    'Position',[0 0.85 1 0.1],'String','Preliminary issues');

uicontrol('Parent',d,'FontSize',fontSizeMedium,'Style','text','Units','normalized',...
    'Position',[0.05 0.71 0.3 0.1],'String','Cassette :');

uicontrol('Parent',d,'Style','popup','FontSize',fontSizeVeryLittle,'Units','normalized','Position',[0.45 0.71 0.4 0.1],...
    'String',{'K701BT_SE';'K701WT_SE';'K701BT_BL';'K701WT_BL';'K701BT_NL';'K701WT_NL';},'Callback',@popup4_callback);

uicontrol('Parent',d,'FontSize',fontSizeMedium,'Style','text','Units','normalized',...
    'Position',[0.05 0.63 0.3 0.1],'String','Conditions :');

uicontrol('Parent',d,'Style','popup','FontSize',fontSizeMedium,'Units','normalized','Position',[0.45 0.63 0.4 0.1],...
    'String',{'UV';'Visible'},'Callback',@popup5_callback);

uicontrol('Parent',d,'FontSize',fontSizeLittle,'Style','text','Units','normalized',...
    'Position',[0.05 0.53 0.3 0.1],'String','Kind of measure :');

uicontrol('Parent',d,'Style','popup','FontSize',fontSizeMedium,'Units','normalized','Position',[0.45 0.53 0.4 0.1],...
    'String',{'Sandwich';'Competition/Inhibition';'Multiplex'},'Callback',@popup2_callback);

uicontrol('Parent',d,'FontSize',fontSizeMedium,'Style','text','Units','normalized',...
    'Position',[0.05 0.33 0.3 0.1],'String','Pixels studied (visible) :');

uicontrol('Parent',d,'Style','popup','FontSize',fontSizeMedium,'Units','normalized','Position',[0.45 0.33 0.4 0.1],...
    'String',{'Red';'Green';'Blue'},'Callback',@popup8_callback);

uicontrol('Parent',d,'FontSize',fontSizeMedium,'Style','text','Units','normalized',...
    'ForegroundColor',[0.35 0.35 0.35],'Position',[0.05 0.42 0.3 0.1],'String','Nanoparticules :');

uicontrol('Parent',d,'Style','popup','FontSize',fontSizeMedium,'Units','normalized','Position',[0.45 0.42 0.4 0.1],...
    'ForegroundColor',[0.35 0.35 0.35],'String',{'Europium';'Dysprosium';'Samarium'},'Callback',@popup7_callback);

uicontrol('Parent',d,'Units','normalized','Position',[0.35 0.052 0.28 0.1],'FontSize',fontSizeMedium,...
    'String','Next','Callback','delete(gcf)');

% Initialization
meas = 'Sandwich';
cassette = 'K701BT_SE';
condition = 'UV';
nanoparticules = 'Europium';
pixelsStudied = 'Red';

% Wait for d to close before running to completion
uiwait(d);
    function popup2_callback(popup,~)
        idx = popup.Value;
        popup_items = popup.String;
        meas = char(popup_items(idx,:));
    end

    function popup4_callback(popup,~)
        idx = popup.Value;
        popup_items = popup.String;
        cassette = char(popup_items(idx,:));
    end

    function popup5_callback(popup,~)
        idx = popup.Value;
        popup_items = popup.String;
        condition = char(popup_items(idx,:));
    end

    function popup7_callback(popup,~)
        idx = popup.Value;
        popup_items = popup.String;
        nanoparticules = char(popup_items(idx,:));
    end

    function popup8_callback(popup,~)
        idx = popup.Value;
        popup_items = popup.String;
        pixelsStudied = char(popup_items(idx,:));
    end
end