function hp = gainruleplot( rule_name, sAud, vF, splot, bCreateFig )
% GAINRULEPLOT - plot IO charactaristics of a given gain rule
%
% Usage:
% gainruleplot( rule_name )
%  plot gain rule with 50 dB flat hearing loss
%
% gainruleplot( rule_name [, htl|sAud [, f [, splot, bCreateFig ] ] ] )
%  plot gain rule with a given flat hearing loss or audiogram
%
% Options:
% - rule_name : name of gain rule
% - htl/sAud  : flat hearing loss or audiogram
%               (default: 50dB flat)
% - f         : frequency vector for IO function plot
% - splot     : name of plot: target, iofun, compr, all (default)
% - bCreateFig : create figure/axes (default: 1)
%
% gainruleplot( rule_name, sAud )
%  plot gain rule with a given audiogram
%
% Author: Giso Grimm
% Date: 2009
  ;
  % todo: Use multifit for target gain estimation
  if nargin < 2
    sAud = [];
  end
  if isempty(sAud)
    sAud = 50;
  end
  if nargin < 3
    vF = [250,500,1000,2000,4000,8000];
  end
  if nargin < 4
    splot = 'all';
  end
  if nargin < 5
    bCreateFig = 1;
  end
  if iscellstr( rule_name )
    for k=1:numel(rule_name)
      gainruleplot(rule_name{k},sAud,vF,splot,k==1);
    end
    return;
  end
  if isreal(sAud)
    libaudprof();
    sAud = audprof.audprof_flat( sAud );
    %htl = sAud;
    %sAud = struct;
    %sAud.frequencies = [250,500,1000,2000,4000,8000];
    %for ch='lr'
    %  sAud.(ch).htl = htl*ones(size(sAud.frequencies));    
    %  sAud.(ch).ucl = inf*sAud.(ch).htl;
    %end
    %sAud.id = '50 dB flat';
    %sAud.client_id = 'dummy';
  end
  %sCfg.frequencies = 1000*2.^[-4:0.1:4];
  sCfg.frequencies = 1000*2.^[-4:1/3:4];
  sCfg.edge_frequencies = 1000*2.^[-4-1/6:1/3:4+1/6];
  sCfg.levels = [-10:1:110];
  sCfg.side = 'l';
  sGt = feval(['gainrule_',rule_name],sAud,sCfg);
  sLev = sGt;
  for ch=sCfg.side
    for k=1:length(sCfg.levels)
      sLev.(ch)(k,:) = sLev.(ch)(k,:) + sCfg.levels(k);
    end
  end
  vLev = [25,45,65,85,105];
  sGain = target_fun(sGt,sCfg,vLev);
  if strcmp(splot,'all')||strcmp(splot,'target')
    if bCreateFig
      figure('Name',['target function ',rule_name]);
    end
    hp = plot(sCfg.frequencies,sGain.l,'-','linewidth',2);
    kf = round(length(sCfg.frequencies)/2);
    for l=1:length(vLev)
      text(sCfg.frequencies(kf),sGain.l(l,kf),sprintf('%g dB',vLev(l)),...
	   'VerticalAlignment','bottom','Fontweight','bold');
    end
    hold on;
    if bCreateFig
      set(gca,'XScale','log',...
	      'XTick',round(1000*2.^[-4:4]),...
	      'XLim',minmax(sCfg.frequencies).*[0.9,1/0.9],...
	      'YLim',minmax(sGain.l(:)')+[-2,2],...
	      'FontSize',14);
      xlabel('frequency / Hz');
      ylabel('Gain / dB');
      title(['target gain for LTASS, gainrule ',rule_name],...
	    'Interpreter','none');
    end
  end
  if strcmp(splot,'all')||strcmp(splot,'iofun')
    if bCreateFig
      figure('Name',['IO function ',rule_name]);
    end
    idx = interp1(sCfg.frequencies,1:length(sCfg.frequencies),vF,'nearest');
    if bCreateFig
      plot(minmax(sCfg.levels),minmax(sCfg.levels),'k-');
      hold on;
    end
    hp = plot(sCfg.levels,sLev.l(:,idx),'-','linewidth',2);
    if bCreateFig
      csLeg = {};
      for k=1:length(idx)
	csLeg{end+1} = sprintf('%g Hz',vF(k));
      end
      set(gca,'FontSize',14,'XLim',minmax(sCfg.levels));
      legend(hp,csLeg,'location','SouthEast','fontsize',10);
      xlabel('Input level / dB');
      ylabel('Output level / dB');
      title(['Input-Output function, gainrule ',rule_name],...
	    'Interpreter','none');
    end
  end
  if strcmp(splot,'all')||strcmp(splot,'comp')
    if bCreateFig
      figure('Name',['Compression rate ',rule_name]);
    end
    %plot(minmax(sCfg.levels),minmax(sCfg.levels),'k-');
    %hold on;
    hp = plot(sCfg.levels(1:end-1),...
	      1./(diff(sLev.l(:,idx))./repmat(diff(sCfg.levels)',[1,length(idx)])),...
	      '-','linewidth',2);
    if bCreateFig
      set(gca,'FontSize',14,'XLim',minmax(sCfg.levels),'YLim',[0 6]);
      legend(hp,csLeg,'location','SouthEast','fontsize',10);
      xlabel('Input level / dB');
      ylabel('compression rate');
      title(['compression rate, gainrule ',rule_name],...
	    'Interpreter','none');
    end
  end
  
function sGain = target_fun( sGt, sCfg, vLev )
  sGain = struct;
  vLT = LTASS_combined(sCfg.frequencies);
  for ch=sCfg.side
    Gt = sGt.(ch);
    vGain = zeros(length(vLev),length(sCfg.frequencies));
    for k=1:length(vLev)
      for kf=1:length(sCfg.frequencies)
	vGain(k,kf) = interp1(sCfg.levels,Gt(:,kf),vLev(k)-vLT(kf),'linear','extrap');
      end
    end
    sGain.(ch) = vGain;
  end
  
function L = LTASS_combined( vF )
  vFin = [63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000];
  vLTASS_combined = 70-[38.6, 43.5, 54.4, 57.7, 56.8, 60.2, 60.3, 59.0, ...
		    62.1, 62.1, 60.5, 56.8, 53.7, 53.0, 52.0, 48.7, 48.1, 46.8, 45.6, 44.5, 44.3, 43.7, 43.4, 41.3, 40.7];
  L = interp1(log(vFin),vLTASS_combined,log(vF),'linear','extrap');
  
function mm = minmax(x)
  mm = [min(x),max(x)];  