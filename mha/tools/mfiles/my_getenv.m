function value = my_getenv(name)
% Get value of environment variable, even on windows.

if ispc
    parameter = ['%' name '%'];
    [dummy, value] = system(['echo ' parameter]);
    value = value(1:(end-1));
    if (~isequal(parameter, value)) && (~isequal('', value))
        return
    end
end

value = getenv(name);
