function sData = sd_eval2( sData, fun, varargin )
% evaluation of parameterized data
%
% sData : structure containing a 'values' field (cell array, each
%         entry defines valid values in one dimension)
% fun   : evaluation function handle, of type
%         mOut = fun( sPar ) or 
%         mOut = fun( sPar, extra )
%         sPar is a structure with evaluation parameters
%         'extra' is the optional extra function parameter (see below)
%
% Optional param-value pairs:
%
% display  : flag to show progress information
% nrep     : number of re-evaluations, in case that 'fun' describes a
%            stochastic process
% brand    : bool: randomize trials
%            cell array of integer: keep given columns ordered
% param    : extra parameters for 'fun'
%
% Author: Giso Grimm
% Date: 11/2006, 3/2014
  
  sPar = struct;
  sPar.display = 0;
  sPar.nrep = 1;
  sPar.brand = 0;
  for k=1:2:length(varargin)-1
    sPar.(varargin{k}) = varargin{k+1};
  end
  npar = length(sData.values);
  ncond = 1;
  vnval = zeros(1,npar);
  for k=1:npar
    vnval(k) = length(sData.values{k});
    ncond = ncond * vnval(k);
  end
  mpar = zeros(ncond,npar);
  kval = zeros(size(vnval));
  for kcond=1:ncond
    mpar(kcond,1:npar) = kval+1;
    kval = kval_step2(kval,vnval);
  end
  if isnumeric(sPar.display) | islogical(sPar.display)
    if sPar.display
      h = waitbar(0,sprintf('%d conditions',ncond));
      h_tic = tic;
    end
  else
    sPar.display(0);
  end
  ktot = 0;
  mdata = [];
  sData.data = [];
  %% start of the evaluation loop:
  try
    for krep=1:sPar.nrep
      if iscell(sPar.brand) || sPar.brand
	if iscell(sPar.brand)
	  [tmp,vCond] = sort(rand(1,ncond));
	  [tmp,idx] = sortrows(mpar(vCond,cell2mat(sPar.brand)));
	  vCond = vCond(idx);
	else
	  [tmp,vCond] = sort(rand(1,ncond));
	end
      else
	vCond = [1:ncond];
      end
      for kcond=vCond
	ktot = ktot + 1;
	arg = cell(1,npar);
	disp_arg = '';
	sFunArg = struct;
	for kpar=1:npar
	  val = sData.values{kpar}(mpar(kcond,kpar));
	  if iscellstr(val)
	    val = val{:};
	  end
	  arg{kpar} = val;
	  sFunArg.(sData.fields{kpar}) = val;
	  disp_arg = sprintf('%s%s:',disp_arg,any2string2(arg{kpar}));
	end
	disp_arg = disp_arg(1:end-1);
	if isfield(sPar,'param')
	  tmpdata = fun( sFunArg, sPar.param );
	else
	  tmpdata = fun( sFunArg );
	end
	if isempty(mdata)
	  mdata = zeros(ncond,numel(tmpdata));
	end
	mdata(kcond,:) = tmpdata(:);
	if isnumeric(sPar.display) | islogical(sPar.display)
	  if sPar.display
	    t_ac = toc( h_tic );
	    prog = ktot/(ncond*sPar.nrep);
	    tt = (t_ac/prog-t_ac)/(24*3600);
	    disp_arg(end+1) = ' ';
	    for kdat=1:size(mdata,2)
	      disp_arg = sprintf('%s%1.3g:',disp_arg,mdata(kcond,kdat));
	    end
	    disp_arg(end) = [];
	    disp_arg = [disp_arg,' ',datestr(tt+now)];
	    disp(disp_arg);
	    waitbar(prog,h,sprintf('%d/%d conditions (ETA: %s)',ktot,ncond,datestr(tt,'HH:MM:SS')));
	  end
	else
	  sPar.display(ktot/(ncond*sPar.nrep));
	end
      end
      sData.data = [sData.data;[mpar mdata]];
    end
    if ~isfield( sData, 'fields' )
      sData.fields = {};
      for k=1:npar
	sData.fields{end+1} = sprintf('input%d',k);
      end
    end
    for k=1:size(mdata,2)
      if numel(sData.fields) < npar+k
	sData.fields{end+1} = sprintf('output%d',k);
      end
    end
    if isnumeric(sPar.display) | islogical(sPar.display)
      if sPar.display
	if ishandle(h)
	  close(h);
	end
      end
    end
  catch
    err = lasterror;
    if isnumeric(sPar.display) | islogical(sPar.display)
      if sPar.display
	close(h);
      end
    end
    rethrow(err);
  end
  
function x = kval_step2( x, n )
  x(1) = x(1) + 1;
  for k=1:length(x)-1
    if x(k) >= n(k)
      x(k+1) = x(k+1)+1;
      x(k) = 0;
    end
  end
  
function s = any2string2( x )
  if isnumeric(x)
    s = num2str(x);
  elseif ischar(x)
    s = x;
  else
    s = '???';
  end
