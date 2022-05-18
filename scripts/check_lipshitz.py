import pandas as pd
import numpy as np
import matplotlib.pyplot as plt



df = pd.read_csv("/home/justin/juce_log.csv")
print(df['value'].shape)
values = df['value'].to_numpy()
plt.plot(values[1:]) # - values[:-1])
plt.show()
