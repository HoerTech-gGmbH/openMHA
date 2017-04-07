function ha_response_plot( sResponse )
  figure('Name','Hearing Aid receiver response');
  H = sResponse.H;
  cCorr = sResponse.corr;
  subplot(4,1,1);
  plot_H(H.insertion);
  ylim([-50 30]);
  ylabel('insertion gain / dB');
  subplot(4,1,2);
  plot_H(H.correction);
  hold on;
  mColor = [0,0,1;1,0,0];
  for k=1:length(cCorr)
    plot(cCorr{k}.f,cCorr{k}.g,'o','MarkerSize',8,'Color',mColor(k,:));
  end
  ylim([-50 30]);
  ylabel('correction gain / dB');
  subplot(4,1,3);
  plot_H(H.feedback);
  ylim([-60 -10]);
  ylabel('feedback path / dB');
  subplot(4,1,4);
  plot_H(H.leakage);
  ylim([-45 5]);
  ylabel('leakage / dB');

function plot_H( H )
  h = plot([1:size(H,1)]-1,20*log10(abs(H)),'linewidth',2);
  xlim([125 16000]);
  set(gca,'XScale','log','XTick',1000*2.^[-3:4]);
  set(h(1),'Color',[0,0,1]);
  set(h(2),'Color',[1,0,0]);
  grid on;
  xlabel('frequency / Hz');
    
