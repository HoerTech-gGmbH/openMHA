function [sn1,sn2,sn3] = hagerman_generate_signals( s, n )
% hagerman_generate_signals - generate mixed signals for processing
% and estimation of processed S and N
%
% Usage:
%
% Create signals for estimation after Hagerman and Olofsson (2004):
%
% [sn1,sn2] = hagerman_generate_signals( s, n )
%
% Create signals also for estimate error signal after Olofsson and
% Hansen (2006):
%
% [sn1,sn2,sn3] = hagerman_generate_signals( s, n )
%
% See hagerman_estimate.m for more details and for signal estimation.
%
% Hagerman, B., & Olofsson, Å. (2004). A method to measure the effect
% of noise reduction algorithms using simultaneous speech and
% noise. Acta Acustica United with Acustica, 90(2), 356-361.
%
% Olofsson, Å., & Hansen, M. (2006). Objectively measured and
% subjectively perceived distortion in nonlinear systems. The Journal
% of the Acoustical Society of America, 120(6),
% 3759–3769. https://doi.org/10.1121/1.2372591
%
% Implementation: Giso Grimm, 2019, 2020

sn1 = s+n;
sn2 = s-n;
if nargout > 2
  sn3 = imag(hilbert(sn1));
end