function test_parsing_complex_variable % erroneous input produces expected error messages
  test_data = {'0+1i)'  , '(mha_parser) opening ''('' missing in complex number'
               '(0+1i'  , '(mha_parser) closing '')'' missing, or complex number has a space in it'
               '0+1i'   , '(mha_parser) parenthesis ''()'' missing, or complex number has a space in it'
               '( 0+1i)', '(mha_parser) closing '')'' missing, or complex number has a space in it'
               '(0 +1i)', '(mha_parser) closing '')'' missing, or complex number has a space in it'
               '(0+ 1i)', '(mha_parser) closing '')'' missing, or complex number has a space in it'
               '(0+1 i)', '(mha_parser) closing '')'' missing, or complex number has a space in it'
               '(0+1i )', '(mha_parser) closing '')'' missing, or complex number has a space in it'
               '(+1i)'  , '(mha_parser) real part is missing in the complex number'
               '(0+i)'  , '(mha_parser) Number expected, found ''i'''
               '(0+)'   , '(mha_parser) Number expected, found '')'''
               '/.(0+i)', '(mha_parser) There must be no symbol or character outside the parenthesis ''()'' containing complex value'
               '(0+i).+', '(mha_parser) There must be no symbol or character outside the parenthesis ''()'' containing complex value'
               '(0<1i)' , '(mha_parser) imaginary part starts with ''<'' instead of ''+'' or ''-'''};
  success_vect = zeros(length(test_data),1);
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'mhalib','gtfb_analyzer');
  for i = 1 : length(test_data)
     try
         mha_set(mha,'mha.norm_phase', test_data( i , 1 ) );
     catch ME
         % strfind returns empty array, if error msg was not found
         if ~isempty (strfind( ME.message, test_data( i , 2 ) ))
             success_vect (i) = 1;
         end
     end
  end
  assert_all ( success_vect, 'Test for parsing complex variable failed' );
end
