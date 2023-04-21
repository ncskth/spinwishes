import numpy as np
import matplotlib.pyplot as plt

# Create a random array of size W x H with float numbers between 0 and 1
W = 640
H = 480
array = np.random.rand(H, W)

# Create a heatmap using imshow function
plt.imshow(array, cmap='hot', interpolation='nearest', vmin=0, vmax=1)

# Add colorbar to show the scale
plt.colorbar()

# Set axis labels and title
plt.xlabel('Width')
plt.ylabel('Height')
plt.title('Heatmap of Array')

# Display the plot
plt.show()