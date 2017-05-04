function temp( fname, ftype, vfreq, sOvlType )
  if nargin < 1
    fname = mfilename;
  end
  if nargin < 2
    ftype = 'edge';
  end
  if nargin < 3
    vfreq = [2000 6667 11333 16000;...
	     2000 4000 8000 16000];
  end
  if nargin < 4
    sOvlType = {'hanning','linear','rect'};
  end
  fh = figure;
  sAx.dx = 0.012;
  sAx.dy = 0.016;
  sAx.lm = 0.12;
  sAx.bm = 0.1;
  sAx.tm = 0.06;
  sAx.fontsize = 18;
  sFreqScale = {'linear','log'};
  nx = length(sFreqScale);
  ny = length(sOvlType);
  sAx.w = (1-sAx.lm-(nx-1)*sAx.dx)/nx;
  sAx.h = (1-(sAx.bm+sAx.tm)-(ny-1)*sAx.dy)/ny;
  vax = zeros(ny,nx);
  for kovl = 1:ny
    for kscale = 1:nx
      vax(kovl,kscale) = axes('Position',[...
	  (kscale-1)*(sAx.dx+sAx.w)+sAx.lm ...
	  (kovl-1)*(sAx.dy+sAx.h)+sAx.bm ...
	  sAx.w sAx.h]);
      plot_fftfb_shapes(ftype,vfreq(kscale,:),sOvlType{kovl},sFreqScale{kscale});
      if kovl == ny
	title(sprintf('symmetry on %s scale',sFreqScale{kscale}),...
	      'fontsize',sAx.fontsize);
      end
      if kovl == 1
	xlabel('frequency / Hz',...
	       'fontsize',sAx.fontsize);
      end
      if kscale == 1
	ylabel(sOvlType{kovl},'fontsize',sAx.fontsize);
      end
    end
  end
  set(vax(:),...
      'Box','on',...
      'xtickmode','manual',...
      'ytickmode','manual',...
      'xtick',[4:4:20],...
      'ytick',[-30:10:0],...
      'xticklabel',{''},...
      'yticklabel',{''},...
      'xlim',[0 22.05],...
      'ylim',[-33 3],...
      'fontsize',sAx.fontsize...
      );
  set(vax(1,:),'xticklabel',{'4','8','12','16','20k'});
  set(vax(:,1),'yticklabel',{'-30','-20','-10','0 dB'});
  saveas(fh,fname,'eps');

function plot_fftfb_shapes(ftype,vf,ovltype,freqscale)

  if strcmp(computer,'GLNXA64')
    platform = 'x86_64';
  else
    platform = 'i686';
  end

  addpath('../../matlab/tools');
  %addpath(['../../matlab/', platform, '-linux-gcc-5']);
  if isempty(my_getenv('MHA_INSTALL_DIR'))
      setenv('MHA_INSTALL_DIR',['../../frameworks/', platform, '-linux-gcc-5']);
  end
  mha.fragsize = 11025;
  mha.srate = 44100;
  mha.nchannels_in = 1;
  mha.mhalib = 'fftfilterbank';
  mha.mha.fftlen = 22050;
  mha.mha.ftype = ftype
  mha.mha.f = vf;
  mha.mha.ovltype = ovltype;
  mha.mha.fscale = freqscale;
  mha.mha.phasemodel = 'linear';
  %mha.mha.phasemodel = 'minimal';
  %dt = mha.fragsize;
  dt = 0;
  n = 2;
  len = mha.mha.fftlen;
  x_test = realifft(exp(i*(rand(len/2+1,1) - 0.5)*2*pi));
  x_test = zeros(size(x_test));
  x_test(1) = 1;
  x_test = [repmat(x_test,[n 1]);zeros(dt,1)];
  %x = repmat(x_test,[1 3]);
  x = x_test;
  k = 1;
  [H1, H2, H3, Hsum] = get_h(mha, x, dt, len, n );
  vf = reshape(mha.srate*([1:length(H1)]-1)/len,size(H2));
  p_vf = [0:mha.srate/2000:mha.srate/2];
  hold on;
  y_patch = 20*log10(interp1( vf, H2, p_vf, 'linear','extrap' ));
  y_patch(1) = -33;
  y_patch(end) = -33;
  patch(0.001*p_vf, max(-33,y_patch),...
	[0.9 0.9 0.9],'linewidth',2);
  h = plot(0.001*p_vf, 20*log10(interp1( vf, H1, p_vf, 'linear','extrap')),'k-',...
	   0.001*p_vf, 20*log10(interp1( vf, H3, p_vf, 'linear','extrap')),'k-');
  if 0
    plot(0.001*p_vf, 20*log10(interp1( vf, Hsum, p_vf, 'linear','extrap')),...
	 'k--','linewidth',0.6);
  end
  %set(h,'linewidth',1.6);
  plot(minmax(0.001*p_vf),20*log10([0.5 0.5]),'k:','linewidth',1.6);
  
function [H1,H2,H3,Hsum] = get_h( mha, x, dt, len, n )
  y = mha_process( mha, x );
  xpl = x(1:end-dt,1);
  y1 = y(dt+1:end,1);
  y2 = y(dt+1:end,2);
  y3 = y(dt+1:end,3);
  xpl = mean(reshape(xpl,len,n),2);
  y1 = mean(reshape(y1,len,n),2);
  y2 = mean(reshape(y2,len,n),2);
  y3 = mean(reshape(y3,len,n),2);
  ysum = y1 + y2 + y3;
  X = realfft(xpl);
  Y1 = realfft(y1);
  Y2 = realfft(y2);
  Y3 = realfft(y3);
  Ysum = realfft(ysum);
  H1 = abs( Y1 ./ X );
  H2 = abs( Y2 ./ X );
  H3 = abs( Y3 ./ X );
  Hsum = abs( Ysum ./ X );
  

function lines_ = split_data_into_lines(data)
% function lines = split_data_into_lines(data)
%
% split the string <data> into <lines> 
  lines_ = {};
  line_delimiter = [char(13), char(10)]; % recognized end-of-line characters
  
  % data given as string
  while length(data)
    [line_, data] = strtok(data, line_delimiter);
    if (~isempty(line_) & ~all(isspace(line_)))
      lines_{length(lines_)+1} = line_;
    end
  end

  

