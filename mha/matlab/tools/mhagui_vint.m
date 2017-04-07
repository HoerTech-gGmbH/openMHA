function data = mhagui_vint( var, mha, fh, pos, range )
% MHAGUI_VINT - generic int vector control
%
% Usage: data = mhagui_vint( var, mha, fh, pos, range )
%
% Input arguments:
% - var : Name of MHA variable
% - mha : MHA handle structure
%
% Optional arguments:
% - fh  : Figure handle to place the objects in
% - pos : Object position
% - range : Range of slider
%
% Return value:
% - data : Parameter struct of control handle
%
% (c) 2006 University of Oldenburg
% Author: Giso Grimm
  ;
  if nargin < 2
    mha.host = 'localhost';
    mha.port = 33337;
  end
  if nargin < 3
    fh = figure('Numbertitle','off',...
	     'Name',var,...
	     'MenuBar','none');
    owner = 1;
  else
    owner = 0;
  end
  if nargin < 4
    pos = [0 0];
  end
  if nargin < 5
    range = [];
  end
  pos = [pos(1) pos(2) 0 0];
  sinfo = mha_getinfo(mha,var);
  if ~strcmp(sinfo.type,'vector<int>')
    error(sprintf('Variable ''%s'' is of type ''%s''.',var,sinfo.type));
  end
  h = fh;
  val = eval(sinfo.val);
  uih = [];
  if isempty( range )
    [v_min,v_max] = mhagui_rangestr2range(sinfo.range,mean(val));
  else
    v_min = range(1);
    v_max = range(2);
  end
  nentries = length(val);
  if ~mod(nentries,2)
    ncols = nentries/2;
    nrows = 2;
    he = 80;
  else
    ncols = nentries;
    nrows = 1;
    he = 160;
  end
  x = max(50,25*ncols);
  for k=1:length(val)
    uih_yp = he-floor((k-1)/ncols)*he;
    if nrows == 1
      uih_yp = 0;
    end
    uih(end+1) = uicontrol(...
	'style','slider',...
	'tooltipstring',sprintf('entry %d\n%s',k,sinfo.help),...
	'tag',sprintf('sl%d',k),...
	'Min',v_min,'Max',v_max,...
	'Value',val(k),...
	'callback',@mhagui_vint_cb,...
	'position',pos+[mod(k-1,ncols)*25+2 uih_yp 20 he-2]);
    %set(uih(end),'UserData',h);
  end
  %x = max(220,25*length(val));
  eh = uicontrol(...
      'style','edit',...
      'BackGroundColor',ones(1,3),...
      'tooltipstring',sinfo.help,...
      'String',sinfo.val,...
      'position',pos+[0 160 x 20],...
      'callback',@mhagui_vint_edit_cb);
  %set(eh,'UserData',h);
  if owner
    p = get(h,'Position');
    set(h,'Position',[p(1) p(2) x 180]);
  end
  data = struct;
  data.mha = mha;
  data.var = var;
  data.fig = h;
  data.uih = uih;
  data.edh = eh;
  data.update = @setvals;
  set(uih,'UserData',data);
  set(eh,'UserData',data);
  
function setvals(data)
  val = mha_get(data.mha,data.var);
  for k=1:length(val)
    set(data.uih(k),'Value',val(k));
  end
  set(data.edh,'String',...
	    num2str(mha_get(data.mha,data.var),' %d'));

function mhagui_vint_cb(obj,tmp)
  h = get(obj,'UserData');
  vals = round(get(h.uih,'Value'));
  if iscell( vals )
    vals = cell2mat(vals)';
  end
  try
    mha_set(h.mha,h.var,vals);
  catch
    errordlg(lasterr,'MHA Error');
  end
  setvals(h);
  
function mhagui_vint_edit_cb(obj,tmp)
  h = get(obj,'UserData');
  val = get(h.edh,'String');
  try
    mha_set(h.mha,h.var,str2num(val));
  catch
    errordlg(lasterr,'MHA Error');
  end
  set(h.edh,'String',...
	    num2str(mha_get(h.mha,h.var),' %g'));
  val = mha_get(h.mha,h.var);
  for k=1:min(length(h.uih),length(val))
    set(h.uih(k),'Value',val(k));
  end

  
