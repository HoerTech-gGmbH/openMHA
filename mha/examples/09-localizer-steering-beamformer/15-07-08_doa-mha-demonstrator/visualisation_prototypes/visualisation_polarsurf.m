% the path to MHA's matlab directory
MHA_PATH='/home/marcec/Downloads/MHA-Forschergruppe-TpA-devel-4.4.58-x86_64-linux-gcc-4.6/matlab';

% add necessary paths
addpath(MHA_PATH);
javaaddpath([MHA_PATH filesep 'mhactl_java.jar']);

mha_handle = struct('host', 'localhost', 'port', 33337);

% number of seconds to record, frames per second, and seconds per frame
recorded_seconds=5;
fps=10;
spf=1/fps;

p = mha_get(mha_handle, 'mha.mhachain.db.mhachain.doasvm_mon.p');
all_p = zeros(recorded_seconds*fps, size(p, 2));
all_p(1, :) = p;

% Both t and d are reversed (and d is offset by 90°) so that the points come
% from the top and go towards the center.
t = recorded_seconds-spf:-spf:0;
d = (180:-5:0)*pi/180;
[D, T] = meshgrid(d, t);
[X, Y] = pol2cart(D, T);

% initial plot
fig = figure();
% ax_polar = polar([0 pi], [0 recorded_seconds], 'k.');
ax_polar = polar([0 pi], [0 recorded_seconds]);
xlabel('Time t/s (10=now)');
ylabel('Direction d/degrees');
hold('on');
contourf(X, Y, all_p);

% maximise the size of the plot
axis('tight');
ylim([0 recorded_seconds]);

% add a colorbar, and make sure it has the right limits
set(gca, 'clim', [0 1]);
colorbar();

% Get the handle to the contour plot now, because the handles as returned by
% contour() above always get deleted for some reason.
ax = get(gca, 'children');
colormap(fig, 'hot');
set(fig, 'colormap', flipud(get(fig, 'colormap')));
set(ax(1), 'edgecolor', 'none');

% update the figure in a loop
alpha = 0.1;
while true
    p = mha_get(mha_handle, 'mha.mhachain.db.mhachain.doasvm_mon.p');

    all_p(2:end) = all_p(1:end-1);
    all_p(1, :) = p/max(p(:))*alpha + all_p(2,:)*(1-alpha);

    [p_max, p_idx] = max(all_p, [], 2);
    [x, y] = pol2cart(d(p_idx), t);

    % set(ax_polar, 'XData', x);
    % set(ax_polar, 'yData', y);
    set(ax(1), 'ZData', all_p);

    % implicitly calls drawnow
    pause(spf);
end
