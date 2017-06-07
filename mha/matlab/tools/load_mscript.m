function s = load_mscript( fname )
  fname = fname_check( fname );
  eval(fname);
  clear('fname');
  snames = whos;
  s = struct;
  for k=1:length(snames)
    eval(sprintf('s.(snames(k).name) = %s;',snames(k).name));
  end
  
function fname = fname_check( fname )
  [path, name, ext] = fileparts(fname);
  try
    nargin(name);
  catch
    fname = name;
    return
  end
  error(sprintf('%s is not a script',fname));