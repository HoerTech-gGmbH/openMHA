function [peak,fir] = fresponse_to_cfg(fresponse)
% function fresponse_to_cfg(fresponse)
% fresponse: struct with fields
% Frequencies, dBFSfor80dB, correctionsdB, REUG as created by measure_fresponse

  fir_length = 65; % Adjust as desired.
  % Greater Length equals better calibration result but also more delay.
  
  gains = fresponse.dBFSfor80dB + fresponse.REUGdB + fresponse.correctionsdB;
  peak_correction = max(gains);
  gains = gains - peak_correction;
  peak = 0  - peak_correction + 80; % the level we used for calibration
  fir = fir2(fir_length, [0 (fresponse.Frequencies / fresponse.sampling_rate * 2) 1], 10.^([gains(1), gains, -inf] / 20));

  printf("The following values can be configured into the transducers plugin for the audio output channel where this output hardware is connected:\n");
  printf("calib_out.peaklevel = [... %f ...]\n", peak);
  printf("calib_out.fir = [... ;[");
  printf("%.7f ", fir);
  printf("]; ...]\n");
  
