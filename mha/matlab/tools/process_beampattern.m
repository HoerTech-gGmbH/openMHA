function sData = process_beampattern( hMHA, sTag, vHRIRChannels, vAz, ...
				      sHRIRPath, sHRIRPattern, ...
				      iOutChannel, vTestSig )
% PROCESS_BEAMPATTERN - measure directivity of a running MHA instance
%
% sData = process_beampattern( hMHA, sTag [, vHRIRChannels, vAz, ...
%                              sHRIRPath, sHRIRPattern, ...
%                              iOutChannel, vTestSig ] )
%
% hMHA : MHA handle
% sTag : title of plot
% vHRIRChannels : MHA input channels in HRIR database
% vAz : list of azimuths in degree
% sHRIRPath : Path to HRIR database
% sHRIRPattern : pattern to match azimuth dependent names (see printf)
% iOutChannel : output channel
% vTestSig : test signal
  if nargin < 8
    vTestSig = [];
  end
  if nargin < 7
    iOutChannel = 1;
  end
  if nargin < 6
    sHRIRPattern = 'anechoic_distcm_300_el_0_az_%d.wav';
  end
  if nargin < 5
    sHRIRPath = '/home/giso/HRIR_database_wav/hrir/anechoic/';
  end
  if nargin < 4
    vAz = -175:5:180;
  end
  if nargin < 3
    vHRIRChannels = 1:2;
  end
  sData.fields = {'az','tag',...
		  '125','177','250','354','500','707','1000','1414','2000','2828','4000','5657'};
  sData.values = {vAz,{sTag}};
  l = libsd();
  sCfg.hMHA = hMHA;
  sCfg.vAz = vAz;
  sCfg.sHRIRPath = sHRIRPath;
  sCfg.sHRIRPattern = sHRIRPattern;
  sCfg.vHRIRChannels = vHRIRChannels;
  sCfg.iOutChannel = iOutChannel;
  sCfg.fs = mha_get(hMHA,'srate');
  sCfg.channels = mha_get(hMHA,'nchannels_in');
  if isempty(vTestSig)
    %vTestSig = rand(round(1.25*sCfg.fs),1)-0.5;
    vTestSig = repmat(realifft(0.5*exp(i*2*pi*rand(sCfg.fs,1))),[2,1]);
  end
  sCfg.vTestSig = vTestSig;
  sCfg.vLTestSig = bandlevel( vTestSig, sCfg.fs, 1);
  sData = l.eval( sData, @eval_pattern, ...
		  'display', 1, 'structarg', 1, ...
		  'param', sCfg );
  vF = round(1000*2.^[-3:0.5:2.5]);
  sData = l.col2par(sData, 'frequency',vF,{'Lout'});
  plot_beampattern( sData );
  
function [L125,L177,L250,L354,L500,L707,L1000,L1414,L2000,L2828,L4000,L5657] = eval_pattern( sPar, sCfg )

  sHRIR = [sCfg.sHRIRPath,sprintf(sCfg.sHRIRPattern,sPar.az)];
  disp(sHRIR)
  [hrir,fs] = wavread(sHRIR);
  if fs ~= sCfg.fs
    hrir = resample(hrir,sCfg.fs,fs);
  end
  hrir = hrir(:,sCfg.vHRIRChannels);
  sOut = fftfilt(hrir,sCfg.vTestSig);
  vL = bandlevel( mha_process( sOut, sCfg ), sCfg.fs, sCfg.iOutChannel);
  vL = vL - sCfg.vLTestSig;
  L125 = vL(1);
  L177 = vL(2);
  L250 = vL(3);
  L354 = vL(4);
  L500 = vL(5);
  L707 = vL(6);
  L1000 = vL(7);
  L1414 = vL(8);
  L2000 = vL(9);
  L2828 = vL(10);
  L4000 = vL(11);
  L5657 = vL(12);

function y = mha_process( x, sCfg )
  wavwrite(x,sCfg.fs,32,'temp_in.wav');
  mha_set(sCfg.hMHA,'io.in','temp_in.wav');
  mha_set(sCfg.hMHA,'io.out','temp_out.wav');
  mha_set(sCfg.hMHA,'cmd','start');
  mha_set(sCfg.hMHA,'cmd','release');
  y = wavread('temp_out.wav');
  
function vL = bandlevel( x, fs, kch );
  Y = realfft(x(end-fs+1:end,kch));
  vL = [10*log10(mean(abs(Y(106:149)).^2)),...
	10*log10(mean(abs(Y(150:210)).^2)),...
	10*log10(mean(abs(Y(211:297)).^2)),...
	10*log10(mean(abs(Y(298:420)).^2)),...
	10*log10(mean(abs(Y(421:595)).^2)),...
	10*log10(mean(abs(Y(596:841)).^2)),...
	10*log10(mean(abs(Y(842:1189)).^2)),...
	10*log10(mean(abs(Y(1190:1682)).^2)),...
	10*log10(mean(abs(Y(1683:2378)).^2)),...
	10*log10(mean(abs(Y(2379:3364)).^2)),...
	10*log10(mean(abs(Y(3365:4757)).^2)),...
	10*log10(mean(abs(Y(4758:6727)).^2))];
