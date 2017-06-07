function sGt = gainrule_Ewert2010( sAud, sCfg )
% Gain prescription rule after Ewert (2010)
%
% Physiologically motivated compressive gain prescription.
%
% Author: Giso Grimm, Stephan Ewert
% Date: 2010
  ;
  % configure global parameters:
  vEqN = outer_mid_2o(sCfg.frequencies);
  max_OHC_gain = ewert_maxgain(sCfg.frequencies);
  Lmax_comp = 85.0;
  mComp = 1/4;
  for side='lr'
    % get the interpolated hearing threshold level:
    vF = [sAud.(side).htl_ac.data.f];
    HTL = [sAud.(side).htl_ac.data.hl];
    HTL = max(0,freq_interp_sh(vF,HTL,sCfg.frequencies));
    % get loudness data:
    f_hfd = [sAud.(side).acalos.f];
    m_low = freq_interp_sh(f_hfd,[sAud.(side).acalos.mlow],sCfg.frequencies);
    L_cut = freq_interp_sh(f_hfd,[sAud.(side).acalos.lcut],sCfg.frequencies);
    % calculate OHC gain loss:
    gainloss = gainloss_ewert2010a( HTL, m_low, L_cut );
    % limit gainloss to max_OHC_gain:
    gainloss = max(0,min(gainloss,max_OHC_gain));
    % calculate gain table:
    vGain = gain_fun(mComp,max_OHC_gain,gainloss,Lmax_comp,sCfg.levels,vEqN);
    sGt.(side) = vGain';
    % configure basilar membrane IO parameter:
    sBasIO = struct('gmax_n',max_OHC_gain,...
		    'gmax_i',max_OHC_gain-gainloss,...
		    'l_passive',repmat(Lmax_comp,size(max_OHC_gain)),...
		    'c_slope',repmat(mComp,size(max_OHC_gain)));
    sGt.basilar_io.(side) = sBasIO;
    sComp = struct;
    sComp.gain = sBasIO.gmax_n-sBasIO.gmax_i;
    sComp.l_kneepoint = sBasIO.l_passive + ...
	sComp.gain/(sBasIO.c_slope-1);
    sComp.c_slope = sBasIO.c_slope;
    sGt.compression.(side) = sComp;
  end
  
function vGain = gain_fun( slope, maxgain, gainloss, Lmax_comp, vLin, ...
			   vEq)
% define compression characteristics by three line segments:
% a) linear at low levels
% b) compressive with 'slope'
% c) linear at levels above 'Lmax_comp'
% the lower knee point is determined by 'maxgain', 'gainloss' and 'slope'
  L1_nh = Lmax_comp - maxgain./(1-slope);
  L1_sh = Lmax_comp - (maxgain-gainloss)./(1-slope);
  vGain = gain_interp(L1_nh,L1_sh,gainloss,vLin,vEq);
  
function vGain = gain_interp( Lin1, Lin2, G1, vLin, vEq )
  G2 = zeros(size(G1));
  vG_ = [G1(:),G1(:),G2(:),G2(:)];
  vLi_ = [Lin1(:)-1,Lin1(:),Lin2(:),Lin2(:)+1];
  vGain = zeros(numel(G1),numel(vLin));
  for k=1:numel(G1)
    [vLi__,idx] = unique(vLi_(k,:));
    vG__ = vG_(k,idx);
    vGain(k,:) = interp1(vLi__,vG__,vLin+vEq(k),'linear','extrap');
  end

function gainLoss = gainloss_ewert2010a(hearingLoss, m_low, l_cut);
% Based on Data from Juergens et al:
%
% hearingloss was average AFC/PTA
%
% l_cut is used as input but disabled since best stability against
% variation in AFC-PTA thresholds is reached like this.
%
% Furthermore l_cut would have to be converted to hearing level before
% entering the fit
%
% 04.05.2010 17:39
  ;
  x = [0.5824   24.3749  -8.5401];
  % R2 =  0.9163
  gainLoss = x(1)*hearingLoss + x(2)*m_low + x(3);  
  
function gainloss = gainloss_ewert( HTL, m_low, L_cut )
% rule-of-thumb
  gainloss = max( 0, HTL-10 );
  
function L = equal_loudness_contour80( vF )
  data = [ ...
      20,118;...
      30,111;...
      40,106;...
      50,102;...
      60,99;...
      80,95;...
      100,92;...
      200,85;...
      300,82;...
      400,81;...
      600,80;...
      900,80;...
      1000,80;...
      1600,82;...
      2000,80;...
      3000,78;...
      4000,79;...
      5000,84;...
      7000,90;...
      9000,93;...
      10000,93;...
      16000,85;...
      18000,84;...
      20000,90;...
	 ];
  L = interp1(log(data(:,1)),data(:,2),log(vF),'linear','extrap');

%  function [
%    max_OHC_gain = 42.5;
%  Lmin_comp = 25.0;
%  Lmax_comp = 87.0;

function vEq = outer_mid( vF )
  vfc = [125,600,775,1000,1500,1800,2250,2700,4000,4500,5000,6000, ...
	 6300,8000];
  vG = [-0.2535,12.9267,14.2480,12.7115,9.0614,8.3029,9.3162,11.1902,...
	0,-0.8349,1.1672,-4.4460,-6.0824,-19.0466];
  vEq = freq_interp_sh(vfc,vG,vF);
  
function vEq = outer_mid_2o( vF )
  %[B,A] = butter(1,[400,4000],'s');
  B = [0,3600,0];
  % Bugfix: invalid filter
  A = [1,3600,1600000];
  vEq = 20*log10(abs(freqs(B,A,vF)));
  
function vG = ewert_maxgain( vF )
  vData = [...
      62.5,8;...
      125,10;...
      250,14;...
      500,20;...
      1000,35;...
      2000,40;...
      4000,40;...
      8000,30;...
      ];
  vG = freq_interp_sh(vData(:,1),vData(:,2),vF);
