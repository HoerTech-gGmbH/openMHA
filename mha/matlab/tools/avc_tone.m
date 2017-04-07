function fig = avc_tone(mha_host, mha_port, dc_path)

if nargin < 1
    mha_host = 'localhost';
end
if nargin < 2
    mha_port = '33337';
end

mha = struct('host',mha_host,'port',str2double(mha_port));

if nargin < 3
    dc_path = mha_findid(mha,'dc');
    if ~isfield(dc_path,'dc')
        error('active mha has no dc plugin loaded');
    end
    dc_path = dc_path.dc
end

data.mha = mha;
data.dc_path = dc_path;
data.channels = mha_get(mha, [dc_path, '.mhaconfig_in.channels']) / 3;
screen_size = get(groot, 'ScreenSize');
screen_width = screen_size(3);
screen_height = screen_size(4);

width = 400;
height = 90;

fig = figure('Position',[floor((screen_width - width) / 2) , floor((screen_height - height) / 2), width,height],'NumberTitle','off','Name','AVC Tone Control','MenuBar','none','ToolBar','none','IntegerHandle','off');


uicontrol(fig, 'Style', 'text', 'HorizontalAlignment', 'left', 'String', 'Output Level', 'Position', [1,height-1*30,162,30]);
uicontrol(fig, 'Style', 'text', 'HorizontalAlignment', 'left', 'String', 'Compression Kneepoint', 'Position', [1,height-2*30,162,30]);
uicontrol(fig, 'Style', 'text', 'HorizontalAlignment', 'left', 'String', 'Tone Control', 'Position', [1,height-3*30,162,30]);

data.s_levl = uicontrol(fig, 'Style', 'slider', 'HorizontalAlignment', 'left', 'Position', [201,height-1*30 + 10,200,20], 'Min', 0, 'Max', 90, 'Callback', {@update,fig}, 'Value',80);
data.s_knee = uicontrol(fig, 'Style', 'slider', 'HorizontalAlignment', 'left', 'Position', [201,height-2*30 + 10,200,20], 'Min', 0, 'Max', 90, 'Callback', {@update,fig}, 'Value',50);
data.s_tone = uicontrol(fig, 'Style', 'slider', 'HorizontalAlignment', 'left', 'Position', [201,height-3*30 + 10,200,20], 'Min', -20, 'Max', 20, 'Callback', {@update,fig}, 'Value', 0);

data.l_levl = uicontrol(fig, 'Style', 'text', 'HorizontalAlignment', 'left', 'String', '80dB', 'Position', [163,height-1*30,38,30]);
data.l_knee = uicontrol(fig, 'Style', 'text', 'HorizontalAlignment', 'left', 'String', '50dB', 'Position', [163,height-2*30,38,30]);
data.l_tone = uicontrol(fig, 'Style', 'text', 'HorizontalAlignment', 'left', 'String', '0dB', 'Position', [163,height-3*30,38,30]);


set(fig,'UserData', data)

function update(hobject, callbackdata, fig)
data = get(fig,'UserData');

output_level = get(data.s_levl,'Value');
knee_level = get(data.s_knee,'Value');
tone_control = get(data.s_tone,'Value');

set(data.l_levl,'String',sprintf('%ddB',round(output_level)));
set(data.l_knee,'String',sprintf('%ddB',round(knee_level)));
set(data.l_tone,'String',sprintf('%ddB',round(tone_control)));

output_deltas = [-tone_control,0,tone_control];
output_deltas_norm = 10*log10(sum(10.^(output_deltas / 10)));

gtdata = repmat(output_level - [0:99], 3, 1) + repmat(output_deltas(:),1,100) - output_deltas_norm;
gtdata(:,1:floor(knee_level)) = repmat(gtdata(:,floor(knee_level)+1),1,floor(knee_level));
gtdata = round(repmat(gtdata,data.channels,1));

mha_set(data.mha,[data.dc_path,'.gtdata'],gtdata);
