% test our doasvm_classification implementation from Matlab

% generate or load your test signal here
x = zeros(10000, 2);

% ATTN: to start MHA, choose one of the following options

% 1. start MHA from within Matlab
mha = mha_start;
mhactl_wrapper(mha, '?read:doasvm_classification.cfg');

% 2. for debugging with Qt Creator, use the following to connect to the default
%    MHA instance
% mha = mha_ensure_mhahandle;

mha_set(mha, 'cmd', 'start');

% get the fragment and FFT sizes from the test configuration
fragsize = mha_get(mha, 'fragsize');
fftlen = mha_get(mha, 'mha.fftlen');

% step through all input fragments
for s=1:fragsize:size(x, 1) - fragsize
   disp(s);
   frameIn = x(s:s + fragsize - 1, :);
   mha_set(mha, 'io.input', frameIn');

   % get the AC variable state for this frame
   %gccphatEst = mha_get(mha, 'mha.chain.acmon.estimate');
end
