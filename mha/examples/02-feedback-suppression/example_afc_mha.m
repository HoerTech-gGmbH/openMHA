% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2017 HörTech gGmbH
%
% openMHA is free software: you can redistribute it and/or modify
% it under the terms of the GNU Affero General Public License as published by
% the Free Software Foundation, version 3 of the License.
%
% openMHA is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Affero General Public License, version 3 for more details.
%
% You should have received a copy of the GNU Affero General Public License, 
% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>
%
% Reference
% H. Schepker, S. Doclo, "A semidefinite programming approach to min-max 
% estimation of the common part of acoustic feedback paths in hearing aids,"
% IEEE Trans. Audio, Speech, Lang. Process.,
% vol. 24, no. 2, Feb. 2016, pp. 366-377.
%
% Original author: Henning Schepker





clear;
close all
clc



% ATTENTION (1):
% To start MHA, set up the environment by setting up the paths
% correctly.

% ATTENTION (2):
% This example currently takes a long time to execute!  The reason for
% the slow execution speed is that we exchange single sound samples
% between octave and mha, which is inefficient.  A future release will
% fix this by integrating the feedback path simulation (which is
% currently done in octave) into the openMHA proper.

% Main MHA path
working_dir = '../../';

%% init
addpath ([working_dir 'tools/mfiles/']);

% MHA binaries path
mhapath = [working_dir '../bin/'];

% MHA binaries path
mha_config_path=[working_dir 'configurations/'];

% java path
javaaddpath( [working_dir 'tools/mfiles/mhactl_java.jar'] );

% define environment
setenv('MHA_INSTALL_DIR', mhapath );
setenv('LD_LIBRARY_PATH',[mhapath] );
setenv('MHA_CONFIG_PATH', mha_config_path);

if isoctave
  pkg load signal;
end

fs = 16000;

% Load feedback path

load('example_afc');
vH = maH(:,1) - mean(maH(:,1));
vHphone = maH(:,13) - mean(maH(:,13));
Lh = length(vH);
Lhhat = 25;         % adaptive filter length
P = 21;             % prediction order

%% Linear Prediction Parmeters & Vector
% Burg Algorithms
epsi=10^-10;                        % SMALL CONSTANT
lam     = 1-(1/160);                % forgetting factor for linear predictor
dm      = zeros(P, 1) + epsi;
nm      = zeros(P, 1) + epsi;
km      = zeros(P, Lhhat) + epsi;

f       = zeros(P, 1) + epsi;   	% adaptive forward linear predictor vector
b       = zeros(P, 2) + epsi;   	% adaptive backward linear predictor vector

fx      = zeros(P, 1) + epsi;   	% forward linear predictor vector for output signal
bx      = zeros(P, 2) + epsi;   	% backward linear predictor vector for output signal

fe_n      = zeros(P, 1) + epsi;   	% forward linear predictor vector for error signal for NLMS
be_n      = zeros(P, 2) + epsi;   	% backward linear predictor vector for error signal for NLMS

%% general settings for common part filtering
Npc = 8;
Nzc = 4;

%% find common part filters
if any(Npc == veNpcNew) && any(Nzc == veNzcNew) && any(Lhhat-1 == veNzvNew)
    idx = find(maCombinations(:,1) == Npc & maCombinations(:,2) == Nzc & maCombinations(:,3) == Lhhat-1);
elseif Npc == 0 && Nzc == 0
    
else
    error('Common part filtering not possible')
end

%% select common part filters
if Npc > 0
    vAc = single(cResults{idx,1}{1,1}(2:end));
else
    vAc = [];
end
vBc = single(cResults{idx,1}{1,2}(1:end));


%% general settings for feedback cancellation
G = 10^(15/20);      % gain of the forward path
Lhhat = 25;         % adaptive filter length
mu = 0.0002;        % step size
P = 21;             % prediction order

dG = 0.006*fs;      % delay of the forward path
vG = [zeros(dG,1); G];  % forward path
Lg = length(vG);        % length of forward path

% Load input signal
[vX,fs] = audioread('../../configurations/AudioFiles/male_long.wav');
vX = single(vX./sqrt(mean(vX.^2))*10^(-20/20));
lenX = 80*fs;     % length of input signal
ff=single(fir1(64,.01,'high')); % high-pass filtering of signal, recommended by Sven E. Nordholm
vX=single(filter(ff,1,vX)); % high-pass filtering...

%% buffer settings for feedback cancellation
vY = single(zeros(lenX,1));     % microphone signal
vF = single(zeros(lenX,1));     % feedback signal
vU = single(zeros(lenX,1));     % loudspeaker signal
vEfullband = single(zeros(lenX,1));             % fullband error signal
vEbuffer = single(zeros(Lg,1));                 % buffer for error signal

vUbuffer = single(zeros(Lh,1));                 % buffer for input signal
vUbufferPrew = single(zeros(8, 1));

% performance measures
vMisalignment = zeros(lenX,1);
vMSG = zeros(lenX,1);


%1. to start MHA from Matlab
mha = mha_start;
mha_query(mha,'','read:${MHA_CONFIG_PATH}example_afc.cfg' );


%get the length of the delay
dG = mha_get(mha, 'mha.dG.delay');
Lg = dG + 1;
%get the dimension from the test configuration
fragsize = mha_get(mha,'fragsize');
hopsize = fragsize;


%% Start MHA processing
mha_set(mha,'cmd','start');

frameIn = zeros(1, fragsize);
frameOut = zeros(1, fragsize);
vF_block = zeros(fragsize, 1);

getAC = @(name) mha_get(mha,['mha.acmon.' name]);
sqrE = @(x) sum( x(:).^2 );

vHFreq = freqz(vH,1,2048);

%% step through all fragments of input
for s=1:hopsize:length(vX)-fragsize
    
    if s == lenX/2
        vH = vHphone;
        vHFreq = freqz(vH,1,2048);
    end
    
    
    % update loudspeaker signal
    vUout = single(getAC('vU'));
    % sample processing
    % compute the feedback signal, block processing
    for i = 0:hopsize-1
        vU(s+i) = vUout(i+1);
        
        vUbuffer = [vU(s+i); vUbuffer(1:end-1)];
        vF_block(i+1) = vH'*vUbuffer(1:Lh);
    end
    
    % block processing
    vF(s:s+hopsize-1) = vF_block;
    
    % compute microphone signal
    vY(s:s+hopsize-1) = vX(s:s+hopsize-1) + vF(s:s+hopsize-1);
    frameIn = [vY(s:s+hopsize-1), frameIn(1:end-hopsize)];
    
    % Feed in the input
    mha_set(mha,'io.input', frameIn');
    
    % Read out the output signal
    frameOut = mha_get(mha,'io.output')';
    
    vWfull = single(getAC('nlms'));
    
    vComplete = filter(conv(vBc,vWfull),[1;vAc],[1; zeros(Lh-1,1)]);
    vMisalignment(s) = 10.*log10(norm(vH-vComplete).^2/norm(vH).^2);
    vHhatFreq = freqz(conv(vWfull,vBc),[1;vAc],2048);
    vMSG(s) = 10.*log10(1./max(abs(vHFreq-vHhatFreq).^2));
    
    % Plot the results every 3200 iterations
    if ~mod(s - 1,3200) && s > 1
        
        
        figure(4001);
        plot(vH)
        hold all
        plot(vComplete, 'r');
        hold off
        drawnow
        
        figure(5002)
        plot((0:lenX-1)/fs,vMisalignment)
        drawnow
        
        figure(6003)
        plot((0:lenX-1)/fs,vMSG)
        drawnow
    end
    
end

mha_set(mha,'cmd','quit');

% avoid clipping of signals when saving to files 
maxVal = max(abs([vY(:);vF(:);vU(:)]))./.99;

audiowrite('microphone.wav',vY./maxVal,fs);
audiowrite('feedback.wav',vF./maxVal,fs);
audiowrite('loudspeaker.wav',vU./maxVal,fs);
