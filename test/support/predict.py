import numpy as np
import pandas as pd
from isotree import IsolationForest

df = pd.read_csv('test/support/data.csv')
model = IsolationForest(ntrees=10, ndim=3, nthreads=1)
model.fit(df)

predictions = model.predict(df)
print(predictions[0:3].tolist())
print('Point with highest outlier score: ', df.iloc[np.argsort(-predictions)[0]])

print('avg_depth')
print(model.predict(df, output='avg_depth')[0:3].tolist())

model.export_model('test/support/model.bin', add_metada_file=True)
