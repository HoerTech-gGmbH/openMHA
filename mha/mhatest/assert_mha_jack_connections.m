function assert_mha_jack_connections(expect_stat,expect_conn)
  [status,connection] = ...
  system('jack_lsp -c |sed -e :a -e ''$!N;s/\n //;ta'' -e "P;D" | grep ^MHA');
  assert_equal(expect_stat,status);
  assert_equal(expect_conn,connection);
end
