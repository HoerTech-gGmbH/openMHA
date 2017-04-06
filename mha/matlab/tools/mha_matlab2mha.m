function mhaval = mha_matlab2mha( type, value )
% MHA_MATLAB2MHA - convert Matlab values to MHA string
% representations
%
% Usage:
%
% mhaval = mha_matlab2mha( type, value )
%
% type : MHA type of variable (character string)
% value : Matlab value to be converted
%
% mhaval : MHA string representation of Matlab value
%
% (c) 2005 Universitaet Oldenburg
% Author: Giso Grimm
  
  mhaval = '';
  switch type
   case 'string'
    if ~ischar(value)
      error( 'Only character strings are accepted' );
    end
    if min(size(value)) > 1
      error( 'Only single-line strings are accepted' );
    end
    mhaval = value;
   case 'keyword_list'
    if ~ischar(value)
      error( 'Only character strings are accepted' );
    end
    if min(size(value)) > 1
      error( 'Only single-line strings are accepted' );
    end
    mhaval = value;
   case 'float'
    if ~( isnumeric(value) && isreal(value) )
      error( 'Only real numeric values are accepted for floats' );
    end
    if prod(size(value)) ~= 1
      error( 'Only scalars are accepted.');
    end
    mhaval = sprintf('%1.30g',value);
   case 'int'
    if ~( isnumeric(value) && isreal(value) )
      error( 'Only real numeric values are accepted for floats' );
    end
    if prod(size(value)) ~= 1
      error( 'Only scalars are accepted.');
    end
    mhaval = sprintf('%1.30g',value);
   case 'complex'
    if ~isnumeric(value)
      error( 'Only numeric values are accepted for floats' );
    end
    if prod(size(value)) ~= 1
      error( 'Only scalars are accepted.');
    end
    if imag(value) >= 0 
      mhaval = sprintf('(%1.30g+%1.30gi)',real(value),imag(value));
    else
      mhaval = sprintf('(%1.30g%1.30gi)',real(value),imag(value));
    end
   case 'bool'
    if prod(size(value)) ~= 1
      error( 'Only scalars are accepted.');
    end
    if value
      mhaval = 'yes';
    else
      mhaval = 'no';
    end
   case 'vector<string>'
    if ~iscell(value)
      error('Only cell strings are accepted');
    end
    tmp = '';
    for val=value
      if ~ischar(val{:})
	error('Only string components are accepted');
      end
      if ~isempty(find(val{:}==' '))
	error(['No whitespace is allowed in MHA string vector' ...
	       ' components.']);
      end
      tmp = sprintf('%s%s ',tmp,val{:});
    end
    mhaval = sprintf('[%s]',tmp);
   case 'vector<float>'
    if ~( isnumeric(value) && isreal(value) )
      error( 'Only real numeric values are accepted for floats' );
    end
    if min(size(value)) > 1
      error( 'Only vectors are accepted' );
    end
    mhaval = sprintf('[%s]',sprintf('%1.30g ',value));
   case 'vector<int>'
    if ~( isnumeric(value) && isreal(value) )
      error( 'Only real numeric values are accepted for floats' );
    end
    if min(size(value)) > 1
      error( 'Only vectors are accepted' );
    end
    mhaval = sprintf('[%s]',sprintf('%1.30g ',value));
   case 'vector<complex>'
    if ~isnumeric(value)
      error( 'Only numeric values are accepted for floats' );
    end
    if min(size(value)) > 1
      error( 'Only vectors are accepted' );
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
      error( 'Only real numeric values are accepted for floats' );
    end
    tmp = '';
    for k=1:size(value,1)
      tmp = sprintf('%s[%s];',tmp,sprintf('%1.30g ',value(k,:)));
    end
    mhaval = sprintf('[%s]',tmp);
   otherwise
    error(sprintf('type ''%s'' is not handled',type));
  end
