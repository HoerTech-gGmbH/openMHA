function result = mha_tcpsound(arg1, arg2)
% Open sound data connection:
% handle = mha_tcpsound(hostname, port)
%
% Exchange sound data:
% output = mha_tcpcound(handle, input)
% rows == frames, colums==channels, rows MUST match fragsize
%
% Close connection:
% mha_tcpcound(handle,'close')
if isequal(class(arg1), 'char')
  result = javaObject('de.hoertech.mha.io.TCPSound',arg1, arg2);
elseif isequal(class(arg2), 'char')
  arg1.close();
  result = [];
else
  result = arg1.exchange_sound_data(arg2(:), 10);
  if exist('OCTAVE_VERSION','builtin') > 0
    result = java2mat(result);
  end
  result = reshape(result, arg1.getFragsize(), arg1.getNchannelsOut());
end
