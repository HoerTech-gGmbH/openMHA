function fbc_control( hostname )
  if nargin < 1
    hostname = 'localhost';
  end
  global mha_basic_cfg;
  mhacontrol(hostname,33337,0);
  try
    midi_cc(24,0,@handle_midi,@wait_fh);
  catch
    warning(lasterr);
  end
  
function r = handle_midi( channel, param, value )
  %disp(sprintf('ch=%d param=%d value=%d\n',channel,param,value));
  if (channel == 0) && (param == 0)
    global mha_basic_cfg;
    vmax = mha_get(mha_basic_cfg.mha,[mha_basic_cfg.base.mastergain,'.max']);
    vmin = mha_get(mha_basic_cfg.mha,[mha_basic_cfg.base.mastergain,'.min']);
    dg = vmax-vmin;
    new_gain = value/127*dg+vmin;
    mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.mastergain, ...
		    '.gain'],new_gain);
    set(findobj('Tag',[mha_basic_cfg.base.mastergain,'.gain']),'Value',new_gain);
    set(findobj('Tag',...
		[mha_basic_cfg.base.mastergain,'.gain=str']),...
	'String',sprintf('%1.1f',new_gain));
    drawnow;
  end
  r = 0;
  
function wait_fh
  if isempty(findobj('Tag','mhacontrol_mainwindow'));
    error('main window closed');
  end