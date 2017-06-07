function check_cpu_load( ipaddress )
% check_cpu_load - check CPU load of a running MHA (Jack audio backend)
%
% Usage:
% check_cpu_load( ipaddress )
%
% ipaddress: hostname or IP address of MHA host.
%
% Author: Giso Grimm
% Date:   6/2008
  ;
  if nargin < 1
    ipaddress = 'localhost';
  end
  mhah = mha_ensure_mhahandle( ipaddress );
  cfg = mha_get_basic_cfg_network( mhah );
  algos = mha_get(mhah,[cfg.base.altplugs,'.labels']);
  algos{end+1} = '(none)';
  sState = mha_get(mhah,'io.state');
  disp(sprintf('Checking MHA CPU load, this will take about %d minutes.',ceil(length(algos)*32/60)));
  if strcmp(mha_get(mhah,'state'),'running') 
    ;
  else
    msg = mha_get(mhah,'asyncerror');
    error(sprintf('MHA is not running:\n%s',msg));
  end
  cpuload = [];
  xruns = [];
  xruns_cfgload = [];
  xruns_netload = [];
  cpuload_cfgload = [];
  cpuload_netload = [];
  tic;
  txruns(1) = mha_get(mhah,'io.state.xruns');
  for alg=algos
    disp(sprintf('testing algorithm ''%s''...',alg{:}));
    mha_set(mhah,[cfg.base.altplugs,'.select'],alg{:});
    xr1 = mha_get(mhah,'io.state.xruns');
    pause(30);
    xr2 = mha_get(mhah,'io.state.xruns');
    xruns(end+1) = (xr2-xr1)*2;
    cpuload(end+1) = mha_get(mhah,'io.state.cpuload');
    mha_query( mhah, '', 'listid' );
    xr3 = mha_get(mhah,'io.state.xruns');
    cpuload_cfgload(end+1) = mha_get(mhah,'io.state.cpuload');
    xruns_cfgload(end+1) = (xr3-xr2);
    mha_get(mhah,'mha');
    xr4 = mha_get(mhah,'io.state.xruns');
    cpuload_netload(end+1) = mha_get(mhah,'io.state.cpuload');
    xruns_netload(end+1) = (xr4-xr3);
  end
  ttime = toc/60;
  txruns(2) = mha_get(mhah,'io.state.xruns');
  disp('-- MHA realtime state: ------------------------------------');
  for k=1:length(algos)
    alg = algos{k};
    alg(end+1:20) = ' ';
    disp(sprintf('  %s CPU=%1.1f%% (%1.1f%% cfg, %1.1f%% net) xruns=%d/min. (%d cfg, %d net)',...
		 alg,cpuload(k),cpuload_cfgload(k),cpuload_netload(k),...
		 xruns(k),xruns_cfgload(k),xruns_netload(k)));
  end
  disp(sprintf('  average xruns: %1.2f/minute (%d since MHA start)',(txruns(2)-txruns(1))/ttime,txruns(2)));
  disp(sprintf('  Real time scheduler: %s (priority: %d)',sState.scheduler,sState.priority));
  disp('-----------------------------------------------------------');
  if strcmp(mha_get(mhah,'state'),'running') 
    ;
  else
    msg = mha_get(mhah,'asyncerror');
    error(sprintf('MHA is not running:\n%s',msg));
  end
  