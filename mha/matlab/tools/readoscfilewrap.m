function vars = readoscfilewrap( filename )
  global vars;
  vars = struct;
  readoscfile(filename,@event_handle);

function x = event_handle( addr, x )
  global vars;
  addr(find(addr=='/')) = [];
  if isfield(vars,addr)
    ox = getfield(vars,addr);
  else
    ox = {};
  end
  ox{end+1} = x;
  vars = setfield(vars, addr, ox );
