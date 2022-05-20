import pandas as pd
import numpy as np
import matplotlib.pyplot as plt



df = pd.read_csv("/home/justin/juce_log.csv")
temp = df['value'].to_numpy()
plt.plot(temp[0:100])
plt.show()
