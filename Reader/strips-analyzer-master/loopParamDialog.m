function [paramName, paramStartValue, paramIncrement, nbLoop] = loopParamDialog(defaultWidth, defaultLengthROI, defaultMaskSize)

paramList = ["Rectangle LengthROI" "Rectangle Width" "Mask Size"];
paramName = "Rectangle LengthROI";
paramStartValue = defaultLengthROI;
paramIncrement = 0.05;
nbLoop = 50;

if (ispc)
    fontSizeBig = 16;
    fontSizeMedium = 14;
else
    fontSizeBig = 18;
    fontSizeMedium = 16;
end

d = dialog('Units','normalized','Position',[0.2 0.35 0.3 0.5],'Name','Loop Parameters');

uicontrol('Parent',d,'FontSize',fontSizeBig,'Style','text','Units','normalized',...
    'Position',[0 0.85 1 0.1],'String','Loop Parameters');

uicontrol('Parent',d,'FontSize',fontSizeMedium,'Style','text','Units','normalized',...
    'Position',[0.05 0.74 0.3 0.1],'String','Param to loop :');

uicontrol('Parent',d,'Style','popup','FontSize',fontSizeMedium,'Units','normalized','Position',[0.45 0.74 0.4 0.1],...
    'String',paramList,...
    'Callback',@paramSelection_callback);

uicontrol('Parent',d,'FontSize',fontSizeMedium,'Style','text','Units','normalized',...
    'Position',[0.05 0.65 0.3 0.1],'String','Start Value :');

startValueUi = uicontrol('Parent',d,'Style','edit','FontSize',fontSizeMedium,'Units','normalized','Position',[0.45 0.67 0.4 0.1],...
    'String',string(paramStartValue),'Callback',@startValue_callback);

uicontrol('Parent',d,'FontSize',fontSizeMedium,'Style','text','Units','normalized',...
    'Position',[0.05 0.54 0.3 0.1],'String','Increment :');

uicontrol('Parent',d,'Style','edit','FontSize',fontSizeMedium,'Units','normalized','Position',[0.45 0.56 0.4 0.1],...
    'String',string(paramIncrement),'Callback',@increment_callback);

uicontrol('Parent',d,'FontSize',fontSizeMedium,'Style','text','Units','normalized',...
    'Position',[0.05 0.43 0.3 0.1],'String','Nb Loops :');

uicontrol('Parent',d,'Style','edit','FontSize',fontSizeMedium,'Units','normalized','Position',[0.45 0.45 0.4 0.1],...
    'String',string(nbLoop),'Callback',@nbloop_callback);

uicontrol('Parent',d,'Units','normalized','Position',[0.38 0.052 0.28 0.1],'FontSize',fontSizeMedium,...
    'String','Start','Callback','delete(gcf)');

uiwait(d);

function paramSelection_callback(popup,~)
    idx = popup.Value;
    popup_items = popup.String;
    paramName = char(popup_items(idx,:));
    switch paramName
        case paramList(1)
            paramStartValue = defaultLengthROI;
        case paramList(2)
            paramStartValue = defaultWidth;
        case paramList(3)
            paramStartValue = defaultMaskSize;
    end
    startValueUi.String = string(paramStartValue);
end

function startValue_callback(edit,~)
    paramStartValue = str2double(edit.String);
end

function increment_callback(edit,~)
    paramIncrement = str2double(edit.String);
end

function nbloop_callback(edit,~)
    nbLoop = str2double(edit.String);
end

end