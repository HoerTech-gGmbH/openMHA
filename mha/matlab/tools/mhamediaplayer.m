function [fh, h] = mhamediaplayer( files, channels, ports, refgain )
% MHAMEDIAPLAYER - play (short) sound files through an MHA/Jack
%
% Usage:
%
% [fh, h] = mhamediaplayer( files, channels, ports, refgain );
%
% files : cell string array with file names
% channels : number of channels
% ports : cell string array with JACK output ports
% refgain : reference gain
%
% Return values:
% fh : figure handle
% h : MHA handle
%
  ;
  if nargin < 2
    channels = 2;
  end
  if nargin < 3
    ports = {'alsa_pcm:playback_1','alsa_pcm:playback_2'};
  end
  if nargin < 4
    refgain = 0;
  end
  mhamedia = struct;
  mhamedia.files = files;
  mhamedia.list = files;
  for k=1:length(files)
    [p,b,e] = fileparts(files{k});
    mhamedia.list{k} = sprintf('%s (%s)',b,files{k});
  end
  fh = figure('NumberTitle','off',...
	      'MenuBar','none',...
	      'Name','MHA media player','Renderer','None');
  p = get(fh,'Position');
  he = 360;
  we = 500;
  p(3) = we;
  p(4) = he;
  set(fh,'position',p);
  cfg = struct;
  cfg.nchannels_in = channels;
  cfg.peaklevel_in = 0;
  cfg.peaklevel_out = 0;
  cfg.fragsize = 64;
  cfg.mhalib = 'addsndfile';
  cfg.iolib = 'MHAIOJack';
  cfg.io.name = 'MHAmedia';
  cfg.io.con_out = ports;
  cfg.mha.filename = files{1};
  cfg.mha.channels = [0:(channels-1)];
  cfg.mha.peaklevel = refgain;
  h = mha_start;
  mhamedia.h = h;
  mha_set(h,'',cfg);
  mha_set(h,'cmd','prepare');
  mha_transport_gui( h, fh, [10 he-140] );
  infopos = [10 he-100 we-20 90];
  uicontrol('Style','Frame','Position',infopos,...
	    'BackgroundColor',[179 224 179]/255);
  mhamedia.info = ...
      uicontrol('Style','Text','Position',infopos+[3 26 -6 -29],...
		'HorizontalAlignment','left',...
		'FontSize',12,'FontWeight','Bold',...
		'BackgroundColor',[179 224 179]/255,...
		'ForegroundColor',[47 76 47]/255);
  mhamedia.fileinfo = ...
      uicontrol('Style','Text','Position',infopos.*[1 1 1 0]+[3 3 -6 20],...
		'HorizontalAlignment','left',...
		'FontSize',10,'FontWeight','normal',...
		'BackgroundColor',[179 224 179]/255,...
		'ForegroundColor',[47 76 47]/255);
  uicontrol('style','text','Position',[390 he-125 we-400 15],...
	    'String','Gain:','HorizontalAlignment','left');
  uicontrol('style','slider',...
	    'min',-30,'max',30,'Value',0,...
	    'position',[390 he-140 we-400 15],...
	    'callback',@setgain,'userdata',mhamedia);
  uicontrol('style','checkbox',...
	    'String','loop','value',1,...
	    'Position',[340 he-140 50 30],...
	    'UserData',mhamedia,...
	    'callback',@setloop);
  hsel = ...
      uicontrol('style','listbox',...
	    'String',mhamedia.list,...
	    'Position',[10 10 we-20 he-160],...
	    'Callback',@selectfile,'userdata',mhamedia);
  selectfile(hsel);
  
function setgain(obj, tmp)
  mhamedia = get(obj,'UserData');
  gain = get(obj,'value');
  mha_set(mhamedia.h,'peaklevel_out',-gain);
  update_info(mhamedia);

function selectfile( obj, tmp)
  mhamedia = get(obj,'UserData');
  file = mhamedia.files{get(obj,'value')};
  mha_set(mhamedia.h,'mha.filename',file);
  update_info(mhamedia);
  
function update_info(mhamedia)
  file = mha_get(mhamedia.h,'mha.filename');
  gain = -mha_get(mhamedia.h,'peaklevel_out');
  loop = mha_get(mhamedia.h,'mha.loop');
  if loop
    sloop = 'looping';
  else
    sloop = 'once';
  end
  [p,b,e] = fileparts(file);
  set(mhamedia.info,...
      'String',sprintf('%s%s\ngain: %g dB\nplaymode: %s',b,e,gain, ...
		       sloop));
  set(mhamedia.fileinfo,...
      'String',file);
  
function setloop( obj, tmp )
  mhamedia = get(obj,'UserData');
  val = get(obj,'Value');
  mha_set(mhamedia.h,'mha.loop',val);
  update_info(mhamedia);
