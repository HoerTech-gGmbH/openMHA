function remove_octave_directories_from_windows_path()
  % When executing in octave on windows, remove directories containing
  % the string octave (case insensitive) from the environment variable PATH
  % unless the global variable KEEP_OCTAVE_DIRECTORIES_IN_PATH is truthy.

  % functionality can be switched off by setting this global variable to true
  global KEEP_OCTAVE_DIRECTORIES_IN_PATH;
  if KEEP_OCTAVE_DIRECTORIES_IN_PATH
    return
  end

  if ispc() && isoctave()
				% We are in octave on windows
    PATH = getenv('PATH');

   % Quick and HARMFUL hack: replace octave with an unlikely
   % string. This is harmful because it might place things in the path
   % that we do not want to have in there.
    PATH = regexprep(PATH, 'octave', 'NICHTS', 'ignorecase');

    setenv('PATH', PATH);
  end
