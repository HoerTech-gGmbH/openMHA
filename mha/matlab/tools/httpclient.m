function r = httpclient( url )
  cmd = sprintf('wget --output-document=- --quiet "%s"',url);
  [c,r] = system(cmd);
  if c ~= 0
    r = sprintf('Command ''%s'' failed:\n%s', cmd, r);
    error(r);
  end
