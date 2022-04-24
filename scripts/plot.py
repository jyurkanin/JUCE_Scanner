import pandas as pd
import numpy as np
import matplotlib.pyplot as plt



df = pd.read_csv("/home/justin/juce_log.csv")
plt.plot(df['value'])
plt.show()
