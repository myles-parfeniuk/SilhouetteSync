battery_table = readtable('test_output_discharge.csv');
battery_data = table2array(battery_table);
sample_nums = battery_data(1:6376, 1);
real_soc = 100 - ((sample_nums/sample_nums(6376)) * 100.0);
voltage_data = battery_data(1:6376, 3);
soc_fn_1 = polyfit(voltage_data, real_soc , 1);
soc_fn_2 = polyfit(voltage_data, real_soc, 2);
soc_fn_3 = polyfit(voltage_data, real_soc, 3);

soc_pred_1 = polyval(soc_fn_1, voltage_data);
soc_pred_2 = polyval(soc_fn_2, voltage_data);
soc_pred_3 = polyval(soc_fn_3, voltage_data);

figure(1)
hold on
grid on 
set ( gca, 'xdir', 'reverse' )
plot(voltage_data, real_soc)
plot(voltage_data, soc_pred_1)
plot(voltage_data, soc_pred_2)
plot(voltage_data, soc_pred_3)
legend('SOC Actual', 'First Order Prediction', 'Second Order Prediction', 'Third Order Prediction')
title('SOC (%) Vs. VBatt (mv)')
hold off
