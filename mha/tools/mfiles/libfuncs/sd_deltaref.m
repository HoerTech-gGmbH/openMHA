function s = sd_deltaref( s, field, val )
% calculate difference to reference condition
% field : field number or name of reference paremeter
% val : value of reference condition
  s = sd_compactval( s );
  sField = sd_getfield( s, field );
  fieldvalues = sd_getvalues(s,field);
  sRef = sd_restrict( s, field, val );
  sRef = sd_par2col( sRef, field );
  Npar = numel(sRef.values);
  Ndata = numel(sRef.fields)-Npar;
  Nfields = numel(fieldvalues);
  fieldnames = s.fields(numel(s.values)+1:end);
  for k=1:numel(fieldnames)
    fieldnames{k} = ['d',fieldnames{k}];
  end
  dataRef = sRef.data(:,Npar+[1:Ndata]);
  s = sd_par2col( s, field );
  s.data(:,Npar+1:end) = ...
      s.data(:,Npar+1:end) - ...
      repmat(dataRef,[1,Nfields]);
  s = sd_reorder_datacol( s, Nfields );
  [s,mFields] = ...
      sd_col2par( s, sField, fieldvalues, fieldnames );
  s = sd_restrict( s, field, val, true );
  s = sd_compactval( s );
