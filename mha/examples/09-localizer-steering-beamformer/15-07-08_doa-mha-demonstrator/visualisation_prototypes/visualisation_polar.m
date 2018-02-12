% the path to MHA's matlab directory
MHA_PATH='/home/marcec/Downloads/MHA-Forschergruppe-TpA-devel-4.4.58-x86_64-linux-gcc-4.6/matlab';

% add necessary paths
addpath(MHA_PATH);
javaaddpath([MHA_PATH filesep 'mhactl_java.jar']);

mha_handle = struct('host', 'localhost', 'port', 33337);

% number of seconds to record, frames per second, and seconds per frame
recorded_seconds=10;
fps=30;
spf=1/fps;

p = mha_get(mha_handle, 'mha.mhachain.db.mhachain.doasvm_mon.p');
all_p = zeros(recorded_seconds*fps, size(p, 2));
all_p(1, :) = p;

% Both t and d are reversed (and d is offset by 90°) so that the points come
% from the top and go towards the center.
t = recorded_seconds-spf:-spf:0;
d = (180:-5:0)*pi/180;

[p_max, p_idx] = max(all_p, [], 2);

% initial plot
fig = figure();
ax = polar(d(p_idx), t, '.');
xlabel('Time t/s (10=now)');
ylabel('Direction d/degrees');

% maximise the size of the plot
axis('tight');
ylim([0 recorded_seconds]);

% update the figure in a loop
while true
    p = mha_get(mha_handle, 'mha.mhachain.db.mhachain.doasvm_mon.p');

    all_p(2:end) = all_p(1:end-1);
    all_p(1, :) = p;
    [p_max, p_idx] = max(all_p, [], 2);
    [x, y] = pol2cart(d(p_idx), t);

    set(ax, 'XData', x);
    set(ax, 'yData', y);

    % implicitly calls drawnow
    pause(spf);
end
