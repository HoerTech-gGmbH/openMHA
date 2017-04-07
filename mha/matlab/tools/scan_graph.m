function sGraph = scan_graph( mha, dotname, showall )
% scan_graph - scan hierarchical structure of a MHA instance
%
% sGraph = scan_graph( mha, dotname, showall )
  if nargin < 1
    mha = [];
  end
  if nargin < 2
    dotname = 'temp.dot';
  end
  if nargin < 3
    showall = false;
  end
  mha = mha_ensure_mhahandle( mha );
  sGraph = struct;
  sGraph.nodes = cell(0,2);
  sGraph.leaves = cell(0,2);
  sGraph.connections = cell(0,2);
  sGraph.pconnections = cell(0,2);
  sGraph.aconnections = cell(0,2);
  sGraph.chains = cell(0,1);
  sGraph = scan_sub( mha, '', sGraph, '', struct('id','') );
  write_dot_file( sGraph, dotname, showall );
  system(sprintf('dot -Tpng %s > %s.png',dotname,dotname(1:end-4)));
  
  
function sGraph = scan_sub( mha, prefix, sGraph, parent, parentinfo )
  info = mha_getinfo(mha,prefix);
  info.fullpath = prefix;
  info.cfgname = prefix;
  info.cfgname(1:max(find(info.cfgname=='.'))) = [];
  info.dll = '';
  if strcmp(info.cfgname,'identity')
    parent
    parentinfo
  end
  if isfield(parentinfo,'chain_names')
    idx = strmatch(info.cfgname,parentinfo.chain_names);
    if ~isempty(idx)
      info.dll = parentinfo.chain_dlls{idx};
    end
  end
  if strcmp(info.id,'mhachain')
    algo_names = mha_get(mha,[prefix,'.algos']);
    algo_dlls = algo_names;
    longalgo_names = algo_names;
    for k=1:numel(algo_names)
      if ~isempty(find(algo_names{k}==':'))
	algo_names{k}(1:max(find(algo_names{k}==':'))) = [];
      end
      if ~isempty(find(algo_dlls{k}==':'))
	algo_dlls{k}(max(find(algo_dlls{k}==':')):end) = [];
      end
      info.chain_names = algo_names;
      info.chain_dlls = algo_dlls;
      longalgo_names{k} = mhapath2dot([prefix,'.',algo_names{k}]);
    end
    for k=2:numel(algo_names)
      sGraph.aconnections(end+1,:) = ...
	  {[longalgo_names{k-1},''],[longalgo_names{k},'']};
    end
    sGraph.aconnections(end+1,:) = ...
    	{[mhapath2dot(prefix),':sw'],[longalgo_names{1},':w']};
    sGraph.aconnections(end+1,:) = ...
    	{[longalgo_names{end},':e'],[mhapath2dot(prefix),':se']};
    sGraph.chains{end+1} = longalgo_names;
  end
  if strcmp(info.id,'split')
    info.plugin = true;
    algo_names = mha_get(mha,[prefix,'.algos']);
    algo_dlls = algo_names;
    longalgo_names = algo_names;
    for k=1:numel(algo_names)
      if ~isempty(find(algo_names{k}==':'))
	algo_names{k}(1:max(find(algo_names{k}==':'))) = [];
      end
      if ~isempty(find(algo_dlls{k}==':'))
	algo_dlls{k}(max(find(algo_dlls{k}==':')):end) = [];
      end
      info.chain_names = algo_names;
      info.chain_dlls = algo_dlls;
      longalgo_names{k} = mhapath2dot([prefix,'.',algo_names{k}]);
    end
    for k=1:numel(algo_names)
      sGraph.aconnections(end+1,:) = ...
    	  {[mhapath2dot(prefix),':sw'],[longalgo_names{k},':nw']};
      sGraph.aconnections(end+1,:) = ...
    	  {[longalgo_names{k},':ne'],[mhapath2dot(prefix),':se']};
    end
    sGraph.chains{end+1} = longalgo_names;
  end
  if strcmp(info.id,'altplugs')
    algo_names = mha_get(mha,[prefix,'.plugs']);
    algo_dlls = algo_names;
    longalgo_names = algo_names;
    for k=1:numel(algo_names)
      if ~isempty(find(algo_names{k}==':'))
	algo_names{k}(1:max(find(algo_names{k}==':'))) = [];
      end
      if ~isempty(find(algo_dlls{k}==':'))
	algo_dlls{k}(max(find(algo_dlls{k}==':')):end) = [];
      end
      info.chain_names = algo_names;
      info.chain_dlls = algo_dlls;
    end
  end
  if strcmp(info.type,'parser')
    %if (~strcmp(info.cfgname,'mhaconfig_in')) && (~strcmp(info.cfgname,'mhaconfig_out'))
    entries = mha_mha2matlab( 'vector<string>', info.entries );
    info.plugin = false;
    if strmatch('mhaconfig_in',entries,'exact')
      info.plugin = true;
      info.mhaconfig_in = mha_get(mha,[prefix,'.mhaconfig_in']);
      info.mhaconfig_out = mha_get(mha,[prefix,'.mhaconfig_out']);
    end
    if ~isempty(prefix)
      sGraph.nodes(end+1,:) = {mhapath2dot(prefix),info};
      if ~isempty(parent) && ~strcmp(parentinfo.id,'mhachain') && ~strcmp(parentinfo.id,'split')
	sGraph.pconnections(end+1,:) = {mhapath2dot(parent), ...
		    mhapath2dot(prefix)};
      end
    end
    for e = entries
      if (~strcmp(e{:},'mhaconfig_in')) && (~strcmp(e{:},'mhaconfig_out'))
	if isempty( prefix )
	  sSub = e{:};
	else
	  sSub = [prefix,'.',e{:}];
	  sGraph.connections(end+1,:) = {...
	      mhapath2dot(prefix),...
	      mhapath2dot(sSub)...
		   };
	end
	sGraph = scan_sub( mha, sSub, sGraph, prefix, info );
      end
    end
    %end
  else
    sGraph.leaves(end+1,:) = {mhapath2dot(prefix),info};
  end
  
function sdot = mhapath2dot( smha )
  sdot = strrep( smha, '.', '_');
  
function write_dot_file( sGraph, fname, showall )
  f = fopen(fname,'w');
  fprintf(f,'digraph mha {\n');
  for k=1:size(sGraph.nodes,1)
    %style="filled", fillcolor="lightblue"
    sLabel = sGraph.nodes{k,2}.cfgname;
    if ~isempty(sGraph.nodes{k,2}.id)
      sLabel = sprintf('%s\\n(%s)',sLabel,sGraph.nodes{k,2}.id);
    else
      if ~isempty(sGraph.nodes{k,2}.dll)
	sLabel = sprintf('%s\\n(%s)',sLabel,sGraph.nodes{k,2}.dll);
      end
    end
    if isfield(sGraph.nodes{k,2},'mhaconfig_in')
      sLabel = sprintf('%s\\n\\ni: %dc/%d/%g Hz\\no: %dc/%d/%g Hz',sLabel,...
		       sGraph.nodes{k,2}.mhaconfig_in.channels,sGraph.nodes{k,2}.mhaconfig_in.fragsize,sGraph.nodes{k,2}.mhaconfig_in.srate,...
		       sGraph.nodes{k,2}.mhaconfig_out.channels,sGraph.nodes{k,2}.mhaconfig_out.fragsize,sGraph.nodes{k,2}.mhaconfig_out.srate);
    end
    fprintf(f,'  %s [label="%s", shape="box"',...
	    sGraph.nodes{k,1},sLabel);
    if sGraph.nodes{k,2}.plugin
      fprintf(f,', style="filled", fillcolor="lightblue"');
    end
    fprintf(f,'];\n');
  end
  if showall
    for k=1:size(sGraph.leaves,1)
      fprintf(f,'  %s [label="%s"];\n',...
	      sGraph.leaves{k},...
	      sGraph.leaves{k,2}.cfgname);
    end
    for k=1:size(sGraph.connections,1)
      fprintf(f,'  %s -> %s [penwidth=1,arrowhead="none"];\n',sGraph.connections{k,1},sGraph.connections{k,2});
    end
  end
  for k=1:size(sGraph.pconnections,1)
    fprintf(f,'  %s -> %s:n [penwidth=1,arrowhead="none"];\n',sGraph.pconnections{k,1},sGraph.pconnections{k,2});
  end
  for k=1:size(sGraph.aconnections,1)
    fprintf(f,'  %s -> %s [penwidth=3,arrowhead="normal",color="#A01010"];\n',sGraph.aconnections{k,1},sGraph.aconnections{k,2});
  end
  for k=1:numel(sGraph.chains)
    fprintf(f,'  { rank="same";\n');
    for k1=1:numel(sGraph.chains{k})
      fprintf(f,'      %s;\n',sGraph.chains{k}{k1});
    end
    fprintf(f,'  }\n');
  end
  fprintf(f,'}\n');
  fclose(f);