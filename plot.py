import matplotlib.pyplot as plt

# Random test data
all_data = []

f = open("output")
data = []
for line in f:
    data.append(int(line))

f.close()

count = 0
while count < len(data):
    temp = []
    for i in range(10):
        base = data[count]
        count += 1
        lock = data[count]
        count += 1
        tm = data[count]
        count += 1
        temp.append(float(base) / lock)
    all_data.append(temp)

print len(all_data)

fig, ax = plt.subplots()

# rectangular box plot
bplot = ax.boxplot(all_data,
        vert=True,   # vertical box aligmnent
        patch_artist=True)   # fill with color

# adding horizontal grid lines
ax.yaxis.grid(True)
ax.set_xticks([y+1 for y in range(len(all_data))], )
ax.set_xlabel('thread num')
ax.set_ylabel('speed')
    
# add x-tick labels
plt.setp(ax, xticks=[y+1 for y in range(len(all_data))],
        xticklabels=['x1', 'x2', 'x3', 'x4'])
plt.show()
