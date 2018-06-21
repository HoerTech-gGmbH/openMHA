function s = sd_eval( s, fun, varargin )
% evaluation of parameterized data
%
% Syntax:
%
% sOut = sd.eval( sIn, fun, ... )
%
% sIn  : structure containing a 'values' field (cell array, each
%        entry defines valid values in one dimension)
% fun  : evaluation function with one or more return values,
%        one input argument for each dimension, plus an optional
%        input argument for additional paramters 'par'.
%
% Optional param-value pairs:
%
% display  : flag to show progress information
% nrep     : number of re-evaluations, in case that 'fun' describes a
%            stochastic process
% brand    : bool: randomize trials
%            cell array of integer: keep given columns ordered
% structarg: pass parameter values as structure to eval function
% param    : additional parameters for 'fun'
%
% Author: Giso Grimm
% Date: 11/2006
  
  sPar = struct;
  sPar.display = 0;
  sPar.nrep = 1;
  sPar.brand = 0;
  sPar.structarg = 0;
  for k=1:2:length(varargin)-1
    sPar.(varargin{k}) = varargin{k+1};
  end
  if iscell( s )
    values = s;
    s = struct;
    s.values = values;
  end
  npar = length(s.values);
  ncond = 1;
  vnval = zeros(1,npar);
  ndata = nargout(fun);
  if ~isfield( s, 'fields' )
    s.fields = {};
    for k=1:npar
      s.fields{end+1} = sprintf('input%d',k);
    end
  end
  for k=1:ndata
    if numel(s.fields) < npar+k
      s.fields{end+1} = sprintf('output%d',k);
    end
  end
  for k=1:npar
    vnval(k) = length(s.values{k});
    ncond = ncond * vnval(k);
  end
  mpar = zeros(ncond,npar);
  kval = zeros(size(vnval));
  for kcond=1:ncond
    mpar(kcond,1:npar) = kval+1;
    kval = kval_step(kval,vnval);
  end
  mdata = zeros(ncond,ndata);
  if isnumeric(sPar.display) | islogical(sPar.display)
    if sPar.display
      h = waitbar(0,sprintf('%d conditions',ncond));
      h_tic = tic;
    end
  else
    sPar.display(0);
  end
  try
  s.data = zeros(0,npar+ndata);
  ktot = 0;
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
      if sPar.structarg
	sFunArg = struct;
      end
      for kpar=1:npar
	val = s.values{kpar}(mpar(kcond,kpar));
	if iscellstr(val)
	  val = val{:};
	end
	arg{kpar} = val;
	if sPar.structarg
	  sFunArg.(s.fields{kpar}) = val;
	end
	disp_arg = sprintf('%s%s:',disp_arg,any2string(arg{kpar}));
      end
      disp_arg = disp_arg(1:end-1);
      if ndata > 0
	sretval = sprintf('mdata(kcond,%d),',1:ndata);
	sretval = ['[',sretval(1:end-1),']='];
      else
	sretval = '';
      end
      sarg = sprintf('arg{%d},',1:npar);
      sarg = sarg(1:end-1);
      if isfield(sPar,'param')
	extrapar = ',sPar.param';
      else
	extrapar = '';
      end
      if sPar.structarg
	seval = sprintf('%sfun(sFunArg%s);',sretval,extrapar);
      else
	seval = sprintf('%sfun(%s%s);',sretval,sarg,extrapar);
      end
      eval(seval);
      if isnumeric(sPar.display) | islogical(sPar.display)
	if sPar.display
	  t_ac = toc( h_tic );
	  prog = ktot/(ncond*sPar.nrep);
	  tt = (t_ac/prog-t_ac)/(24*3600);
	  disp_arg(end+1) = ' ';
	  for kdat=1:ndata
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
    s.data = [s.data;[mpar mdata]];
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
  
function x = kval_step( x, n )
  x(1) = x(1) + 1;
  for k=1:length(x)-1
    if x(k) >= n(k)
      x(k+1) = x(k+1)+1;
      x(k) = 0;
    end
  end
  
function s = any2string( x )
  if isnumeric(x)
    s = num2str(x);
  elseif ischar(x)
    s = x;
  else
    s = '???';
  end
