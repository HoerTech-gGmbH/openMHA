function close_mha(window_title)
% This function closes a running mha with default port by sending cmd=quit
% if this does not work, then on windows the mha window is closed.

if isequal(false, window_title)
    return
end

mha.host = 'localhost';
mha.port = 33337;
mha.timeout = 2;
try
    mha_set(mha, 'cmd', 'quit');
catch
    err = lasterror;
    if ~ispc
        % we have not implemented means to close mhas on other platforms
        rethrow(lasterror);
    end
    [pids, exes] = enumprocesses;
    mha_idx = strmatch('mha.exe', exes, 'exact');
    for pid = pids(mha_idx)
        killprocess(pid);
    end
end
