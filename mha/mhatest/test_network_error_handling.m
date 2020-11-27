function test_network_error_handling
  mha.host='localhost';
  mha.port=2; % or any other TCP port that is closed on mha.host
  try
    mha_get(mha,'nchannels_in'); % results in matlab-user-visible null pointer exception
    error('No error message thrown, although expected.');
  catch exception
    if ~strcmp(exception.message,'Could not resolve host localhost or could not connect to localhost:2')
      error('Wrong error message thrown.');
    end
  end
