function mha_set( handle, field, value )
% answer = mha_set(handle, field, value)
%
% set MHA variables
%
% handle     : Handle of MHA server, e.g. for mhactl interface a
%              struct containing the fields 'host' and 'port'.
%
% field      : Name of MHA variable or parser.
%
% value      : New value of the MHA variable. If value is a struct,
%              then field has to be the name of a MHA parser.

  assignments = struct2mhacfg( value, [], field );
  mhactl_wrapper(handle,assignments);
  %if isstruct(value)
  %    fns = fieldnames(value)';
  %  for fn=fns
  %    nval = getfield(value,fn{:});
  %    if ~isstruct(nval)
  %	mha_set(handle, [field '.' fn{:}], nval);
  %    end
  %  end
  %  for fn=fns
  %    nval = getfield(value,fn{:});
  %    if isstruct(nval)
  %	mha_set(handle, [field '.' fn{:}], nval);
  %    end
  %  end
  %else
  %  type = mhactl_wrapper(handle,[field '?type']);
  %  mhaval = matlab2mhaval(type,value,field);
  %  mhactl_wrapper(handle,[field '=' mhaval] );
  %end

function mhaval = matlab2mhaval( type, value, field )
  if nargin < 3
      field = '';
  end
  mhaval = '';
  switch type
   case 'string'
    if ~ischar(value)
      error( [field ': Only character strings are accepted'] );
    end
    if min(size(value)) > 1
      error( [field ': Only single-line strings are accepted'] );
    end
    mhaval = value;
   case 'keyword_list'
    if ~ischar(value)
      error( [field ': Only character strings are accepted'] );
    end
    if min(size(value)) > 1
      error( [field ': Only single-line strings are accepted'] );
    end
    mhaval = value;
   case 'float'
    if ~( isnumeric(value) && isreal(value) )
      error( [field ': Only real numeric values are accepted for floats'] );
    end
    if prod(size(value)) ~= 1
      error( [field ': Only scalars are accepted.']);
    end
    mhaval = sprintf('%1.30g',value);
   case 'int'
    if ~( isnumeric(value) && isreal(value) )
      error( [field ': Only real numeric values are accepted for floats'] );
    end
    if prod(size(value)) ~= 1
      error( [field ': Only scalars are accepted.']);
    end
    mhaval = sprintf('%1.30g',value);
   case 'complex'
    if ~isnumeric(value)
      error( [field ': Only numeric values are accepted for floats'] );
    end
    if prod(size(value)) ~= 1
      error( [field ': Only scalars are accepted.']);
    end
    if imag(value) >= 0 
      mhaval = sprintf('(%1.30g+%1.30gi)',real(value),imag(value));
    else
      mhaval = sprintf('(%1.30g%1.30gi)',real(value),imag(value));
    end
   case 'bool'
    if prod(size(value)) ~= 1
      error( [field ': Only scalars are accepted.'] );
    end
    if value
      mhaval = 'yes';
    else
      mhaval = 'no';
    end
   case 'vector<string>'
    if ~iscell(value)
      error( [field ': Only cell strings are accepted'] );
    end
    tmp = '';
    for val=value
      if ~ischar(val{:})
	error( [field ': Only string components are accepted'] );
      end
      if ~isempty(find(val{:}==' '))
	error([field ': No whitespace is allowed in MHA string vector' ...
	       ' components.']);
      end
      tmp = sprintf('%s%s ',tmp,val{:});
    end
    mhaval = sprintf('[%s]',tmp);
   case 'vector<float>'
    if ~( isnumeric(value) && isreal(value) )
      error( [field ': Only real numeric values are accepted for floats'] );
    end
    if min(size(value)) > 1
      error( [field ': Only vectors are accepted'] );
    end
    mhaval = sprintf('[%s]',sprintf('%1.30g ',value));
   case 'vector<int>'
    if ~( isnumeric(value) && isreal(value) )
      error( [field ': Only real numeric values are accepted for floats'] );
    end
    if min(size(value)) > 1
      error( [field ': Only vectors are accepted'] );
    end
    mhaval = sprintf('[%s]',sprintf('%1.30g ',value));
   case 'vector<complex>'
    if ~isnumeric(value)
      error( [field ': Only numeric values are accepted for floats'] );
    end
    if min(size(value)) > 1
      error( [field ': Only vectors are accepted'] );
    end
    tmp = '';
    for k=1:length(value)
      if imag(value(k)) >=0 
	tmp = sprintf('%s (%1.30g+%1.30gi)',...
		      tmp,...
		      real(value(k)), ...
		      imag(value(k)));
      else
	tmp = sprintf('%s (%1.30g%1.30gi)',...
		      tmp,...
		      real(value(k)), ...
		      imag(value(k)));
      end
    end
    mhaval = sprintf('[%s]',tmp);
   case 'matrix<float>'
    if ~( isnumeric(value) && isreal(value) )
      error( [field ': Only real numeric values are accepted for floats'] );
    end
    tmp = '';
    for k=1:size(value,1)
      tmp = sprintf('%s[%s];',tmp,sprintf('%1.30g ',value(k,:)));
    end
    mhaval = sprintf('[%s]',tmp);
   otherwise
    error(sprintf('%s: type ''%s'' is not handled',field, type));
  end


