function db = get_database(pattern, dbfilename)
% db = get_database(pattern,dbfilename)
% Return the messol result database table contents as a cell matrix of
% strings.  If pattern is given, return only those records that match the
% pattern. if dbfilename is given, use that dbfilename instead of
% 'C:\MOL\MOL1.3.0.5_mha\dbc.lsql\MESSOL_MEASRESULT.DB'. (should not
% contain spaces)
% Pattern is a cellstring. Only records where all nonempty strings in the
% pattern match the respective fields in the record are included in the
% result.

% return all 
if nargin < 1
    pattern = {};
end
if nargin < 2
    dbfilename = 'C:\MOL\MOL1.3.0.5_mha\dbc.lsql\MESSOL_MEASRESULT.DB';
end
dbexe = 'c:\Programme\cygwin\bin\paradox2text.exe';

[dummy, databasestr] = system(sprintf('%s %s', dbexe, dbfilename));
databasestr = split_data_into_lines(databasestr);
db = {};
for i = 2:length(databasestr);
    record = split_dbrecord_into_fields(databasestr{i});
    matching = true;
    for j = 1:length(pattern)
        if ~isempty(pattern{j})
            if (j > length(record)) || ~isequal(pattern{j}, record{j})
                matching = false;
            end
        end
    end
    if matching
        db = [db; record];
    end
end

function fields = split_dbrecord_into_fields(record)
% function fields = split_dbrecord_into_fields(record)
%
% split the string <record> into <comma-separated fields> and undo the
% backslash-escapes.
idx = [0, find(record == ','), (length(record) + 1)];
fields = {};
for i = 2:length(idx)
    fields = [fields {deoctalize(record((idx(i-1)+1):(idx(i)-1)))}];
end
  
function result = deoctalize(field)
idx = [-3 find(field == '\')];
result = '';
for i = 2:length(idx)
    result = [result field([(idx(i-1)+4):(idx(i)-1)])];
    result = [result sprintf(field(idx(i):(idx(i)+3)))];
end
result = [result field((idx(end)+4):end)];






