import numpy as np
import pandas as pd
from isotree import IsolationForest

df = pd.read_csv('test/support/data.csv')
model = IsolationForest.import_model("/tmp/model.bin")

predictions = model.predict(df)
print(predictions[0:3].tolist())
print('Point with highest outlier score: ', df.iloc[np.argsort(-predictions)[0]])
