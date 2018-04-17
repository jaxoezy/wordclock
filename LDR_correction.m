% LDR calibration
% 1: logarithm, 2: square root

x = 0:1:1023;
y = log10(x+1)*1023/log10(1023+1);
y = floor(y);

y2 = sqrt(x)*1023/sqrt(1023);
y2 = floor(y2);

figure(1)
clf
hold on
grid on
box on
plot(x,y, 'r-')
plot(x,x, 'k--')
plot(x,y2, 'k-')

axis([0 1023 0 1023])

y_mat = reshape(y,[16,64])';
csvwrite('LDR_correction_1.csv',y_mat)

y_mat = reshape(y2,[16,64])';
csvwrite('LDR_correction_2.csv',y_mat)