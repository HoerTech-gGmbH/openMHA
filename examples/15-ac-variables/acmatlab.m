%% 15-ac-variables
% This examples should give you an introduction how to use AC-Variables in
% combination with MATLAB

%% clear,close all, clc

clc;clear;close all;

%% Check Operating System and run Path Settings 

if ismac
    % Code to run on Mac platform
    setenv('PATH', [getenv('PATH') ':/usr/local/bin']);
    addpath('/usr/local/lib/openmha/mfiles/')
    javaaddpath('/usr/local/lib/openmha/mfiles/mhactl_java.jar')    
elseif isunix
    % Code to run on Linux platform
    setenv('LD_LIBRARY_PATH','') % Set LD_LIBRARY_PATH to empty
    addpath('/usr/lib/openmha/mfiles')
    javaaddpath('/usr/lib/openmha/mfiles/mhactl_java.jar')
elseif ispc
    % Code to run on Windows platform
    setenv('LD_LIBRARY_PATH','') % Set LD_LIBRARY_PATH to empty
    addpath('C:\Program Files\openMHA\mfiles')
    javaaddpath('C:\Program Files\openMHA\mfiles\mhactl_java.jar')
else
    disp('Platform not supported')
end

%% Starting openMHA and read in configuration file 

%Start openMHA process
openmha = mha_start; 
% Read configuration file
mha_query(openmha,'','read:coherence_live.cfg');
% Start configuration file
mha_set(openmha,'cmd','start')

%% Label center frequencies and set gain factor of ac_proc

% Label center frequencies
freqs=mha_get(openmha,'mha.overlapadd.mhachain.coherence.cf'); 
% Set gain factor in dB - default value was choosen to be 6
mha_set(openmha,'mha.overlapadd.mhachain.coh_gain.gain.gains',6);

%% Plotting Coherence Data before and after gain application of ac_proc

fig = figure;
box on
xlabel('Center Frequency / Hz');
ylabel('Coherence');
grid on
ylim([0 2]);

hold on
while true
    cla;% Read in coherence before applying gain factor
    coh=mha_get(openmha,'mha.overlapadd.mhachain.acmon.coherence_rcoh');
    % Read in coherence after applying gain factor
    coh_ac_proc=mha_get(openmha,'mha.overlapadd.mhachain.acmon.coh_gain');
    plot(freqs,coh);
    hold on
    plot(freqs,coh_ac_proc);
    legend('Coherence without gain','Coherence with gain applied');
    drawnow
    if ~ishghandle(fig)
        mha_set(openmha,'cmd','quit')
        break
    end
end
