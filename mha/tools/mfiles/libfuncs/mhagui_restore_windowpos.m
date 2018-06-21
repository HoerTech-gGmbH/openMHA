function mhagui_restore_windowpos(fh)
% MHAGUI_RESTORE_WINDOWPOS - restore previously stored window
% position, based on window tag name
%
% Usage:
% mhagui_restore_windowpos(fh)
%
% - fh : figure handle of current window to be restored.
%
% Author: Giso Grimm
% Date: 2007
  ;
  tag = get(fh,'Tag');
  tag = strrep(tag,':','.');
  tag = strrep(tag,' ','_');
  pos = get(fh,'Position');
  tag = sprintf('wndpos.%s',tag);
  fname = 'mhagui_window_positions.mat';
  cfdb = libconfigdb();
  pos(1:2) = cfdb.readfile(fname,tag,pos(1:2));
  set(fh,'Position',pos);
