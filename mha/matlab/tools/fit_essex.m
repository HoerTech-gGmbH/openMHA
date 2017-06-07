function fit_essex(audiogram)
% audiogram, first row: Hearing Threshold / dB HL for 250, 500, 1000, 2000,
% 4000, 8000, left, second row: same for right.

mha = struct('host','localhost','port',33337);
ids = mha_findid(mha);
essex = ids.essex_aid;
edge_freq = mha_get(mha,[essex,'.edge_frequencies']);
bands = length(edge_freq) - 1;
center_freq = sqrt(edge_freq(1:bands).*edge_freq(2:end));

gain = max(zeros(2,6),audiogram-5);
tc = 95 - gain;
tm = ones(1,6) * 25;

mha_set(mha,[essex,'.inst_compr_threshold'],tc);
mha_set(mha,[essex,'.gain'],gain);
mha_set(mha,[essex,'.moc_tc'],tm);

