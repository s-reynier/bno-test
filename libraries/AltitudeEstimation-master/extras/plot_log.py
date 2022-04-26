import matplotlib.pyplot as plt

with open("log.txt") as f:
	data = f.readlines()
	data = [d.rstrip().split() for d in data]
	accel = [d[0] for d in data]
	vel = [d[1] for d in data]
	height = [d[2] for d in data]
	height_from_baro = [d[3] for d in data]

fig = plt.figure()

ax1 = fig.add_subplot(111)

ax1.set_title("acceleration, velocity and height estimation")    
ax1.set_xlabel('Sample')
ax1.set_ylabel('Value')

ax1.plot(height_from_baro, c='#99ffe6', label='Height from baro')
ax1.plot(accel, c='#ffa366', label='Acceleration')
ax1.plot(vel, c='r', label='Velocity')
ax1.plot(height, c='g', label='Height')

leg = ax1.legend()

plt.show()