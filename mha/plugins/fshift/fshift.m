%% Setup
%test our fshift implementation from Matlab
javaaddpath ('~/workspaces/openMHA/mha/tools/mfiles/');
%% Signal Generation
y = zeros( 10000, 2 );

%% MHA Initalization
%start MHA from Matlab
mha = mha_start;
mha_set(mha,"srate","16000");
mha_set(mha,"fragsize","32");
mha_set(mha,"iolib","MHAIOParser");
mha_set(mha,"mhalib","overlapadd");
mha_set(mha,"mha.wnd.type","hanning");
mha_set(mha,"mha.fftlen","128");
mha_set(mha,"mha.wnd.len","64");
mha_set(mha,"mha.plugin_name","fshift:fshift");
mha_set(mha,"mha.fshift.df","126");
mha_set(mha,"mha.fshift.fmin","2000");
mha_set(mha,"mha.fshift.fmax","8000");
mha_set(mha,'cmd','start');

%get the dimension from the test configuration
fragsize = mha_get(mha,'fragsize');
fftlen = mha_get(mha,'mha.fftlen');

%% Test loop
%step through all fragments of input
for s=1:fragsize:size(y,1)-fragsize
   s
   frameIn = y(s:s+fragsize-1,:);
   mha_set(mha,'io.input', frameIn' );
   
   %get AC variable state for this frame
   %gccphatEst = mha_get(mha,'mha.chain.acmon.estimate');
   
end
