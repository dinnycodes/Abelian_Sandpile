import numpy as np
from PIL import Image

# Define color mapping for each cell value
colors = {
    0: (0, 0, 0),       # Black
    1: (0, 255, 0),     # Green
    2: (0, 0, 255),     # Blue
    3: (255, 0, 0),     # Red
}

# Load the board from the file
data = np.loadtxt("board.txt", dtype=int)
height, width = data.shape

# Scale factor to enlarge each cell (10x10 pixels per cell)
scale = 10
img = Image.new("RGB", (width * scale, height * scale))

# Paint each tile
for i in range(height):
    for j in range(width):
        color = colors.get(data[i, j], (200, 200, 200))  # Gray for unexpected values
        for dy in range(scale):
            for dx in range(scale):
                img.putpixel((j*scale + dx, i*scale + dy), color)

# Save the image
img.save("output.png")
print("Saved output.png")
