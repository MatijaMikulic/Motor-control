%load saved Gp, G_pi_new and G_PID_new
clc

Gx1=feedback(Gp*G_pi_new,1);
Gx2=feedback(Gp*G_PID_new,1);

figure(1)
step(Gx1)
hold on
step(Gx2)

legend('PI','PID')

stepinfo(Gx1)
stepinfo(Gx2)