import csv
import matplotlib.pyplot as plt

# Data
grid_sizes = [129, 257, 385, 513, 729, 1025, 1537]
sync_times = [0.137159, 2.520113, 11.866734, 39.914363, 163.869716, 690.590143, 4677.481956]
async_times = [0.098729, 1.590930, 7.843416, 25.853520, 114.962984, 475.958853, 1865.537191]

# Save to CSV
with open("sandpile_serial_performance.csv", "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["Grid Size", "Synchronous Time (s)", "Asynchronous Time (s)"])
    for i in range(len(grid_sizes)):
        writer.writerow([grid_sizes[i], sync_times[i], async_times[i]])

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(grid_sizes, sync_times, marker='o', label="Synchronous", linewidth=2)
plt.plot(grid_sizes, async_times, marker='s', label="Asynchronous", linewidth=2)
plt.title("Synchronous vs Asynchronous Sandpile Performance (Serial Version)")
plt.xlabel("Grid Size (NxN)")
plt.ylabel("Time (seconds)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("sandpile_serial_performance_plot.png")
plt.show()
