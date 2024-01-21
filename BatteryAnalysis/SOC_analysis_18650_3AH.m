battery_table = readtable('test_output_18650_3AH.csv')
battery_data = table2array(battery_table);
time_data = battery_data(1:23188, 2);
time_data = 100 - (time_data/time_data(23188)) * 100.0
voltage_data = battery_data(1:23188, 3);
voltage_data = voltage_data * (3758/2080)
vbatt_fn_1 = polyfit(time_data, voltage_data, 1);
vbatt_fn_2 = polyfit(time_data, voltage_data, 2);
vbatt_fn_3 = polyfit(time_data, voltage_data, 3);

vbatt_pred_1 = polyval(vbatt_fn_1, time_data);
vbatt_pred_2 = polyval(vbatt_fn_2, time_data);
vbatt_pred_3 = polyval(vbatt_fn_3, time_data);

figure(1)
hold on
grid on 
set ( gca, 'xdir', 'reverse' )
plot(time_data, voltage_data)
plot(time_data, vbatt_pred_1)
plot(time_data, vbatt_pred_2)
plot(time_data, vbatt_pred_3)
legend('VBatt Actual', 'First Order Prediction', 'Second Order Prediction', 'Third Order Prediction')
title('VBatt Vs. SOC')
hold off

