function success = runtests(directory)
% runtests([directory])
%
% Run all tests in the current directory or,
% if given, in the directory specified by the [directory] parameter.
%
% runtests invokes all m-files in [directory] that correspond to a
% "test_*.m" naming scheme. These files should implement unit tests,
% they can use any of the assert_* messages for this purpose.
% Placement of [directory] in the Matlab Path should be done by the
% user, because this function will not alter the Matlab Path.
%
% runtests collects the test results and recovers from test failures
% and errors. runtests prints a summary of test results after all tests
% have executed.
% 
% runtests will call unittest_teardown after every test,
% regardless of success, failure, or error.
%
% runtests returns true if all tests passed, else false

% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke

success = false;

if (nargin < 1)
    directory = pwd;
end

% initialize counters and message collectors
tests = 0;

failures = 0;
failtexts = {};

errors = 0;
errortexts = {};

teardown_errors = 0;
teardown_errortexts = {};

% reset counter for assertions to 0 (argument indicates this special case)
unittest_assert('unittest: reset assertion count');

% Get all mfiles in dirctory 
files = what(directory);
files = files(1).m;

% iterate over all "test_*.m" files
times = {};
indices = strmatch('test_', files);
for index = indices(:)'
    filename = files{index};
    start_time = clock;
    times{end+1,2} = filename;
    try
      tests = tests + 1;

      % function name is filename without trailing ".m" 
      func = str2func(filename(1:length(filename)-2));
      
      % ensure teardown stack is empty (argument indicates this special case)
      unittest_teardown([]);
      
      % invoke test
      feval(func);
      
    catch
      % test invocation caused a matlab error. Assertion failures as well
      % as general errors are reported via matlab errors. Assertion
      % failures are recognized by a "unittest:failure" matlab error id
      [err,id] = lasterr;
      if isequal('unittest:failure', id)
        failures = failures + 1;
        failtexts{length(failtexts) + 1} = ...
            [fullfile(directory, filename) ': ' err];
      else
        errors = errors + 1;
        errortexts{length(errortexts) + 1} = ...
            [fullfile(directory, filename) ': ' err];
      end
    end
    try
      unittest_teardown; % execute teardowns registered by test function
    catch
      [err,id] = lasterr;
      teardown_errors = teardown_errors + 1;
      teardown_errortexts{length(errortexts) + 1} = ...
          [fullfile(directory, filename) ': TEARDOWN ERROR: ' err];
    end
    times{end,1} = etime(clock, start_time);
end

% print summary of test results
fprintf(1, ...
        ['%d tests: %d assertions, ', ...
         '%d failures, %d errors, %d teardown errors\n'], ...
        tests, ...
        unittest_assert(1)-1, failures, ...
        errors, teardown_errors);
if failures
  for index = 1:length(failtexts)
    fprintf(1, 'failed: %s\n', failtexts{index});
  end
end
if errors
  disp('errors:')
  for index = 1:length(errortexts)
    fprintf(1,'%s\n', errortexts{index});
  end
end
if teardown_errors
  disp('teardown errors:')
  for index = 1:length(teardown_errortexts)
    fprintf(1,'%s\n', teardown_errortexts{index});
  end
end

if ~failures && ~errors && ~teardown_errors
  success = true

  % sort times
  [dummy, permutation] = sort(cell2mat(times(:,1)));
  for t = times(permutation,:)'
    disp(sprintf('%f\t%s', t{:}));
  end
  Sum = sum(cell2mat(times(:,1)))
end

