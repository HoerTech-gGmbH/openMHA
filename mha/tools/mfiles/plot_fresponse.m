function plot_fresponse(fresponse)
% function plot_fresponse(fresponse)
% fresponse: struct with fields
% Frequencies, dBFSfor80dB, correctionsdB, REUG as created by measure_fresponse

  figure;
  gains = fresponse.dBFSfor80dB + fresponse.REUGdB + fresponse.correctionsdB;
  mingain = min(gains);
  gains = gains - mingain;
  semilogx(fresponse.Frequencies, gains);
  set(gca,'xtick',fresponse.Frequencies);
  labels = {};
  for f = fresponse.Frequencies
    labels = [labels, {num2str(f)}];
  end
  set(gca,'xticklabel',labels);
  grid on;

  ylabel('Required gain in dB');
  xlabel('Frequency in Hz');
  title('Calibration filter frequency response for the measured output hardware');
  
