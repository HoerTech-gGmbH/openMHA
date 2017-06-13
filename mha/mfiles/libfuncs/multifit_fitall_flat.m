function csPlugs = multifit_fitall_flat( mha, sGainrule, HTL)
  cdb = libclientdb();
  sAud = cdb.flat_aud( HTL );
  csPlugs = multifit_uploadallfirstfit( mha, sGainrule, sAud, 'lr' );
