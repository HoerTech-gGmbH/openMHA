function clientdb_export_auds( name, bAnonymous )
% CLIENTDB_EXPORT_AUDS - Export audiograms from client database in
% csv format
%
% clientdb_export_auds( name, bAnonymous )
%
% name: Name of output file
% bAnonymous: do not print personal information
  fh = fopen(name,'w');
  if fh < 0
    error(['unable to open file: ',name]);
  end
  if nargin < 2
    bAnonymous = 0;
  end
  s = load('client_datab.mat');
  if bAnonymous
    fprintf(fh,'"%s","%s"',...
	    'client number',...
	    'aud ID');
  else
    fprintf(fh,'"%s","%s","%s","%s","%s"',...
	    'ID',...
	    'lastname',...
	    'firstname',...
	    'birthday',...
	    'aud ID');
  end
  fprintf(fh,',"left %g"',[125 250 500 750 1000 1500 2000 3000 4000 6000 8000]);
  fprintf(fh,',"right %g"',[125 250 500 750 1000 1500 2000 3000 4000 6000 8000]);
  fprintf(fh,'\n');
  for kcl=1:size(s.clients,2)
    sCl = s.clients{2,kcl};
    if isfield(sCl, 'audiograms')
      for kaud=1:size(sCl.audiograms,2)
        if bAnonymous
	  fprintf(fh,'%d,"%s"',...
		  kcl,...
		  sCl.audiograms{1,kaud});
        else
	  fprintf(fh,'"%s","%s","%s","%s","%s"',...
		  s.clients{1,kcl},...
		  sCl.lastname,...
		  sCl.firstname,...
		  sCl.birthday,...
		  sCl.audiograms{1,kaud});
        end
        fprintf(fh,',%g',sCl.audiograms{2,kaud}.l.htl);
        fprintf(fh,',%g',sCl.audiograms{2,kaud}.r.htl);
        fprintf(fh,'\n');
      end
    end
  end
  fclose(fh);