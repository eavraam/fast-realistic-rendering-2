import matplotlib.pyplot as plt

# Read the file
filename = "g-off-oc.txt"
with open(filename, "r") as file:
    lines = file.readlines()

# Extract FPS values
time = []
fps = []
for line in lines:
    if line.startswith("FPS:"):
        parts = line.split(":")
        time.append(len(fps) + 1)  # Assuming 1 second intervals
        fps.append(float(parts[1].strip()))

# Create the plot
plt.plot(time, fps)
plt.xlabel("Time (seconds)")
plt.ylabel("FPS")
plt.title("FPS Over Time")
plt.show()
