function gains_out = nal_nl1_fill_void(levels, frequencies, gains_in, discrim, db_per_octave, db_per_hz, db_per_band)
if nargin < 5
    db_per_octave = 12;
end
if nargin < 6
    db_per_hz = 0;
end
if nargin < 7
    db_per_band = 0;
end

db_per_db = 0.7;

discrim = discrim ~= 0;

gains_out = repmat(nan,size(gains_in));
gains_out(discrim) = gains_in(discrim);

while any(isnan(gains_out(:)))

gains_out_1 = gains_out;
for fi = (2:1:length(frequencies))
    octaves = log2(frequencies(fi) / frequencies(fi-1));
    hz = frequencies(fi) - frequencies(fi-1);
    diffgain = octaves * db_per_octave + hz * db_per_hz + db_per_band;
    for li = (1:1:length(levels))
        if isnan(gains_out_1(li,fi)) && ~isnan(gains_out_1(li,fi-1))
            gains_out_1(li,fi) = gains_out_1(li,fi-1) - diffgain;
        end
    end
end

gains_out_2 = gains_out;
for fi = ((length(frequencies)-1):-1:1)
    octaves = log2(frequencies(fi+1) / frequencies(fi));
    hz = frequencies(fi+1) - frequencies(fi);
    diffgain = octaves * db_per_octave + hz * db_per_hz + db_per_band;
    for li = (1:1:length(levels))
        if isnan(gains_out_2(li,fi)) && ~isnan(gains_out_2(li,fi+1))
            gains_out_2(li,fi) = gains_out_2(li,fi+1) - diffgain;
        end
    end
end

gains_out_3 = gains_out;
for li = (2:1:length(levels))
    diffgain = db_per_db * (levels(li)-levels(li-1));
    for fi = (1:1:length(frequencies))
        if isnan(gains_out_3(li,fi)) && ~isnan(gains_out_3(li-1,fi))
            gains_out_3(li,fi) = gains_out_3(li-1,fi) - diffgain;
        end
    end
end

gains_out_4 = gains_out;
for li = (length(levels)-1):-1:1
    diffgain = db_per_db * (levels(li+1)-levels(li));
    for fi = (1:1:length(frequencies))
        if isnan(gains_out_4(li,fi)) && ~isnan(gains_out_4(li+1,fi))
            gains_out_4(li,fi) = gains_out_4(li+1,fi) - diffgain;
        end
    end
end

gains_out = max(max(gains_out_1,gains_out_2),max(gains_out_3,gains_out_4));

end

gains_out = max(gains_out,0);