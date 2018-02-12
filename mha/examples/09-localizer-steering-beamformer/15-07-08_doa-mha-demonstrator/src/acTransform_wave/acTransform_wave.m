%test our acTransform_wave implementation from Matlab

%generate or load your test signal here in 'y'
y = zeros( 10000, 2 );

%ATTN: to start MHA, choose an option

%1. to start MHA from Matlab
mha = mha_start;
mhactl_wrapper( mha, '?read:acTransform_wave.cfg' );

%2. debugging with Qt Creator, use the following to connect to default MHA
%mha = mha_ensure_mhahandle;

mha_set(mha,'cmd','start');

%get the dimension from the test configuration
fragsize = mha_get(mha,'fragsize');
fftlen = mha_get(mha,'mha.fftlen');

%step through all fragments of input
for s=1:fragsize:size(y,1)-fragsize
   s
   frameIn = y(s:s+fragsize-1,:);
   mha_set(mha,'io.input', frameIn' );
   
   %get AC variable state for this frame
   %gccphatEst = mha_get(mha,'mha.chain.acmon.estimate');
   
end

