function [vL, vCF, vEF] = third_octave_power( fname, fftlen, bands_per_octave )
  if nargin < 2
    fftlen = 1024;
  end
  if nargin < 3
    bands_per_octave = 3;
  end
  [vDim,fs] = wavread(fname,'size');
  mhacfg.nchannels_in = vDim(2);
  mhacfg.fragsize = floor(fftlen/2);
  mhacfg.srate = fs;
  mhacfg.mhalib = 'overlapadd';
  mhacfg.iolib = 'MHAIOFile';
  mhacfg.mha.fftlen = fftlen;
  mhacfg.mha.wnd.type = 'hanning';
  mhacfg.mha.wnd.len = fftlen;
  mhacfg.mha.wnd.pos = 0.5;
  mhacfg.mha.wnd.exp = 1;
  mhacfg.mha.zerownd.type = 'rect';
  mhacfg.mha.smoothgains.mode = 'off';
  mhacfg.mha.plugname = 'mhachain';
  mhacfg.mha.plug.algos = {'fftfbpow','acsave'};
  mhacfg.mha.plug.fftfbpow.fscale = 'log';
  mhacfg.mha.plug.fftfbpow.ovltype = 'rect';
  mhacfg.mha.plug.fftfbpow.plateau = 0;
  mhacfg.mha.plug.fftfbpow.ftype = 'center';
  mhacfg.mha.plug.fftfbpow.f = 1000*2.^[-4:1/bands_per_octave:floor(log2(fs/2000))];
  mhacfg.mha.plug.acsave.fileformat = 'mat4';
  mhacfg.mha.plug.acsave.name = [fname,'.mat'];
  mhacfg.mha.plug.acsave.reclen = vDim(1)/fs;
  mhacfg.mha.plug.acsave.vars = {'fftfbpow'};
  mhacfg.io.in = fname;
  mhacfg.io.out = '/dev/null';
  h = mha_start;
  mha_set(h,'',mhacfg);
  mha_set(h,'cmd','start');
  mha_set(h,'mha.plug.acsave.flush',1);
  vCF = mha_get(h,'mha.plug.fftfbpow.cf')';
  vEF = mha_get(h,'mha.plug.fftfbpow.ef')';
  lfname = mha_get(h,'mha.plug.acsave.name');
  mha_set(h,'cmd','quit');
  s = load(lfname);
  vL = 10*log10(sum(10.^(0.1*s.fftfbpow),1));
  vL = reshape(vL,[vDim(2),length(vCF)])';