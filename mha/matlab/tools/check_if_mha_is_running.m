function is_running = check_if_mha_is_running()
% Checks if there is a window whose title ends in '\mha.exe'
% return false if there is none, return the title of that window if there
% is one.

if ~ispc
    error('check for running mhas only works on Windows');
end

MHA_EXE = '\mha.exe';
window_titles = enumwindows;
is_running = false;
for i = 1:length(window_titles)
    title = window_titles{i};
    pos = strfind(title, MHA_EXE);
    if ~isempty(pos)
        if (pos(end) + length(MHA_EXE) - 1) == length(title)
            is_running = title;
            return
        end
    end
end
