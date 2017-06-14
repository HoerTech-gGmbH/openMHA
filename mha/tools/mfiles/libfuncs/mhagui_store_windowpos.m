function mhagui_store_windowpos( varargin )
% MHAGUI_STORE_WINDOWPOS - Store current window position based on
% window tag
%
% Author: Giso Grimm
% Date: 2007
  ;
  tag = get(gcf,'Tag');
  tag = strrep(tag,':','.');
  tag = strrep(tag,' ','_');
  pos = get(gcf,'Position');
  tag = sprintf('wndpos.%s',tag);
  fname = 'mhagui_window_positions.mat';
  cfdb = libconfigdb();
  cfdb.writefile(fname,tag,pos(1:2));
  
