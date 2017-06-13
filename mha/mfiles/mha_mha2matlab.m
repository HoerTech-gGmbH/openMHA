function value = mha_mha2matlab( type, mhaval )
% MHA_MHA2MATLAB - convert MHA string representations to Matlab
% values
%
% Usage:
%
% value = mha_mha2matlab( type, mhaval )
%
% type : MHA type of variable (character string)
% mhaval : MHA string representation
%
% value : Matlab value
%
% (c) 2005 Universitaet Oldenburg
% Author: Giso Grimm
   
  ;
  % remove blanks at beginnig and end:
  if ~isempty(mhaval)
    while mhaval(1) == ' '
      mhaval(1) = [];
    end
  end
  if ~isempty(mhaval)
    while mhaval(end) == ' '
      mhaval(end) = [];
    end
  end
  switch type
   case {'float','vector<float>','matrix<float>', ...
	 'complex','vector<complex>','matrix<complex>', ...
	 'int','vector<int>','matrix<int>'}
    
    %replace both nanis and infis
    subval = strrep(mhaval, 'nani','nan*i');
    subval = strrep(subval, 'infi','inf*i');
    value = eval(subval);
    
   case {'string','keyword_list'}
    value = mhaval;
   case 'vector<string>'
    if (mhaval(1) ~= '[') || (mhaval(end) ~= ']')
      error('A MHA vector has to be enclosed by ''[ ]''');
    end
    mhaval = mhaval(2:end-1);
    value = {};
    while ~isempty(mhaval)
      [token, mhaval] = strtok(mhaval);
      if ~isempty(token)
	value = [value, {token}];
      end
    end
   case 'bool'
    switch mhaval
     case 'yes'
      value = 1;
     case 'no'
      value = 0;
     otherwise
      error(sprintf('Invalid boolean value: ''%s''',mhaval));
    end
   otherwise
    error(sprintf('Unsupported type: ''%s''',type));
  end
