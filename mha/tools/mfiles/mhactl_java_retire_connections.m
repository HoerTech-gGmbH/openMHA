function mhactl_java_retire_connections()
% Function to be registered with atexit in octave if there are active mhactl
% java connections
mhactl_java('retire_connections');
