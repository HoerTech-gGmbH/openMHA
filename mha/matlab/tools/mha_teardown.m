function mha_teardown(mha, varargin)
% mha_teardown(mha_handle [, resource ...])
%
% Release mha resources. If no resource argument is given, all mha
% resources are freed, otherwise only the specified resources are
% freed. Possible resources (and effects) are:
% 'tcpsound' -- tcp connection for exchanging sound data (close)
% 'tcpcmd'   -- tcp connection for exchanging mha skript commands (close)
  
if any(strcmp(varargin, 'tcpsound'))
  mha_tcpsound(mha.tcp.sound.handle, 'close');
else
  if isempty(varargin) 
    if isfield(mha,'tcp')
      if isfield(mha.tcp, 'sound')
        mha_tcpsound(mha.tcp.sound.handle, 'close');
      end
    end
  end  
end
if isempty(varargin) | any(strcmp(varargin, 'tcpcmd'))
    pause(0.01);
    try
        mha.tcp.cmd = mhactl(mha.tcp.cmd, 'close');
    catch
        if isfield(mha.tcp.cmd, 'socket');
            mha.tcp.cmd = rmfield(mha.tcp.cmd, 'socket');
        end
    end
    mhactl_wrapper(mha.tcp.cmd, 'cmd=quit');
end
