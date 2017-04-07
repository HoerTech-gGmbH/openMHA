% MHA Finetuning-GUI, Version 0.99 (BETA)
% Status: 1st version with new "apperance"!
%
% MatLab-Call:  mhagui_finetuning(handle, prefix, f, pos)
%
% when calling, MHA has to be active and the following
% variables have to exist:
%    handle.host = 'localhost';
%    handle.port = 33337;
%    prefix = 'mha.plug.finetuning.';
%         --> otherwise the paramters above have to be
%             handed to the finetuning GUI manually...
%    mhagui_generic
%    mha_set(handle,['cmd'],'prepare')
%
% f and pos are for internal use and can (should) not be
% specified when calling manually: when specified, f is
% the figure handle of an existing GUI, in which this
% Finetuning-GUI will be included and pos is the position
% inside the GUI specified by f where the Finetuning-GUI
% will be placed...
%
% 19.07.07, M. Vormann, Hörzentrum Oldenburg

function mhagui_finetuning(handle, prefix, f, pos)



% check if this function is called by other functions:
  if ischar(handle)
    if strcmp(handle,'reset')          % gains are reseted somewhere in the MHA --> do a reset!
      button_reset_callback;
      return;
    elseif strcmp(handle,'update')     % gains are updated somewhere in the MHA --> do an update!
      button_update_gui_callback;
      return;
    else
      error(['invalid mode ''',handle,'''.']);
    end
  end

  % position-argument if this GUI is placed in other GUIs
  if nargin < 4
    pos = [0 0];
  end
  pos(3:4) = 0;



  %%%%%%%%%%%%%%%%%%%%%%
  % Read data from MHA %
  %%%%%%%%%%%%%%%%%%%%%%

  % TEMP
  % nchannels = 2;
  % nbands = 10;
  % %nbands = 20;
  % gains = [0 0 0 0 0 0 0 0 0 0; 0 0 0 0 0 0 0 0 0 0];
  % %gains = [0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0; 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0];
  % max = 16;
  % min = -16;
  % cf = [ 125 250 500 1000 1500 2000 3000 4000 8000 10000];
  % %cf = [80 125 200 250 400 500 750 1000 1200 1500 1750 2000 2500 3000 3500 4000 6000 8000 9000 10000];


  try
    nchannels = mha_get(handle, [prefix 'nchannels']);
  catch
    error('MHA_FineTuningGUI:CanNotReadFromMHA', ...
	  'Finetuning GUI cannot read from the MHA (channels)')
    disp(lasterr);
  end
  if nchannels ~= 2
    error ('MHA_FineTuningGUI:nchannels<>2', ...
	   'mhagui_finetuning can only tune hearing aids with two sound channels');
    disp(lasterr);
  end

  try
    nbands = mha_get(handle, [prefix 'nbands']);
  catch
    error('MHA_FineTuningGUI:CanNotReadFromMHA', ...
	  'Finetuning GUI cannot read from the MHA (nbands)')
    disp(lasterr);
  end
  try
    gains = mha_get(handle, [prefix 'gains']);
  catch
    error('MHA_FineTuningGUI:CanNotReadFromMHA', ...
	  'Finetuning GUI cannot read from the MHA (gains)')
    disp(lasterr);
  end
  try
    max = roundnumber(mha_get(handle, [prefix 'max']));
  catch
    error('MHA_FineTuningGUI:CanNotReadFromMHA', ...
	  'Finetuning GUI cannot read from the MHA (max)')
    disp(lasterr);
  end
  try
    min = roundnumber(mha_get(handle, [prefix 'min']));
  catch
    error('MHA_FineTuningGUI:CanNotReadFromMHA', ...
	  'Finetuning GUI cannot read from the MHA (min)')
    disp(lasterr);
  end
  try
    cf = mha_get(handle, [prefix 'cf']);
  catch
    error('MHA_FineTuningGUI:CanNotReadFromMHA', ...
	  'Finetuning GUI cannot read from the MHA (cf)')
    disp(lasterr);
  end

  %disp(sprintf('nbands: %u  max: %+2d  min: %+2d', nbands, max, min));

  if nbands <3
    error('MHA_FineTuningGUI:nbands_OutOfRange', ...
	  'FinetungingGUI not (yet?) implemented for nbands <3 !')
    disp(lasterr);
  end

  % for nbands > 20 the GUI will be more than 1024 pixels wide (with standard-paramters)
  % furthermore no frequncies for "combining" (lmh-freq) are defined any more...
  if nbands > 20
    error('MHA_FineTuningGUI:nbands_OutOfRange', ...
	  'FinetungingGUI not (yet?) implemented for nbands > 20 !')
    disp(lasterr);
  end

  %%% GISO!?!?!?
  % for nbands > 15 the GUI will be more than 770 pixels wide (with standard-paramters)
  % and therefore it might give problems... --> warning
  if nbands > 15
    disp(' ');
    disp('WARNING');
    disp('nbands>15 --> Check if FinetungingGUI fits onto the screen!?');
    disp(' ');
  end



  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Parameters which can be set by user %
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  % divide frequencies into 3 areas: low, middle, high:
  lmh_freq = [1 1 1; 1 2 1; 2 1 2; 2 2 2; ...     % variable defining the 3 frequency-regions
	      2 3 2; 3 2 3; 3 3 3; 3 4 3; 4 3 4;  ...     % "low" "midle" "high" when combining bands/
	      4 4 4; 4 5 4; 5 4 5; 5 5 5; 5 6 5;  ...     % channels
	      6 5 6; 6 6 6; 6 7 6; 7 6 7;];

  sides = [2 1];
  % order of the channels ("audilogy" [2 1] = right / left
  % or "normal" [1 2] = left / right)

  sidesn = ['L' 'R'];
  % label(names) of the sides(channels): order is left/right independent on
  % "sides"-variable above

  CB_bn = 1;
  % combine is valid for numbox-entries: 0 = NO, 1 = YES
  % --> normally combine is only valid for sliders (default)

  GUI_title = 'MHA FineTuning GUI 0.99 (BETA)';



  %%%%%%%%%%%%%%%%%%%%%%%%%%
  % Parameters for the GUI %
  %%%%%%%%%%%%%%%%%%%%%%%%%%

  hspacer(1) = 1;          % horizontal space between top border and "labels"
  band_xwidth = 25;        % maximum width of each band (2 sliders and 2 boxes under the sliders):
  label_height = 12;       % height of the labels
  hspacer(2) = 1;          % horizontal space between "labels" and slider
  slider_height = 95;      % height of the silders
  slider_width = 16;       % width of the silders
  hspacer(3) = 1;          % horizontal space between slider and mbox
  mbox_height = 20;        % height of the box for manual entry
  mbox_width = 27;         % width of the box for manual entry
  hspacer(4) = 2;          % horizontal space between mbox and checkboxes
  checkb_height = 16;      % height of the checkbox for combining
  checkb_width = 16;       % width of the checkbox for combining
  hspacer(5) = 3;          % horizontal space between checkboxes and combine-buttons
  hspacer(6) = 2;          % horizontal space between combine-buttons
  hspacer(7) = 5;          % horizontal space between combine-buttons and reset-buttons
  hspacer(8) = 1;          % horizontal space between reset-buttons and lower border
  CB_height_lmh = 8;       % height of the combine-buttons (low/mid/high)
  CB_height_all = 8;       % height of the combine-button "all"
  Reset_height = 20;       % height of the reset-button
  Reset_width = 80;        % width of the reset-button
  uncheck_width = 80;      % width of the reset-button
  x_space = 80;            % space between the channels (left and
  ;                        % right)
  
  % This is the space where the Gain-Buttons will be
  % placed...
  gainb_width  = 60;       % Width of the Gains-Buttons
  gainb_height = 20;       % Height of the Gains-Buttons           
  
  hspaces = sum(hspacer);
  vspaces = x_space;
  window_width  = 2*(band_xwidth*nbands +1) +vspaces;
  window_height = label_height + slider_height + mbox_height + checkb_height + ...
      CB_height_lmh + CB_height_all + Reset_height + hspaces;

  hpos(1) =                           hspacer(8);
  hpos(2) = hpos(1) + Reset_height  + hspacer(7);
  hpos(3) = hpos(2) + CB_height_all + hspacer(6);
  hpos(4) = hpos(3) + CB_height_lmh + hspacer(5);
  hpos(5) = hpos(4) + checkb_height + hspacer(4);
  hpos(6) = hpos(5) + mbox_height   + hspacer(3);
  hpos(7) = hpos(6) + slider_height + hspacer(2);

  % position of boxes beneath the sliders
  numboxpos = [band_xwidth/2-mbox_width/2, hpos(5), mbox_width, mbox_height];

  % position of the sliders:
  sliderpos = [band_xwidth/2-slider_width/2, hpos(6), slider_width, slider_height];

  % position of checkboxes beneath the numboxes
  checkbpos = [band_xwidth/2-checkb_width/2, hpos(4), checkb_width, checkb_height];

  % position of the "label" (frequency) for the band:
  labelpos = [0, hpos(7), band_xwidth, label_height+1];


  % backgroundcolor (slider): left = blue (1st entry), right = red (2nd entry)
  bgcolor = {[0.33,0.33,0.66],[0.66,0.33,0.33]};

  % foregroundcolor (text): left = blue (1st entry), right = red (2nd entry)
  color = {[0,0,1],[1,0,0]};

  % color of resetbutton
  col_resetb = [1 0.7 1];

  % colors of combine-buttons
  colcomb1 =    [1 1 0.7];       % low frequencies
  colcomb2 =    [1 1 0.4];       % mid frequencies
  colcomb3 =    [1 1 0.7];       % high frequencies
  colcomb4 =    [0.8 1 0.8];     % "all" frequencies

  %color of uncheck-button
  col_uncheck = colcomb4;        % the same as comcomb4!?

  col_gainbp = [0.5 0.6 0.5];
  col_gainbm = col_gainbp;

  % positon of GUI in the user-screen (if not fixed to a different window!)
  xpos = (1280 -window_width)/2;
  ypos = 500;



  %%%%%%%%%%%%%%%%%%%%%
  % Construct the GUI %
  %%%%%%%%%%%%%%%%%%%%%

  if nargin < 3
    f = figure('MenuBar', 'none', ...
	       'Position', [xpos, ypos, window_width, window_height], ...
	       'Name',GUI_title);
  else
    figure(f);
  end

  %button_reset_handle = ...
  uicontrol(f, 'Position', pos+[(window_width -Reset_width)/2, hpos(1), Reset_width, Reset_height], ...
	    'Tag', 'button_reset', ...
	    'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	    'Backgroundcolor',col_resetb, ...
	    'String', 'Reset to 0!', 'Fontsize',10,...
	    'Callback', @button_reset_callback);

  uncheckpos1 = (window_width -Reset_width)/4 -(uncheck_width/2);
  uncheckpos2 = window_width -(window_width -Reset_width)/4 -(uncheck_width/2);

  %button_uncheck1_handle = ...
  uicontrol(f, 'Position', pos+[uncheckpos1, hpos(1), uncheck_width, Reset_height], ...
	    'Tag', sprintf('button_uncheck%d',sides(1)), ...
	    'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	    'Backgroundcolor',col_uncheck, ...
	    'String', sprintf('Uncheck %s',sidesn(sides(1))), 'Fontsize',10,...
	    'Callback', @button_uncheck_callback);

  %button_uncheck2_handle = ...
  uicontrol(f, 'Position', pos+[uncheckpos2, hpos(1), uncheck_width, Reset_height], ...
	    'Tag', sprintf('button_uncheck%d',sides(2)), ...
	    'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	    'Backgroundcolor',col_uncheck, ...
	    'String', sprintf('Uncheck %s',sidesn(sides(2))), 'Fontsize',10,...
	    'Callback', @button_uncheck_callback);


  for ch =0:1
    side = sides(ch+1);
    x_offset = ch*nbands*band_xwidth +1;
    if x_offset > 1
      x_offset = x_offset +x_space;
    end

    %button_CBall_handle = ...
    uicontrol(f, 'Position', pos+[x_offset, hpos(2), nbands*band_xwidth+1, CB_height_all], ...
	      'Tag', sprintf('button_CBall_%u',side), ...
	      'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	      'Backgroundcolor',colcomb4, ...
	      'Callback', @button_CBall_callback);


    %button_CBlow_handle = ...
    uicontrol(f, 'Position', pos+[x_offset, hpos(3), lmh_freq(nbands-2,1)*band_xwidth+1, CB_height_lmh], ...
	      'Tag', sprintf('button_CBlow_%u',side), ...
	      'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	      'Backgroundcolor',colcomb1, ...
	      'Callback', @button_CBlow_callback);

    %button_CBmid_handle = ...
    uicontrol(f, 'Position', pos+[x_offset + lmh_freq(nbands-2,1)*band_xwidth, hpos(3), lmh_freq(nbands-2,2)*band_xwidth+1, CB_height_lmh], ...
	      'Tag', sprintf('button_CBmid_%u',side), ...
	      'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	      'Backgroundcolor',colcomb2, ...
	      'Callback', @button_CBmid_callback);


    %button_CBhigh_handle = ...
    uicontrol(f, 'Position', pos+[x_offset + ((lmh_freq(nbands-2,1)+lmh_freq(nbands-2,2))*band_xwidth), hpos(3), lmh_freq(nbands-2,3)*band_xwidth+1, CB_height_lmh], ...
	      'Tag', sprintf('button_CBhigh_%u',side), ...
	      'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	      'Backgroundcolor',colcomb3, ...
	      'Callback', @button_CBhigh_callback);


    for band = 0:(nbands-1)
      %handles.steller(band+1).num2str(side).checkbox = ...
      uicontrol(f, 'Position', pos+checkbpos + [((band_xwidth *band) +x_offset), 0,0,0], ...
		'Tag', sprintf('checkb_%u_%u',band+1,side), ...
		'ForegroundColor',color{side}, 'Style', 'CheckBox', ...
		'String', '', ...
		'Callback', @checkbox_callback);

      %handles.steller(band+1).num2str(side).numbox = ...
      uicontrol(f, 'Position', pos+numboxpos + [((band_xwidth *band) +x_offset), 0,0,0], ...
		'Tag', sprintf('numbox_%u_%u',band+1,side), ...
		'ForegroundColor',color{side}, 'Style', 'Edit', ...
		'String', num2str(gains(side,band+1)), 'Fontsize',10,...
		'Callback', @numbox_callback);

      %handles.steller(band+1).num2str(side).slider = ...
      uicontrol(f, 'Position', pos+sliderpos + [((band_xwidth *band) +x_offset), 0,0,0], ...
		'Tag', sprintf('slider_%d_%u',band+1,side), ...
		'BackgroundColor',bgcolor{side}, 'Style', 'Slider', ...
		'Max', max, 'Min', min, 'SliderStep', [1 1]/(max-min), ...
		'Value', gains(side,band+1), ...
		'Callback', @slider_callback);

      if cf(band+1) < 1000
	fstr = sprintf('%.0f',cf(band+1));
      else
	fstr = sprintf('%.1fk', cf(band+1) / 1000);
      end
      uicontrol(f, 'Position', pos+labelpos+[(x_offset +band_xwidth*band), 0,0,0], ...
		'String', fstr, 'Style', 'text');
    end
  end


  % Buttons fpr OverallGain:
  buttombuttons =labelpos(2) -hspacer(2);
  x_offset =  band_xwidth*nbands +x_space/2 -gainb_width/2 +2;

  uicontrol(f, 'Position', pos+[x_offset, labelpos(2), gainb_width, labelpos(4)], ...
	    'String', 'OverallGain', 'Style', 'text');

  uicontrol(f, 'Position', pos+[x_offset, (buttombuttons-1.*gainb_height), gainb_width, gainb_height], ...
	    'Tag', 'gainp5b', ...
	    'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	    'Backgroundcolor',col_gainbp, ...
	    'String', '+5 dB', 'Fontsize',10,...
	    'Callback', @button_gain_callback);
  
  uicontrol(f, 'Position', pos+[x_offset, (buttombuttons -2.25*gainb_height), gainb_width, gainb_height], ...
	    'Tag', 'gainp1b', ...
	    'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	    'Backgroundcolor',col_gainbp, ...
	    'String', '+1 dB', 'Fontsize',10,...
	    'Callback', @button_gain_callback);

  uicontrol(f, 'Position', pos+[x_offset, (buttombuttons -3.5*gainb_height), gainb_width, gainb_height], ...
	    'Tag', 'gainm1b', ...
	    'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	    'Backgroundcolor',col_gainbm, ...
	    'String', '-1 dB', 'Fontsize',10,...
	    'Callback', @button_gain_callback);

  uicontrol(f, 'Position', pos+[x_offset, (buttombuttons -4.75*gainb_height), gainb_width, gainb_height], ...
	    'Tag', 'gainm5b', ...
	    'ForegroundColor',[0 0 0], 'Style', 'pushbutton', ...
	    'Backgroundcolor',col_gainbm, ...
	    'String', '-5 dB', 'Fontsize',10,...
	    'Callback', @button_gain_callback);



  %%%%%%%%%%%%%%%%%%%%%%%%
  % Setup data-structure %
  %%%%%%%%%%%%%%%%%%%%%%%%

  dat = guidata(f);
  dat.nchannels = nchannels;
  dat.nbands = nbands;
  dat.max = max;
  dat.min = min;
  dat.CB_lr = num2str(0);
  dat.CB_high = num2str(0);
  dat.CB_low = num2str(0);
  dat.lmh = lmh_freq(nbands-2,:);
  dat.gains = gains;
  dat.hd = handle;
  dat.pref = prefix;
  dat.CB_bn = CB_bn;
  dat.checked = zeros(size(gains));
  guidata(f, dat);


  handles.function.button_reset_callback = @button_reset_callback;
    handles.function.button_uncheck_callback = @button_uncheck_callback;
      handles.function.button_all_callback = @button_CBall_callback;
	handles.function.button_low_callback = @button_CBlow_callback;
	  handles.function.button_mid_callback = @button_CBmid_callback;
	    handles.function.button_high_callback = @button_CBhigh_callback;
	      handles.function.checkbox_callback = @checkbox_callback;
		handles.function.numbox_callback = @numbox_callback;
		  handles.function.slider_callback = @slider_callback;
		    handles.function.bgain_callback = @button_gain_callback;


%%%%%%%%%%%%%%%%%
% GUI-CallBacks %
%%%%%%%%%%%%%%%%%


% SLIDERS are moved:
function slider_callback(hObject,dummy)
% determine respective band and channel:
  scan = sscanf(get(hObject,'tag'),'slider_%d_%d', [1,2]);
  band = scan(1);
  side = scan(2);
  % get slider-value
  SliValue = get(hObject,'Value');
  % update sliders/numboxes
  update_GUI(hObject, band, side, SliValue);



% NUMBOX entries have been changed:
function numbox_callback(hObject,dummy)
% read number from numbox:
  BoxEntry = str2double(get(hObject,'String'));
  % determine respective band and channel:
  scan = sscanf(get(hObject,'tag'),'numbox_%d_%d', [1,2]);
  band = scan(1);
  side = scan(2);
  if isempty(BoxEntry) || isnan(BoxEntry) || ~isnumeric(BoxEntry)
    dat = guidata(gcf);
    BoxEntry = dat.gains(side,band);
    set(hObject,'String',num2str(BoxEntry));
  end
  update_GUI(hObject, band, side, BoxEntry);



% activate "COMBINING" all channels (for one side):
function button_CBall_callback(hObject,dummy)
  dat = guidata(gcf);
  a = get(hObject,'tag');
  side = str2num(a(end));
  %disp(sprintf('all %u',side));
  for i=1:size(dat.checked,2)
    %disp(sprintf('checkb_%u_%u',i,side))
    dat.checked(side,i) = 1;
    res_checkbox_handle = findobj('Tag',sprintf('checkb_%u_%u',i,side));
    set(res_checkbox_handle,'Value', 1);
  end
  %dat.checked
  guidata(gcf, dat);



% activate "COMBINING" for LOW frequencies:
function button_CBlow_callback(hObject,dummy)
  dat = guidata(gcf);
  a = get(hObject,'tag');
  side = str2num(a(end));
  %disp(sprintf('low %u',side));
  for i=1:dat.lmh(1)
    %disp(sprintf('checkb_%u_%u',i,side))
    dat.checked(side,i) = 1;
    res_checkbox_handle = findobj('Tag',sprintf('checkb_%u_%u',i,side));
    set(res_checkbox_handle,'Value', 1);
  end
  %dat.checked
  guidata(gcf, dat);


% activate "COMBINING" for MIDDLE frequencies:
function button_CBmid_callback(hObject,dummy)
  dat = guidata(gcf);
  a = get(hObject,'tag');
  side = str2num(a(end));
  %disp(sprintf('mid %u',side));
  for i=dat.lmh(1)+1:dat.lmh(1)+dat.lmh(2)
    %disp(sprintf('checkb_%u_%u',i,side))
    dat.checked(side,i) = 1;
    res_checkbox_handle = findobj('Tag',sprintf('checkb_%u_%u',i,side));
    set(res_checkbox_handle,'Value', 1);
  end
  %dat.checked
  guidata(gcf, dat);


% activate "COMBINING" for HIGH frequencies:
function button_CBhigh_callback(hObject,dummy)
  dat = guidata(gcf);
  a = get(hObject,'tag');
  side = str2num(a(end));
  %disp(sprintf('high %u',side));
  for i=dat.lmh(1)+dat.lmh(2)+1:size(dat.checked,2)
    %disp(sprintf('checkb_%u_%u',i,side))
    dat.checked(side,i) = 1;
    res_checkbox_handle = findobj('Tag',sprintf('checkb_%u_%u',i,side));
    set(res_checkbox_handle,'Value', 1);
  end
  %dat.checked
  guidata(gcf, dat);


% uncheck all Combinations
function button_uncheck_callback(hObject,dummy)
  dat = guidata(gcf);
  a = get(hObject,'tag');
  side = str2num(a(end));
  for i=1:size(dat.checked,2)
    if (dat.checked(side,i) == 1)
      dat.checked(side,i) = 0;
      res_checkbox_handle = findobj('Tag',sprintf('checkb_%u_%u',i,side));
      set(res_checkbox_handle,'Value', 0);
    end
  end
  guidata(gcf, dat);


% Check/Uncheck one specific band:
function checkbox_callback(hObject,dummy)
  dat = guidata(gcf);
  a = get(hObject,'tag');
  side = str2num(a(end));
  temp = findstr(a,'_');
  band = str2num(a(temp(1)+1:temp(2)-1));
  dat.checked(side,band) = mod((dat.checked(side,band)+1),2);
  %disp(sprintf('check/uncheck side: %u band: %u',side,band));
  guidata(gcf, dat);


% RESET everything to zero
function button_reset_callback(hObject,dummy)
  dat = guidata(gcf);
  dat.gains = zeros(dat.nchannels,dat.nbands);

  for band = 1:dat.nbands
    for side = 1:dat.nchannels
      res_numbox_handle = findobj('Tag',sprintf('numbox_%u_%u',band,side));
      res_slider_handle = findobj('Tag',sprintf('slider_%u_%u',band,side));
      set(res_numbox_handle,'String',num2str(0));
      set(res_slider_handle,'Value',0);
    end
  end

  guidata(gcf, dat);
  % write data to MHA
  update_mha();



% RESET everything to values saved in MHA (when called from other GUI)
function button_update_gui_callback(hObject,dummy)
  dat = guidata(gcf);
  dat.gains = mha_get(dat.hd, [dat.pref 'gains']);

  for band = 1:dat.nbands
    for side = 1:dat.nchannels
      res_numbox_handle = findobj('Tag',sprintf('numbox_%u_%u',band,side));
      res_slider_handle = findobj('Tag',sprintf('slider_%u_%u',band,side));
      set(res_numbox_handle,'String',num2str(roundnumber(dat.gains(side,band))));
      set(res_slider_handle,'Value',roundnumber(dat.gains(side,band)));
    end
  end

  guidata(gcf, dat);
  %%%  GISO: soll das nicht besser upgedatet werden!? ich meine, ich runde ja
  %%%  und u.u. ist danach der angezeigte wert und der wahre wert im MHA
  %%%  (eben um diese rundung) verschieden!?
  % write data to MHA
  update_mha();



% change OverAllGain:
function button_gain_callback(hObject,dummy)
  dat = guidata(gcf);
  dat.gains = mha_get(dat.hd, [dat.pref 'gains']);

  a = get(hObject,'tag');
  %disp(sprintf('tag: %s',a));

  switch a
   case 'gainp5b'
    oag =5;
   case 'gainp1b'
    oag =1;
   case 'gainm1b'
    oag = -1;
   case 'gainm5b'
    oag = -5;
  end

  for band = 1:dat.nbands
    for side = 1:dat.nchannels
      res_numbox_handle = findobj('Tag',sprintf('numbox_%u_%u',band,side));
      res_slider_handle = findobj('Tag',sprintf('slider_%u_%u',band,side));
      dat.gains(side,band) = dat.gains(side,band) +oag;
      if dat.gains(side,band) > dat.max
	dat.gains(side,band) = dat.max;
      end
      if dat.gains(side,band) < dat.min
	dat.gains(side,band) = dat.min;
      end
      set(res_numbox_handle,'String',num2str(roundnumber(dat.gains(side,band))));
      set(res_slider_handle,'Value',roundnumber(dat.gains(side,band)));
    end
  end

  guidata(gcf, dat);
  update_mha();




%%%%%%%%%%%%%
% Functions %
%%%%%%%%%%%%%


function rounded = roundnumber(number)
%  rounding numbers - can be extended/adapted if needed...
  rounded = round(number);



function update_GUI(hObject, band, side, SliValue)
%  update all controls (sliders/numboxes) of GUI
  dat = guidata(gcf);
  SliValue = roundnumber(SliValue);
  Change = SliValue -dat.gains(side,band);

  % check if this band/side is checked --> then change all checked sliders/numboxes
  % the same amount - remember that changes via the numboxes will only be applicated
  % when the user-configurable switch "CB_bn" is set to 1:
  if (dat.checked(side,band) == 1 && ...
      (strcmp(get(hObject,'Style'),'slider') == 1 || dat.CB_bn == 1))
    array = dat.checked *Change;
    newarray = array + dat.gains;
    for band = 1:dat.nbands
      for side = 1:dat.nchannels
	if array(side, band) ~= 0;
	  res_numbox_handle = findobj('Tag',sprintf('numbox_%u_%u',band,side));
	  res_slider_handle = findobj('Tag',sprintf('slider_%u_%u',band,side));

	  % check if Values are inside (slider~)intervall
	  if newarray(side, band) > dat.max
	    newarray(side, band) = dat.max;
	  end
	  if newarray(side, band) < dat.min
	    newarray(side, band) = dat.min;
	  end
	  set(res_numbox_handle,'String',num2str(newarray(side, band)));
	  set(res_slider_handle,'Value',newarray(side, band));
	end
      end
    end
  else  % band/side is NOT checked --> only change THIS single value!
    array = zeros(dat.nchannels,dat.nbands);
    array(side,band) = 1;
    array = array *Change;
    newarray = array + dat.gains;

    res_numbox_handle = findobj('Tag',sprintf('numbox_%u_%u',band,side));
    res_slider_handle = findobj('Tag',sprintf('slider_%u_%u',band,side));

    % check if Value is inside (slider~)intervall
    if newarray(side, band) > dat.max
      newarray(side, band) = dat.max;
    end
    if newarray(side, band) < dat.min
      newarray(side, band) = dat.min;
    end
    set(res_numbox_handle,'String',num2str(newarray(side, band)));
    set(res_slider_handle,'Value',newarray(side, band));
  end

  dat.gains = newarray;
  %%newarray
  guidata(gcf, dat);
  % write data to MHA
  update_mha();



function update_mha()
% writes gains to MHA
  dat = guidata(gcf);
  mha_set(dat.hd, [dat.pref 'gains'], dat.gains);



  % EOF
