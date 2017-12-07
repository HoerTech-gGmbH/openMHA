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

clear
close all
clc

% Main MHA path
working_dir = '../../';

%% init
addpath ([working_dir 'tools/mfiles/']);
 
if isoctave
  pkg load signal;
end
 %% signal settings
[vX,fs] = audioread('../../configurations/AudioFiles/male_long.wav');
vX = vX./sqrt(mean(vX.^2))*10^(-20/20);
lenX=length(vX);

%% feedback path settings
load('example_afc');
vH = maH(:,1) - mean(maH(:,1));  % feedback path and remove mean value
vHphone = maH(:,13) - mean(maH(:,13)); % feedback path with phone in close distance
Lh = length(vH);    % length of feedback path

%% general settings for feedback cancellation
G = 10^(15/20);      % gain of the forward path
dG = 0.006*fs;      % delay of the forward path
Lhhat = 25;         % adaptive filter length
mu = 0.0002;        % step size
P = 21;             % prediction order

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
    vAc = cResults{idx,1}{1,1}(2:end);
else
    vAc = [];
end
vBc = cResults{idx,1}{1,2}(1:end);

% high-pass filtering
ff=fir1(64,.01,'high'); % high-pass filtering of signal, recommended by Sven E. Nordholm
vX=filter(ff,1,vX); % high-pass filtering...


%% buffer settings for feedback cancellation
vY = zeros(lenX,1);     % microphone signal
vF = zeros(lenX,1);     % feedback signal
vU = zeros(lenX,1);     % loudspeaker signal
vG = [zeros(dG,1); G];  % forward path
Lg = length(vG);        % length of forward path


%% buffer settings for filtering
vEfullband = zeros(lenX,1);             % fullband error signal
vUbuffer = zeros(Lh,1);                 % buffer for input signal
vUbufferCP = zeros(Lh,1);               % buffer for cp filtered input signal
vEbuffer = zeros(Lg,1);                 % buffer for error signal
vWfull = zeros(Lhhat,1);                    % fullband filter
vUbufferPrew = vUbuffer;
vUbufferPrewCP = vUbufferCP;
vEbufferPrew = vEbuffer;
lambda = 0.99;          % smoothing constant for power estimate
Power = 0;


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
fu1      = zeros(P, 1) + epsi;   	% forward linear predictor vector for u1 the driving input signal
bu1      = zeros(P, 2) + epsi;  	% backward linear predictor vector for u1 the driving input signal

% performance measures
vMisalignment = zeros(lenX,1);
vMSG = zeros(lenX,1);

vHFreq = freqz(vH,1,2048);


h=waitbar(0,'Processing Signal...');
%% process signal loop
for kk = 1:lenX
    
    if kk == lenX/2
        vH = vHphone;
        vHFreq = freqz(vH,1,2048);
    end
    
    if mod(kk,floor(lenX/100)) == 0 
      waitbar(kk/lenX,h,'Processing Signal...');
    end
    % update loudspeaker signal
    vU(kk) = vG(end)'*vEbuffer(Lg);
    
    % update buffer
    vUbuffer = [vU(kk); vUbuffer(1:end-1)];
    
    % compute feedback signal
    vF(kk) = vH'*vUbuffer(1:Lh);
    
    % compute microphone signal
    vY(kk) = vX(kk) + vF(kk);
    
    % filter loudspeaker signal with CP filter
    if ~isempty(vAc)
        UCP = vBc'*vUbuffer(1:Nzc+1) - vAc'*vUbufferCP(1:Npc);
    else
        UCP = vBc'*vUbuffer(1:Nzc+1);
    end
    vUbufferCP = [UCP; vUbufferCP(1:end-1)];
    
    % compute fullband apriori error
    vEfullband(kk,1) = vY(kk) - vWfull'*vUbufferCP(1:Lhhat);
    vEbuffer = [vEfullband(kk); vEbuffer(1:end-1)];
    
    % calculate LPC use Burg-Lattice
    f(1)    = vEfullband(kk,1);  
    b(1,1)  = vEfullband(kk,1);
    fx(1)   = vU(kk);  
    bx(1,1) = vU(kk);
    fe_n(1) = vEfullband(kk,1);  
    be_n(1,1) = vEfullband(kk,1);
    for m = 2: P
            % Burg Lattice Algorithm
            dm(m-1) = lam * dm(m-1) + (1-lam) * (f(m-1)^2 + b(m-1,2)^2);
            nm(m-1) = lam * nm(m-1) + (1-lam) * (-2)*(f(m-1)*b(m-1,2));
            km(m,kk) = nm(m-1) / (dm(m-1));
            
            % Adaptive Lattice Predictor
            f(m)    = f(m-1) + km(m,kk)*b(m-1,2);
            b(m,1)  = b(m-1,2) + km(m,kk)*f(m-1);
            
            % perform filtering using the km from last iteration
            if kk >= 2
                % Weighted Lattice Predictor for Updating Input Vector
                fx(m)    = fx(m-1) + km(m,kk-1)*bx(m-1,2);
                bx(m,1)  = bx(m-1,2) + km(m,kk-1)*fx(m-1);

                % Lattice Predictor for Updating Error Vector
                fe_n(m)    = fe_n(m-1) +  km(m,kk-1) * be_n(m-1,2);
                be_n(m,1)  = be_n(m-1,2) + km(m,kk-1) * fe_n(m-1);

            else
                fx(m)    = fx(m-1);
                bx(m,1)  = bx(m-1,2);

                %%% Lattice Predictor for Updating Error Vector of MD-NLMS
                fe_n(m)    = fe_n(m-1);
                be_n(m,1)  = be_n(m-1,2);

            end
    end
    b(:,2)  = b(:,1);
    bx(:,2) = bx(:,1);
    be_n(:,2) = be_n(:,1);
    bu1(:,2) = bu1(:,1);
    
    % prewhitened buffer
    vUbufferPrew = [fx(end); vUbufferPrew(1:end-1)];
    
    % filter loudspeaker signal using common part filter
    if ~isempty(vAc)
        UPrewCP = vBc'*vUbufferPrew(1:Nzc+1) - vAc'*vUbufferPrewCP(1:Npc);
    else
        UPrewCP = vBc'*vUbufferPrew(1:Nzc+1);
    end
    vUbufferPrewCP = [UPrewCP; vUbufferPrewCP(1:end-1)];
    
    % update adaptive filter
    % only if prewhitened buffers are filled use the prewhitening
    if kk >= dG+160 % 160 is integration constant from lattice filter
        vWfull = vWfull + mu./(vUbufferPrewCP(1:Lhhat)'*vUbufferPrewCP(1:Lhhat)+1e-10)*vUbufferPrewCP(1:Lhhat)*fe_n(end);

    else
    end
    
    vComplete = filter(conv(vBc,vWfull),[1;vAc],[1; zeros(Lh-1,1)]);
    vMisalignment(kk) = 10.*log10(norm(vH-vComplete).^2/norm(vH).^2);
    vHhatFreq = freqz(conv(vWfull,vBc),[1;vAc],2048);
    vMSG(kk) = 10.*log10(1./max(abs(vHFreq-vHhatFreq).^2));
    if ~mod(kk,3200)
        figure(1001); 
        plot(vH)
        hold all
        plot(vComplete, 'r');
        hold off
        drawnow
        
        figure(2002)
        plot((0:lenX-1)/fs,vMisalignment)
        drawnow
        
        figure(3003)
        plot((0:lenX-1)/fs,vMSG)
        drawnow
    end
    
end
% avoid clipping of signals when saving to files 
maxVal = max(abs([vY(:);vF(:);vU(:)]))./.99;

audiowrite('microphone.wav',vY./maxVal,fs);
audiowrite('feedback.wav',vF./maxVal,fs);
audiowrite('loudspeaker.wav',vU./maxVal,fs);

close(h);
