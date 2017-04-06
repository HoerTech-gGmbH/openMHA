function logmsg(str)
% write logmessage to file "logmsgfile" if it exitst
fd = fopen('logmsgfile','r');
if fd > -1
  fclose(fd);
  fd = fopen('logmsgfile','a');
  fprintf(fd, '%s\n', str);
  fclose(fd);
end
disp(str);
pause(0.01);
