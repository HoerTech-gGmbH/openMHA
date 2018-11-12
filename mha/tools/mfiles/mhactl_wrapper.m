function r = mhactl_wrapper( mha_handle, query )

  if isstruct(mha_handle)
    if isfield( mha_handle, 'tcp' )
      handle = mha_handle.tcp.cmd;
      interface = @mhactl_wrapper_2;
    else
      handle = mha_handle;
      interface = @mhactl_wrapper_2;
    end
    try
      javaObject('de.hoertech.mha.control.ParserResponse');
    catch
      error(['Please add the mhactl_java.jar file to your matlab java ' ...
             'classpath.  To do this, issue >> edit classpath.txt ' ...
             '  and add the full path of the mhactl_java.jar file ' ...
             'as the last line, and save the file.  ' ...
             'You may have to start matlab as administrator to be able to ' ...
             'save this file. ' ...
             ' Then, restart matlab and check that the .jar file was added' ...
             ' to the class path by issuing >> javaclasspath' ...
             ' Note: This does not work for matlab2011a because of a bug in' ...
             ' that release. In 2011a, you should instead add the' ...
             ' mhactl_java.jar file to the dynamic java class path using' ...
             ' >> javaaddpath(full_path_to_mhactl_java.jar). Please do this' ...
             ' upon matlab startup in 2011a, as javaaddpath will in turn' ...
             ' invoke clear(''all'')']);
    end
  else
    handle = mha_handle;
    interface = @mha_plug_wrapper;
  end
  r = interface( handle, query );

function r = mhactl_wrapper_2( h, query )
  was_cell = 1;
  if ~iscell(query)
    query = {query};
    was_cell = 0;
  end
  if ~isfield(h,'timeout')
    global mhactl_timeout;
    if isempty(mhactl_timeout)
      mhactl_timeout = 50;
    end
    h.timeout = mhactl_timeout;
  end
  [r, state] = mhactl_java( h, 'eval', query );
  idx = find(state==1);
  if ~isempty(idx)
    err = sprintf('An error occured while sending to MHA server:');
    for k=idx
      err = sprintf('%s\n[%d: ''%s''] %s',err,k,query{k},r{k});
    end
    error(err);
  end
  if ~was_cell
    r = r{:};
  end

function r = mha_plug_wrapper( h, query )
  was_cell = 1;
  if ~iscell(query)
    query = {query};
    was_cell = 0;
  end
  r = {};
  for q=query
    r = {r{:} mha_plug(h, 'eval', q{:} )};
  end
  if ~was_cell
    r = r{:};
  end
  
