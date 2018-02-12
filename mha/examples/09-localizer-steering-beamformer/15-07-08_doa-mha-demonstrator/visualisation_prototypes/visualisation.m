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

t = linspace(0, recorded_seconds, recorded_seconds*fps);
d = -90:5:90;
[D, T] = meshgrid(d, t);

% initial plot
fig = figure();
% ax = imshow(all_p);
ax = surf(T, D, all_p, 'LineSmoothing', 'on');
xlabel('Time t/s (0=now)');
ylabel('Direction d/degrees');
zlabel('Probability');

% maximise the size of the plot
% set(gca, 'Position', [0 0.02 1 0.97])
set(gca, 'cameraposition', [60 -300 16])

% update the figure in a loop
while true
    p = mha_get(mha_handle, 'mha.mhachain.db.mhachain.doasvm_mon.p');

    all_p(2:end) = all_p(1:end-1);
    all_p(1, :) = p;

    % set(ax, 'CData', all_p);
    set(ax(1), 'ZData', all_p);

    % implicitly calls drawnow
    pause(spf);
end
